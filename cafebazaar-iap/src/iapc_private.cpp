#if defined(DM_PLATFORM_ANDROID)

#include <dmsdk/sdk.h>

#include "iapc.h"
#include "iapc_private.h"
#include <string.h>
#include <stdlib.h>


IAPCListener::IAPCListener() {
    IAPC_ClearCallback(this);
}

void IAPC_ClearCallback(IAPCListener* callback)
{
    callback->m_L = 0;
    callback->m_Callback = LUA_NOREF;
    callback->m_Self = LUA_NOREF;
}

void IAPC_RegisterCallback(lua_State* L, int index, IAPCListener* callback)
{
    luaL_checktype(L, index, LUA_TFUNCTION);
    lua_pushvalue(L, index);
    callback->m_Callback = dmScript::Ref(L, LUA_REGISTRYINDEX);

    dmScript::GetInstance(L);
    callback->m_Self = dmScript::Ref(L, LUA_REGISTRYINDEX);

    callback->m_L = L;
}

void IAPC_UnregisterCallback(IAPCListener* callback)
{
    if (LUA_NOREF != callback->m_Callback)
        dmScript::Unref(callback->m_L, LUA_REGISTRYINDEX, callback->m_Callback);
    if (LUA_NOREF != callback->m_Self)
        dmScript::Unref(callback->m_L, LUA_REGISTRYINDEX, callback->m_Self);
    callback->m_Callback = LUA_NOREF;
    callback->m_Self = LUA_NOREF;
    callback->m_L = 0;
}

bool IAPC_SetupCallback(IAPCListener* callback)
{
    lua_State* L = callback->m_L;
    lua_rawgeti(L, LUA_REGISTRYINDEX, callback->m_Callback);

    // Setup self
    lua_rawgeti(L, LUA_REGISTRYINDEX, callback->m_Self);
    lua_pushvalue(L, -1);
    dmScript::SetInstance(L);

    if (!dmScript::IsInstanceValid(L))
    {
        dmLogError("Could not run callback because the instance has been deleted.");
        lua_pop(L, 2);
        return false;
    }
    return true;
}

bool IAPC_CallbackIsValid(IAPCListener* callback)
{
    return callback != 0 && callback->m_L != 0 && callback->m_Callback != LUA_NOREF && callback->m_Self != LUA_NOREF;
}

// Creates a comma separated string, given a table where all values are strings (or numbers)
// Returns a malloc'ed string, which the caller must free
char* IAPC_List_CreateBuffer(lua_State* L)
{
    int top = lua_gettop(L);

    luaL_checktype(L, 1, LUA_TTABLE);
    lua_pushnil(L);
    int length = 0;
    while (lua_next(L, 1) != 0) {
        if (length > 0) {
            ++length;
        }
        const char* p = lua_tostring(L, -1);
        if(!p)
        {
            luaL_error(L, "IAPC: Failed to get value (string) from table");
        }
        length += strlen(p);
        lua_pop(L, 1);
    }

    char* buf = (char*)malloc(length+1);
    if( buf == 0 )
    {
        dmLogError("Could not allocate buffer of size %d", length+1);
        assert(top == lua_gettop(L));
        return 0;
    }
    buf[0] = '\0';

    int i = 0;
    lua_pushnil(L);
    while (lua_next(L, 1) != 0) {
        if (i > 0) {
            dmStrlCat(buf, ",", length+1);
        }
        const char* p = lua_tostring(L, -1);
        if(!p)
        {
            luaL_error(L, "IAPC: Failed to get value (string) from table");
        }
        dmStrlCat(buf, p, length+1);
        lua_pop(L, 1);
        ++i;
    }

    assert(top == lua_gettop(L));
    return buf;
}

void IAPC_PushError(lua_State* L, const char* error, int reason)
{
    if (error != 0) {
        lua_newtable(L);
        lua_pushstring(L, "error");
        lua_pushstring(L, error);
        lua_rawset(L, -3);
        lua_pushstring(L, "reason");
        lua_pushnumber(L, reason);
        lua_rawset(L, -3);
    } else {
        lua_pushnil(L);
    }
}

void IAPC_PushConstants(lua_State* L)
{
    #define SETCONSTANT(name) \
            lua_pushnumber(L, (lua_Number) name); \
            lua_setfield(L, -2, #name);\

        SETCONSTANT(TRANS_STATE_PURCHASING)
        SETCONSTANT(TRANS_STATE_PURCHASED)
        SETCONSTANT(TRANS_STATE_FAILED)
        SETCONSTANT(TRANS_STATE_RESTORED)
        SETCONSTANT(TRANS_STATE_UNVERIFIED)

        SETCONSTANT(REASON_UNSPECIFIED)
        SETCONSTANT(REASON_USER_CANCELED)

    #undef SETCONSTANT
}


void IAPC_Queue_Create(IAPCCommandQueue* queue)
{
    queue->m_Mutex = dmMutex::New();
}

void IAPC_Queue_Destroy(IAPCCommandQueue* queue)
{
    dmMutex::Delete(queue->m_Mutex);
}

void IAPC_Queue_Push(IAPCCommandQueue* queue, IAPCCommand* cmd)
{
    DM_MUTEX_SCOPED_LOCK(queue->m_Mutex);

    if(queue->m_Commands.Full())
    {
        queue->m_Commands.OffsetCapacity(2);
    }
    queue->m_Commands.Push(*cmd);
}

void IAPC_Queue_Flush(IAPCCommandQueue* queue, IAPCCommandFn fn, void* ctx)
{
    assert(fn != 0);
    if (queue->m_Commands.Empty())
    {
        return;
    }

    DM_MUTEX_SCOPED_LOCK(queue->m_Mutex);

    for(uint32_t i = 0; i != queue->m_Commands.Size(); ++i)
    {
        fn(&queue->m_Commands[i], ctx);
    }
    queue->m_Commands.SetSize(0);
}

#endif // DM_PLATFORM_ANDROID
