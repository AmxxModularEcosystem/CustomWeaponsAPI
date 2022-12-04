#if defined __UTILS_events_included
    #endinput
#endif
#define __UTILS_events_included

#include <amxmodx>

/*
Functions & Macroses:
    T_Events:Events_Init(const iEnum);
    T_Events:Events_Destroy(&T_Events:aEvents);

    Events_AddListener(const T_Events:aEvents, const any:iEvent, const iPluginId, const sCallback[], ...param_types);
    Events_PushListener(const T_Events:aEvents, const any:iEvent, const iForwardHandler);

    bool:Events_HasListeners(const T_Events:aEvents, const any:iEvent);

    Events_Call(const T_Events:aEvents, const any:iEvent);
    Events_CallP(const T_Events:aEvents, const any:iEvent, [...params_values]);

    Events_SetReturnedValue(const any:iRet);
    Events_GetReturnedValue();
*/

// Thx: https://dev-cs.ru/threads/222/page-12#post-103174
#define Events_CompositeMacros(%1) \
    do { %1 } while(is_linux_server() == 0xDEADBEEF)

enum T_Events { Invalid_Events = _:Invalid_Array }

stock T_Events:Events_Init(const any:iEnum) {
    new Array:aEvents = ArrayCreate(1, iEnum);
    for (new i = 0; i < iEnum; i++) {
        ArrayPushCell(aEvents, Invalid_Array);
    }

    return T_Events:aEvents;
}

stock T_Events:Events_Destroy(&T_Events:aEvents) {
    if (aEvents == Invalid_Events) {
        return Invalid_Events;
    }

    for (new i = 0; i < ArraySize(Array:aEvents); i++) {
        DestroyForward(ArrayGetCell(Array:aEvents, i));
    }
    ArrayDestroy(Array:aEvents);

    return Invalid_Events;
}

#define Events_AddListener(%1,%2,%3) \
    Events_PushListener(%1, %2, CreateOneForward(%3))

stock Events_PushListener(const T_Events:aEvents, const any:iEvent, const iForwardHandler) {
    if (aEvents == Invalid_Events) {
        return;
    }

    new Array:aListeners = ArrayGetCell(Array:aEvents, _:iEvent);
    if (aListeners == Invalid_Array) {
        aListeners = ArrayCreate(1, 1);
        ArraySetCell(Array:aEvents, _:iEvent, aListeners);
    }
    
    ArrayPushCell(aListeners, iForwardHandler);
}

stock Array:Events_GetListeners(const T_Events:aEvents, const any:iEvent) {
    if (aEvents == Invalid_Events) {
        return Invalid_Array;
    }

    return Array:ArrayGetCell(Array:aEvents, _:iEvent);
}

stock bool:Events_HasListeners(const T_Events:aEvents, const any:iEvent) {
    new Array:aListeners = Events_GetListeners(aEvents, iEvent);

    return (
        aListeners != Invalid_Array
        && ArraySize(aListeners) > 0
    );
}

// Events_ForeachListener(const T_Events:aEvents, const any:iEvent: aListeners[i] => iListener)
#define Events_ForeachListener(%1,%2:%3[%4]=>%5) \
    if (%1 != Invalid_Events) \
        for ( \
            new %4 = 0, Array:%3 = Events_GetListeners(%1, %2), %5; \
            ( \
                %4 < (%3 == Invalid_Array ? 0 : ArraySize(%3)) \
                && (%5 = ArrayGetCell(%3, %4)) \
            ); \
            %4++ \
        )

stock any:__Events_Call_ret;
#define Events_SetReturnedValue(%1) \
    __Events_Call_ret = %1

#define Events_GetReturnedValue() \
    __Events_Call_ret

#define Events_IsRet(%1) \
    (Events_GetReturnedValue() == %1)

#define Events_CallP(%1,%2,[%3]) Events_CompositeMacros( \
    Events_ForeachListener(%1, %2: __Events_Call_listeners[__Events_Call_i] => __Events_Call_listener) { \
        ExecuteForward(__Events_Call_listener, __Events_Call_ret, %3); \
    } \
)

#define Events_CallPWhile(%1,%2,[%3],%4) Events_CompositeMacros( \
    Events_ForeachListener(%1, %2: __Events_Call_listeners[__Events_Call_i] => __Events_Call_listener) { \
        ExecuteForward(__Events_Call_listener, __Events_Call_ret, %3); \
        if (!(%4)) { \
            break; \
        } \
    } \
)

#define Events_Call(%1,%2) Events_CompositeMacros( \
    Events_ForeachListener(%1, %2: __Events_Call_listeners[__Events_Call_i] => __Events_Call_listener) { \
        ExecuteForward(__Events_Call_listener, __Events_Call_ret); \
    } \
)

#define Events_CallWhile(%1,%2,%3) Events_CompositeMacros( \
    Events_ForeachListener(%1, %2: __Events_Call_listeners[__Events_Call_i] => __Events_Call_listener) { \
        ExecuteForward(__Events_Call_listener, __Events_Call_ret); \
        if (!(%3)) { \
            break; \
        } \
    } \
)
