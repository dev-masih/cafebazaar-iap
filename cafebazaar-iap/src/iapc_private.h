#if defined(DM_PLATFORM_ANDROID)

#ifndef IAPC_PRIVATE_H
#define IAPC_PRIVATE_H

#include <dmsdk/sdk.h>

struct IAPCListener
{
    IAPCListener()
    {
        m_L = 0;
        m_Callback = LUA_NOREF;
        m_Self = LUA_NOREF;
    }
    lua_State* m_L;
    int        m_Callback;
    int        m_Self;
};


char* IAPC_List_CreateBuffer(lua_State* L);
void IAPC_PushError(lua_State* L, const char* error, int reason);
void IAPC_PushConstants(lua_State* L);

#endif

#endif // DM_PLATFORM_ANDROID
