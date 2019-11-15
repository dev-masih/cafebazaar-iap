#if defined(DM_PLATFORM_ANDROID)

#ifndef IAPC_PRIVATE_H
#define IAPC_PRIVATE_H

#include <dmsdk/sdk.h>

// TODO: Rename Callback
struct IAPCListener
{
    IAPCListener();

    lua_State* m_L;
    int        m_Callback;
    int        m_Self;
};

enum EIAPCCommand
{
	IAPC_PRODUCT_RESULT,
	IAPC_PURCHASE_RESULT,
};

struct DM_ALIGNED(16) IAPCCommand
{
    IAPCCommand()
    {
        memset(this, 0, sizeof(IAPCCommand));
    }

    // Used for storing eventual callback info (if needed)
    IAPCListener m_Callback;

    // THe actual command payload
    int32_t  	m_Command;
    int32_t  	m_ResponseCode;
    void*    	m_Data;
};

struct IAPCCommandQueue
{
    dmArray<IAPCCommand>  m_Commands;
    dmMutex::HMutex      m_Mutex;
};

void IAPC_ClearCallback(IAPCListener* callback);
void IAPC_RegisterCallback(lua_State* L, int index, IAPCListener* callback);
void IAPC_UnregisterCallback(IAPCListener* callback);
bool IAPC_SetupCallback(IAPCListener* callback);
bool IAPC_CallbackIsValid(IAPCListener* callback);

char* IAPC_List_CreateBuffer(lua_State* L);
void IAPC_PushError(lua_State* L, const char* error, int reason);
void IAPC_PushConstants(lua_State* L);

typedef void (*IAPCCommandFn)(IAPCCommand* cmd, void* ctx);

void IAPC_Queue_Create(IAPCCommandQueue* queue);
void IAPC_Queue_Destroy(IAPCCommandQueue* queue);
// The command is copied by value into the queue
void IAPC_Queue_Push(IAPCCommandQueue* queue, IAPCCommand* cmd);
void IAPC_Queue_Flush(IAPCCommandQueue* queue, IAPCCommandFn fn, void* ctx);

#endif

#endif // DM_PLATFORM_ANDROID
