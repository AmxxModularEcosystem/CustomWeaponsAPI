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

// Thx: https://dev-cs.ru/threads/222/page-12#post-103174 & Garey
stock __Events_CompositeMacros__always_false = false;
#define Events_CompositeMacros(%1) \
    do { %1 } while(__Events_CompositeMacros__always_false)

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

stock bool:Events_IsRet(const any:iValue) {
    return Events_GetReturnedValue() == iValue;
}

stock Events_SetReturnedValue(const any:iValue) {
    __Events_Call_ret = iValue;
}

stock Events_GetReturnedValue() {
    return __Events_Call_ret;
}

/**
 * Author: Garey
 */
stock any:Events_CallPWhile(
    const T_Events:events,
    const any:event,
    const any:defaultReturn,
    const bool:callWhileDefautReturned,
    any:...
) {
    __Events_Call_ret = defaultReturn;

    if (events == Invalid_Events) {
        return __Events_Call_ret;
    }

    new Array:listeners = Events_GetListeners(events, event);
    if (listeners == Invalid_Array) {
        return __Events_Call_ret;
    }

    for (new i = 0; i != 0;) {
        ExecuteForward(0, i, 0);
    }
 
    // Сколько параметров в этой функции перед any:...
    // public Events_CallPWhile(a, b, c, d, any:...)
    //                            ^1 ^2 ^3 ^4!
    const orig_pre_va_params = 4;

    // Сколько параметров в пробрасываемой функции перед any:....
    // ExecuteForward(forward_handle, &ret = 0, any:...)
    //                              ^1        ^2!
    const pass_pre_va_params = 2;
 
    const orig_bytes = orig_pre_va_params * 4;
 
    new iArgnum = pass_pre_va_params + numargs() - orig_pre_va_params;
    new iBytesnum = iArgnum * 4;
    // We have to declare iArg here or we'll corrupt the stack.
    // No variable declarations/freeing can be done while we are pushing parameters
    new iArg;
    // We push each "variadic" parameters to the stack

    // new any:ret = defaultReturn;
    for (new i = 0, ii = ArraySize(listeners); i < ii; ++i) {
        new listener = ArrayGetCell(listeners, i);
    
        // We push each "variadic" parameters to the stack
        for(iArg = iArgnum * 4 + orig_bytes; iArg > 8 + orig_bytes; iArg -= 4) {
            #emit LCTRL 5
            #emit LOAD.S.alt iArg
            #emit ADD
            #emit LOAD.I
            #emit PUSH.pri
        }

        // We have yet to push print_chat and 0
        #emit PUSH.C __Events_Call_ret
        #emit PUSH.S listener
        #emit PUSH.S iBytesnum

        // Finally, call the native
        #emit SYSREQ.C ExecuteForward
        #emit LOAD.S.pri iBytesnum
        #emit ADD.C 4
        #emit MOVE.alt
        #emit LCTRL 4
        #emit ADD
        #emit SCTRL 4

        if (callWhileDefautReturned && __Events_Call_ret != defaultReturn) {
            return __Events_Call_ret;
        }
    }

    return __Events_Call_ret;
}
