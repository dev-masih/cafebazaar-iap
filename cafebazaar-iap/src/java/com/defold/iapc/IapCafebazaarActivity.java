package com.defold.iapc;

import java.util.ArrayList;
import java.util.List;

import org.json.JSONException;
import org.json.JSONObject;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentSender.SendIntentException;
import android.content.ServiceConnection;
import android.content.pm.ResolveInfo;
import android.os.Bundle;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup.LayoutParams;

import com.android.vending.billing.IInAppBillingService;
import com.defold.iapc.IapCafebazaar.Action;

public class IapCafebazaarActivity extends Activity {

    private boolean hasPendingPurchases = false;
    private boolean autoFinishTransactions = true;
    private boolean isDone = false;
    private Messenger messenger;
    ServiceConnection serviceConn;
    IInAppBillingService service;

    // NOTE: Code from "trivialdrivesample"
    int getResponseCodeFromBundle(Bundle b) {
        Object o = b.get(IapCafebazaar.RESPONSE_CODE);
        if (o == null) {
            Log.d(IapCafebazaar.TAG, "Bundle with null response code, assuming OK (known issue)");
            return IapJNI.BILLING_RESPONSE_RESULT_OK;
        } else if (o instanceof Integer)
            return ((Integer) o).intValue();
        else if (o instanceof Long)
            return (int) ((Long) o).longValue();
        else {
            Log.e(IapCafebazaar.TAG, "Unexpected type for bundle response code.");
            Log.e(IapCafebazaar.TAG, o.getClass().getName());
            throw new RuntimeException("Unexpected type for bundle response code: " + o.getClass().getName());
        }
    }

    private void sendBuyError(int error) {
        Bundle bundle = new Bundle();
        bundle.putString("action", Action.BUY.toString());

        bundle.putInt(IapCafebazaar.RESPONSE_CODE, error);
        Message msg = new Message();
        msg.setData(bundle);

        try {
            messenger.send(msg);
        } catch (RemoteException e) {
            Log.wtf(IapCafebazaar.TAG, "Unable to send message", e);
        }
        this.finish();
    }

    private void buy(String product, String productType) {
    	// Flush any pending items, in order to be able to buy the same (new) product again
    	processPendingConsumables();

        try {
            Bundle buyIntentBundle = service.getBuyIntent(3, getPackageName(), product, productType, "");
            int response = getResponseCodeFromBundle(buyIntentBundle);
            if (response == IapJNI.BILLING_RESPONSE_RESULT_OK) {
                hasPendingPurchases = true;
                PendingIntent pendingIntent = buyIntentBundle.getParcelable("BUY_INTENT");
                startIntentSenderForResult(pendingIntent.getIntentSender(), 1001, new Intent(), Integer.valueOf(0), Integer.valueOf(0), Integer.valueOf(0));
            } else if (response == IapJNI.BILLING_RESPONSE_RESULT_ITEM_ALREADY_OWNED) {
                sendBuyError(IapJNI.BILLING_RESPONSE_RESULT_ITEM_ALREADY_OWNED);
            } else {
                sendBuyError(response);
            }

        } catch (RemoteException e) {
            Log.e(IapCafebazaar.TAG, String.format("Failed to buy", e));
            sendBuyError(IapJNI.BILLING_RESPONSE_RESULT_ERROR);
        } catch (SendIntentException e) {
            Log.e(IapCafebazaar.TAG, String.format("Failed to buy", e));
            sendBuyError(IapJNI.BILLING_RESPONSE_RESULT_ERROR);
        }
    }

    private boolean consume(String purchaseData) {
        try {
            if (purchaseData == null) {
                Log.e(IapCafebazaar.TAG, String.format("Failed to consume purchase, purchaseData was null!"));
                return false;
            }

            JSONObject pd = new JSONObject(purchaseData);
            if (!pd.isNull("autoRenewing")) {
                Log.i(IapCafebazaar.TAG, "Will not consume purchase since it is a subscription.");
                return true;
            }
            String token = pd.getString("purchaseToken");
            int consumeResponse = service.consumePurchase(3, getPackageName(), token);
            if (consumeResponse == IapJNI.BILLING_RESPONSE_RESULT_OK) {
                return true;
            } else {
                Log.e(IapCafebazaar.TAG, String.format("Failed to consume purchase (%d)", consumeResponse));
                sendBuyError(consumeResponse);
            }
        } catch (RemoteException e) {
            Log.e(IapCafebazaar.TAG, "Failed to consume purchase", e);
            sendBuyError(IapJNI.BILLING_RESPONSE_RESULT_ERROR);
        } catch (JSONException e) {
            Log.e(IapCafebazaar.TAG, "Failed to consume purchase", e);
            sendBuyError(IapJNI.BILLING_RESPONSE_RESULT_ERROR);
        }
        return false;
    }

    private boolean processPurchase(String purchaseData, String signature)
    {
        if (this.autoFinishTransactions && !consume(purchaseData)) {
            Log.e(IapCafebazaar.TAG, "Failed to consume and send message");
            return false;
        }

        Bundle bundle = new Bundle();
        bundle.putString("action", Action.BUY.toString());
        bundle.putInt(IapCafebazaar.RESPONSE_CODE, IapJNI.BILLING_RESPONSE_RESULT_OK);
        bundle.putString(IapCafebazaar.RESPONSE_INAPP_PURCHASE_DATA, purchaseData);
        bundle.putString(IapCafebazaar.RESPONSE_INAPP_SIGNATURE, signature);

        Message msg = new Message();
        msg.setData(bundle);
        try {
            messenger.send(msg);
            return true;
        } catch (RemoteException e) {
            Log.wtf(IapCafebazaar.TAG, "Unable to send message", e);
            return false;
        }
    }

    // Make buy response codes for all consumables not yet processed.
    private void processPendingConsumables() {
        try {
            // Note: subscriptions cannot be consumed
            // https://developer.android.com/google/play/billing/api.html#subs
            Bundle items = service.getPurchases(3, getPackageName(), "inapp", null);
            int response = getResponseCodeFromBundle(items);
            if (response == IapJNI.BILLING_RESPONSE_RESULT_OK) {
                ArrayList<String> purchaseDataList = items.getStringArrayList(IapCafebazaar.RESPONSE_INAPP_PURCHASE_DATA_LIST);
                ArrayList<String> signatureList = items.getStringArrayList(IapCafebazaar.RESPONSE_INAPP_SIGNATURE_LIST);
                for (int i = 0; i < purchaseDataList.size(); ++i) {
                    String purchaseData = purchaseDataList.get(i);
                    String signature = signatureList.get(i);
                    if (!processPurchase(purchaseData, signature)) {
                        // abort and retry some other time
                        break;
                    }
                }
            }
        } catch (RemoteException e) {
            Log.e(IapCafebazaar.TAG, "Failed to process purchase", e);
        }
    }

    private void restore() {
        int response = IapJNI.BILLING_RESPONSE_RESULT_ERROR;
        Bundle bundle = new Bundle();
        bundle.putString("action", Action.RESTORE.toString());

        Bundle items = new Bundle();
        try {
            ArrayList<String> purchaseItemList = new ArrayList<String>();
            ArrayList<String> purchaseDataList = new ArrayList<String>();
            ArrayList<String> signatureList = new ArrayList<String>();

            Bundle inapp = service.getPurchases(3, getPackageName(), "inapp", null);
            if (getResponseCodeFromBundle(inapp) == IapJNI.BILLING_RESPONSE_RESULT_OK) {
                purchaseItemList.addAll(inapp.getStringArrayList(IapCafebazaar.RESPONSE_INAPP_ITEM_LIST));
                purchaseDataList.addAll(inapp.getStringArrayList(IapCafebazaar.RESPONSE_INAPP_PURCHASE_DATA_LIST));
                signatureList.addAll(inapp.getStringArrayList(IapCafebazaar.RESPONSE_INAPP_SIGNATURE_LIST));
            }

            Bundle subs = service.getPurchases(3, getPackageName(), "subs", null);
            if (getResponseCodeFromBundle(subs) == IapJNI.BILLING_RESPONSE_RESULT_OK) {
                purchaseItemList.addAll(subs.getStringArrayList(IapCafebazaar.RESPONSE_INAPP_ITEM_LIST));
                purchaseDataList.addAll(subs.getStringArrayList(IapCafebazaar.RESPONSE_INAPP_PURCHASE_DATA_LIST));
                signatureList.addAll(subs.getStringArrayList(IapCafebazaar.RESPONSE_INAPP_SIGNATURE_LIST));
            }

            items.putStringArrayList(IapCafebazaar.RESPONSE_INAPP_ITEM_LIST, purchaseItemList);
            items.putStringArrayList(IapCafebazaar.RESPONSE_INAPP_PURCHASE_DATA_LIST, purchaseDataList);
            items.putStringArrayList(IapCafebazaar.RESPONSE_INAPP_SIGNATURE_LIST, signatureList);
        } catch (RemoteException e) {
            Log.e(IapCafebazaar.TAG, "Failed to restore purchases", e);
        }
        bundle.putBundle("items", items);

        bundle.putInt(IapCafebazaar.RESPONSE_CODE, response);
        Message msg = new Message();
        msg.setData(bundle);

        try {
            messenger.send(msg);
        } catch (RemoteException e) {
            Log.wtf(IapCafebazaar.TAG, "Unable to send message", e);
        }
        this.finish();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        View view = new View(this);
        view.setBackgroundColor(0x10ffffff);
        setContentView(view, new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT));

        Intent intent = getIntent();
        final Bundle extras = intent.getExtras();
        this.messenger = (Messenger) extras.getParcelable(IapCafebazaar.PARAM_MESSENGER);
        final Action action = Action.valueOf(intent.getAction());
        this.autoFinishTransactions = extras.getBoolean(IapCafebazaar.PARAM_AUTOFINISH_TRANSACTIONS);

        Intent serviceIntent = new Intent("ir.cafebazaar.pardakht.InAppBillingService.BIND");
        serviceIntent.setPackage("com.farsitel.bazaar");
        List<ResolveInfo> intentServices = getPackageManager().queryIntentServices(serviceIntent, 0);
        if (intentServices != null && !intentServices.isEmpty()) {
            // service available to handle that Intent
            serviceConn = new ServiceConnection() {
                @Override
                public void onServiceDisconnected(ComponentName name) {
                    service = null;
                }

                @Override
                public void onServiceConnected(ComponentName name, IBinder serviceBinder) {
                    service = IInAppBillingService.Stub.asInterface(serviceBinder);
                    if (action == Action.BUY) {
                        buy(extras.getString(IapCafebazaar.PARAM_PRODUCT), extras.getString(IapCafebazaar.PARAM_PRODUCT_TYPE));
                    } else if (action == Action.RESTORE) {
                        restore();
                    } else if (action == Action.PROCESS_PENDING_CONSUMABLES) {
                        processPendingConsumables();
                        finish();
                    } else if (action == Action.FINISH_TRANSACTION) {
                        consume(extras.getString(IapCafebazaar.PARAM_PURCHASE_DATA));
                        finish();
                    }
                }
            };

            bindService(serviceIntent, serviceConn, Context.BIND_AUTO_CREATE);
        } else {
            // Service will never be connected; just send unavailability message
            Bundle bundle = new Bundle();
            bundle.putString("action", intent.getAction());
            bundle.putInt(IapCafebazaar.RESPONSE_CODE, IapJNI.BILLING_RESPONSE_RESULT_BILLING_UNAVAILABLE);
            Message msg = new Message();
            msg.setData(bundle);
            try {
                messenger.send(msg);
            } catch (RemoteException e) {
                Log.wtf(IapCafebazaar.TAG, "Unable to send message", e);
            }
            this.finish();
        }
    }

    @Override
    public void finish() {
        super.finish();
        this.isDone = true;
    }

    @Override
    protected void onDestroy() {
        if (hasPendingPurchases) {
            // Not sure connection is up so need to check here.
            if (service != null) {
                if(autoFinishTransactions) {
                    processPendingConsumables();
                }
            }
            hasPendingPurchases = false;
        }

        if( !isDone )
        {
            Intent intent = getIntent();

            if( intent != null && intent.getComponent().getClassName().equals( getClass().getName() ) )
            {
                Log.v(IapCafebazaar.TAG, "There's still an intent left: " + intent.getAction() );
                sendBuyError(IapJNI.BILLING_RESPONSE_RESULT_ERROR);
            }
        }

        if (serviceConn != null) {
            try
            {
                unbindService(serviceConn);
            } catch (IllegalArgumentException e) {
                Log.wtf(IapCafebazaar.TAG, "Unable to unbind service", e);
            }
        }
        super.onDestroy();
    }

    // NOTE: Code from "trivialdrivesample"
    int getResponseCodeFromIntent(Intent i) {
        Object o = i.getExtras().get(IapCafebazaar.RESPONSE_CODE);
        if (o == null) {
            Log.e(IapCafebazaar.TAG, "Intent with no response code, assuming OK (known issue)");
            return IapJNI.BILLING_RESPONSE_RESULT_OK;
        } else if (o instanceof Integer) {
            return ((Integer) o).intValue();
        } else if (o instanceof Long) {
            return (int) ((Long) o).longValue();
        } else {
            Log.e(IapCafebazaar.TAG, "Unexpected type for intent response code.");
            Log.e(IapCafebazaar.TAG, o.getClass().getName());
            throw new RuntimeException("Unexpected type for intent response code: " + o.getClass().getName());
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        Bundle bundle = null;
        if (data != null) {
            int responseCode = getResponseCodeFromIntent(data);
            String purchaseData = data.getStringExtra(IapCafebazaar.RESPONSE_INAPP_PURCHASE_DATA);
            String dataSignature = data.getStringExtra(IapCafebazaar.RESPONSE_INAPP_SIGNATURE);
            if (responseCode == IapJNI.BILLING_RESPONSE_RESULT_OK) {
                processPurchase(purchaseData, dataSignature);
            } else {
                 bundle = new Bundle();
                 bundle.putString("action", Action.BUY.toString());
                 bundle.putInt(IapCafebazaar.RESPONSE_CODE, responseCode);
                 bundle.putString(IapCafebazaar.RESPONSE_INAPP_PURCHASE_DATA, purchaseData);
                 bundle.putString(IapCafebazaar.RESPONSE_INAPP_SIGNATURE, dataSignature);
            }
        } else {
            bundle = new Bundle();
            bundle.putString("action", Action.BUY.toString());
            bundle.putInt(IapCafebazaar.RESPONSE_CODE, IapJNI.BILLING_RESPONSE_RESULT_ERROR);
        }

        // Send message if generated above
        if (bundle != null) {
            Message msg = new Message();
            msg.setData(bundle);
            try {
                messenger.send(msg);
            } catch (RemoteException e) {
                Log.wtf(IapCafebazaar.TAG, "Unable to send message", e);
            }
        }

        this.finish();
    }
}
