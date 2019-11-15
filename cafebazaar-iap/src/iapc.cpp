#if defined(DM_PLATFORM_ANDROID)

#include <dmsdk/sdk.h>
#include <jni.h>
#include <stdlib.h>
#include <unistd.h>
#include "iapc.h"
#include "iapc_private.h"

#define LIB_NAME "iapc"

static JNIEnv* Attach()
{
    JNIEnv* env;
    dmGraphics::GetNativeAndroidJavaVM()->AttachCurrentThread(&env, NULL);
    return env;
}

static void Detach()
{
    dmGraphics::GetNativeAndroidJavaVM()->DetachCurrentThread();
}


struct IAPC
{
    IAPC()
    {
        memset(this, 0, sizeof(*this));
        m_Callback = LUA_NOREF;
        m_Self = LUA_NOREF;
        m_Listener.m_Callback = LUA_NOREF;
        m_Listener.m_Self = LUA_NOREF;
        m_autoFinishTransactions = true;
    }
    int                  m_InitCount;
    int                  m_Callback;
    int                  m_Self;
    bool                 m_autoFinishTransactions;
    lua_State*           m_L;
    IAPCListener         m_Listener;

    jobject              m_IAPC;
    jobject              m_IAPCJNI;
    jmethodID            m_List;
    jmethodID            m_Stop;
    jmethodID            m_Buy;
    jmethodID            m_Restore;
    jmethodID            m_ProcessPendingConsumables;
    jmethodID            m_FinishTransaction;

    IAPCCommandQueue m_CommandQueue;
};

static IAPC g_IAPC;

static void ResetCallback(lua_State* L)
{
    if (g_IAPC.m_Callback != LUA_NOREF) {
        dmScript::Unref(L, LUA_REGISTRYINDEX, g_IAPC.m_Callback);
        dmScript::Unref(L, LUA_REGISTRYINDEX, g_IAPC.m_Self);
        g_IAPC.m_Callback = LUA_NOREF;
        g_IAPC.m_Self = LUA_NOREF;
        g_IAPC.m_L = 0;
    }
}

static int IAPC_List(lua_State* L)
{
    int top = lua_gettop(L);
    ResetCallback(L);

    char* buf = IAPC_List_CreateBuffer(L);
    if( buf == 0 )
    {
        assert(top == lua_gettop(L));
        return 0;
    }

    luaL_checktype(L, 2, LUA_TFUNCTION);
    lua_pushvalue(L, 2);
    g_IAPC.m_Callback = dmScript::Ref(L, LUA_REGISTRYINDEX);

    dmScript::GetInstance(L);
    g_IAPC.m_Self = dmScript::Ref(L, LUA_REGISTRYINDEX);

    g_IAPC.m_L = dmScript::GetMainThread(L);

    JNIEnv* env = Attach();
    jstring products = env->NewStringUTF(buf);
    env->CallVoidMethod(g_IAPC.m_IAPC, g_IAPC.m_List, products, g_IAPC.m_IAPCJNI);
    env->DeleteLocalRef(products);
    Detach();

    free(buf);
    assert(top == lua_gettop(L));
    return 0;
}

static int IAPC_Buy(lua_State* L)
{
    int top = lua_gettop(L);

    const char* id = luaL_checkstring(L, 1);

    JNIEnv* env = Attach();
    jstring ids = env->NewStringUTF(id);
    env->CallVoidMethod(g_IAPC.m_IAPC, g_IAPC.m_Buy, ids, g_IAPC.m_IAPCJNI);
    env->DeleteLocalRef(ids);
    Detach();

    assert(top == lua_gettop(L));
    return 0;
}

static int IAPC_Finish(lua_State* L)
{
    if(g_IAPC.m_autoFinishTransactions)
    {
        dmLogWarning("Calling iapc.finish when autofinish transactions is enabled. Ignored.");
        return 0;
    }

    int top = lua_gettop(L);

    luaL_checktype(L, 1, LUA_TTABLE);

    lua_getfield(L, -1, "state");
    if (lua_isnumber(L, -1))
    {
        if(lua_tointeger(L, -1) != TRANS_STATE_PURCHASED)
        {
            dmLogError("Invalid transaction state (must be iapc.TRANS_STATE_PURCHASED).");
            lua_pop(L, 1);
            assert(top == lua_gettop(L));
            return 0;
        }
    }
    lua_pop(L, 1);

    lua_getfield(L, -1, "receipt");
    if (!lua_isstring(L, -1)) {
        dmLogError("Transaction error. Invalid transaction data, does not contain 'receipt' key.");
        lua_pop(L, 1);
    }
    else
    {
        const char * receipt = lua_tostring(L, -1);
        lua_pop(L, 1);

        JNIEnv* env = Attach();
        jstring receiptUTF = env->NewStringUTF(receipt);
        env->CallVoidMethod(g_IAPC.m_IAPC, g_IAPC.m_FinishTransaction, receiptUTF, g_IAPC.m_IAPCJNI);
        env->DeleteLocalRef(receiptUTF);
        Detach();
    }

    assert(top == lua_gettop(L));
    return 0;
}

static int IAPC_Restore(lua_State* L)
{
    // TODO: Missing callback here for completion/error

    int top = lua_gettop(L);
    JNIEnv* env = Attach();
    env->CallVoidMethod(g_IAPC.m_IAPC, g_IAPC.m_Restore, g_IAPC.m_IAPCJNI);
    Detach();

    assert(top == lua_gettop(L));

    lua_pushboolean(L, 1);
    return 1;
}

static int IAPC_SetListener(lua_State* L)
{
    IAPC* iapc = &g_IAPC;
    luaL_checktype(L, 1, LUA_TFUNCTION);
    lua_pushvalue(L, 1);
    int cb = dmScript::Ref(L, LUA_REGISTRYINDEX);

    bool had_previous = false;
    if (iapc->m_Listener.m_Callback != LUA_NOREF) {
        dmScript::Unref(iapc->m_Listener.m_L, LUA_REGISTRYINDEX, iapc->m_Listener.m_Callback);
        dmScript::Unref(iapc->m_Listener.m_L, LUA_REGISTRYINDEX, iapc->m_Listener.m_Self);
        had_previous = true;
    }

    iapc->m_Listener.m_L = dmScript::GetMainThread(L);
    iapc->m_Listener.m_Callback = cb;

    dmScript::GetInstance(L);
    iapc->m_Listener.m_Self = dmScript::Ref(L, LUA_REGISTRYINDEX);

    // On first set listener, trigger process old ones.
    if (!had_previous) {
        JNIEnv* env = Attach();
        env->CallVoidMethod(g_IAPC.m_IAPC, g_IAPC.m_ProcessPendingConsumables, g_IAPC.m_IAPCJNI);
        Detach();
    }
    return 0;
}

static const luaL_reg IAPC_methods[] =
{
    {"list", IAPC_List},
    {"buy", IAPC_Buy},
    {"finish", IAPC_Finish},
    {"restore", IAPC_Restore},
    {"set_listener", IAPC_SetListener},
    {0, 0}
};


#ifdef __cplusplus
extern "C" {
#endif


JNIEXPORT void JNICALL Java_com_defold_iapc_IapJNI_onProductsResult__ILjava_lang_String_2(JNIEnv* env, jobject, jint responseCode, jstring productList)
{
    const char* pl = 0;
    if (productList)
    {
        pl = env->GetStringUTFChars(productList, 0);
    }

    IAPCCommand cmd;
    cmd.m_Command = IAPC_PRODUCT_RESULT;
    cmd.m_ResponseCode = responseCode;
    if (pl)
    {
        cmd.m_Data = strdup(pl);
        env->ReleaseStringUTFChars(productList, pl);
    }
    IAPC_Queue_Push(&g_IAPC.m_CommandQueue, &cmd);
}

JNIEXPORT void JNICALL Java_com_defold_iapc_IapJNI_onPurchaseResult__ILjava_lang_String_2(JNIEnv* env, jobject, jint responseCode, jstring purchaseData)
{
    const char* pd = 0;
    if (purchaseData)
    {
        pd = env->GetStringUTFChars(purchaseData, 0);
    }

    IAPCCommand cmd;
    cmd.m_Command = IAPC_PURCHASE_RESULT;
    cmd.m_ResponseCode = responseCode;
    if (pd)
    {
        cmd.m_Data = strdup(pd);
        env->ReleaseStringUTFChars(purchaseData, pd);
    }
    IAPC_Queue_Push(&g_IAPC.m_CommandQueue, &cmd);
}

#ifdef __cplusplus
}
#endif

static void HandleProductResult(const IAPCCommand* cmd)
{
    lua_State* L = g_IAPC.m_L;
    int top = lua_gettop(L);

    if (g_IAPC.m_Callback == LUA_NOREF) {
        dmLogError("No callback set");
        return;
    }

    lua_rawgeti(L, LUA_REGISTRYINDEX, g_IAPC.m_Callback);

    // Setup self
    lua_rawgeti(L, LUA_REGISTRYINDEX, g_IAPC.m_Self);
    lua_pushvalue(L, -1);
    dmScript::SetInstance(L);

    if (!dmScript::IsInstanceValid(L))
    {
        dmLogError("Could not run IAPC callback because the instance has been deleted.");
        lua_pop(L, 2);
        assert(top == lua_gettop(L));
        return;
    }

    if (cmd->m_ResponseCode == BILLING_RESPONSE_RESULT_OK) {
        dmJson::Document doc;
        dmJson::Result r = dmJson::Parse((const char*) cmd->m_Data, &doc);
        if (r == dmJson::RESULT_OK && doc.m_NodeCount > 0) {
            char err_str[128];
            if (dmScript::JsonToLua(L, &doc, 0, err_str, sizeof(err_str)) < 0) {
                dmLogError("Failed converting product result JSON to Lua; %s", err_str);
                lua_pushnil(L);
                IAPC_PushError(L, "failed to convert JSON to Lua for product response", REASON_UNSPECIFIED);
            } else {
                lua_pushnil(L);
            }
        } else {
            dmLogError("Failed to parse product response (%d)", r);
            lua_pushnil(L);
            IAPC_PushError(L, "failed to parse product response", REASON_UNSPECIFIED);
        }
        dmJson::Free(&doc);
    } else {
        dmLogError("IAPC error %d", cmd->m_ResponseCode);
        lua_pushnil(L);
        IAPC_PushError(L, "failed to fetch product", REASON_UNSPECIFIED);
    }

    int ret = lua_pcall(L, 3, 0, 0);
    if (ret != 0) {
        dmLogError("Error running callback: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
    }

    dmScript::Unref(L, LUA_REGISTRYINDEX, g_IAPC.m_Callback);
    dmScript::Unref(L, LUA_REGISTRYINDEX, g_IAPC.m_Self);
    g_IAPC.m_Callback = LUA_NOREF;
    g_IAPC.m_Self = LUA_NOREF;

    assert(top == lua_gettop(L));
}

static void HandlePurchaseResult(const IAPCCommand* cmd)
{
    lua_State* L = g_IAPC.m_Listener.m_L;
    int top = lua_gettop(L);

    if (g_IAPC.m_Listener.m_Callback == LUA_NOREF) {
        dmLogError("No callback set");
        return;
    }


    lua_rawgeti(L, LUA_REGISTRYINDEX, g_IAPC.m_Listener.m_Callback);

    // Setup self
    lua_rawgeti(L, LUA_REGISTRYINDEX, g_IAPC.m_Listener.m_Self);
    lua_pushvalue(L, -1);
    dmScript::SetInstance(L);

    if (!dmScript::IsInstanceValid(L))
    {
        dmLogError("Could not run IAPC callback because the instance has been deleted.");
        lua_pop(L, 2);
        assert(top == lua_gettop(L));
        return;
    }

    if (cmd->m_ResponseCode == BILLING_RESPONSE_RESULT_OK) {
        if (cmd->m_Data != 0) {
            dmJson::Document doc;
            dmJson::Result r = dmJson::Parse((const char*) cmd->m_Data, &doc);
            if (r == dmJson::RESULT_OK && doc.m_NodeCount > 0) {
                char err_str[128];
                if (dmScript::JsonToLua(L, &doc, 0, err_str, sizeof(err_str)) < 0) {
                    dmLogError("Failed converting purchase JSON result to Lua; %s", err_str);
                    lua_pushnil(L);
                    IAPC_PushError(L, "failed to convert purchase response JSON to Lua", REASON_UNSPECIFIED);
                } else {
                    lua_pushnil(L);
                }
            } else {
                dmLogError("Failed to parse purchase response (%d)", r);
                lua_pushnil(L);
                IAPC_PushError(L, "failed to parse purchase response", REASON_UNSPECIFIED);
            }
            dmJson::Free(&doc);
        } else {
            dmLogError("IAPC error, purchase response was null");
            lua_pushnil(L);
            IAPC_PushError(L, "purchase response was null", REASON_UNSPECIFIED);
        }
    } else if (cmd->m_ResponseCode == BILLING_RESPONSE_RESULT_USER_CANCELED) {
        lua_pushnil(L);
        IAPC_PushError(L, "user canceled purchase", REASON_USER_CANCELED);
    } else {
        dmLogError("IAPC error %d", cmd->m_ResponseCode);
        lua_pushnil(L);
        IAPC_PushError(L, "failed to buy product", REASON_UNSPECIFIED);
    }

    int ret = lua_pcall(L, 3, 0, 0);
    if (ret != 0) {
        dmLogError("Error running callback: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
    }

    assert(top == lua_gettop(L));
}

static dmExtension::Result InitializeIAPC(dmExtension::Params* params)
{
    // TODO: Life-cycle managaemnt is *budget*. No notion of "static initalization"
    // Extend extension functionality with per system initalization?
    if (g_IAPC.m_InitCount == 0) {
        IAPC_Queue_Create(&g_IAPC.m_CommandQueue);

        g_IAPC.m_autoFinishTransactions = dmConfigFile::GetInt(params->m_ConfigFile, "iap.auto_finish_transactions", 1) == 1;

        JNIEnv* env = Attach();

        jclass activity_class = env->FindClass("android/app/NativeActivity");
        jmethodID get_class_loader = env->GetMethodID(activity_class,"getClassLoader", "()Ljava/lang/ClassLoader;");
        jobject cls = env->CallObjectMethod(dmGraphics::GetNativeAndroidActivity(), get_class_loader);
        jclass class_loader = env->FindClass("java/lang/ClassLoader");
        jmethodID find_class = env->GetMethodID(class_loader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");

        const char* class_name = "com.defold.iapc.IapCafebazaar";
        jstring str_class_name = env->NewStringUTF(class_name);

        jclass iapc_class = (jclass)env->CallObjectMethod(cls, find_class, str_class_name);
        env->DeleteLocalRef(str_class_name);

        str_class_name = env->NewStringUTF("com.defold.iapc.IapJNI");
        jclass iapc_jni_class = (jclass)env->CallObjectMethod(cls, find_class, str_class_name);
        env->DeleteLocalRef(str_class_name);

        g_IAPC.m_List = env->GetMethodID(iapc_class, "listItems", "(Ljava/lang/String;Lcom/defold/iapc/IListProductsListener;)V");
        g_IAPC.m_Buy = env->GetMethodID(iapc_class, "buy", "(Ljava/lang/String;Lcom/defold/iapc/IPurchaseListener;)V");
        g_IAPC.m_Restore = env->GetMethodID(iapc_class, "restore", "(Lcom/defold/iapc/IPurchaseListener;)V");
        g_IAPC.m_Stop = env->GetMethodID(iapc_class, "stop", "()V");
        g_IAPC.m_ProcessPendingConsumables = env->GetMethodID(iapc_class, "processPendingConsumables", "(Lcom/defold/iapc/IPurchaseListener;)V");
        g_IAPC.m_FinishTransaction = env->GetMethodID(iapc_class, "finishTransaction", "(Ljava/lang/String;Lcom/defold/iapc/IPurchaseListener;)V");

        jmethodID jni_constructor = env->GetMethodID(iapc_class, "<init>", "(Landroid/app/Activity;Z)V");
        g_IAPC.m_IAPC = env->NewGlobalRef(env->NewObject(iapc_class, jni_constructor, dmGraphics::GetNativeAndroidActivity(), g_IAPC.m_autoFinishTransactions));

        jni_constructor = env->GetMethodID(iapc_jni_class, "<init>", "()V");
        g_IAPC.m_IAPCJNI = env->NewGlobalRef(env->NewObject(iapc_jni_class, jni_constructor));

        Detach();
    }
    g_IAPC.m_InitCount++;

    lua_State*L = params->m_L;
    int top = lua_gettop(L);
    luaL_register(L, LIB_NAME, IAPC_methods);

    IAPC_PushConstants(L);

    lua_pop(L, 1);
    assert(top == lua_gettop(L));

    return dmExtension::RESULT_OK;
}

static void IAPC_OnCommand(IAPCCommand* cmd, void*)
{
    switch (cmd->m_Command)
    {
    case IAPC_PRODUCT_RESULT:
        HandleProductResult(cmd);
            break;
    case IAPC_PURCHASE_RESULT:
        HandlePurchaseResult(cmd);
            break;

        default:
            assert(false);
        }

    if (cmd->m_Data) {
        free(cmd->m_Data);
        }
}

static dmExtension::Result UpdateIAPC(dmExtension::Params* params)
{
    IAPC_Queue_Flush(&g_IAPC.m_CommandQueue, IAPC_OnCommand, 0);
    return dmExtension::RESULT_OK;
}

static dmExtension::Result FinalizeIAPC(dmExtension::Params* params)
{
    IAPC_Queue_Destroy(&g_IAPC.m_CommandQueue);
    --g_IAPC.m_InitCount;

    if (params->m_L == g_IAPC.m_Listener.m_L && g_IAPC.m_Listener.m_Callback != LUA_NOREF) {
        dmScript::Unref(g_IAPC.m_Listener.m_L, LUA_REGISTRYINDEX, g_IAPC.m_Listener.m_Callback);
        dmScript::Unref(g_IAPC.m_Listener.m_L, LUA_REGISTRYINDEX, g_IAPC.m_Listener.m_Self);
        g_IAPC.m_Listener.m_L = 0;
        g_IAPC.m_Listener.m_Callback = LUA_NOREF;
        g_IAPC.m_Listener.m_Self = LUA_NOREF;
    }

    if (g_IAPC.m_InitCount == 0) {
        JNIEnv* env = Attach();
        env->CallVoidMethod(g_IAPC.m_IAPC, g_IAPC.m_Stop);
        env->DeleteGlobalRef(g_IAPC.m_IAPC);
        env->DeleteGlobalRef(g_IAPC.m_IAPCJNI);
        Detach();
        g_IAPC.m_IAPC = NULL;
    }
    return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(IAPCExt, "IAPC", 0, 0, InitializeIAPC, UpdateIAPC, 0, FinalizeIAPC)

#endif //DM_PLATFORM_ANDROID

