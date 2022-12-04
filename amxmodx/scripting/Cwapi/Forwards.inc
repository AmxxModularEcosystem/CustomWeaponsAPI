#if defined __UTILS_forwards_included
    #endinput
#endif
#define __UTILS_forwards_included

/*
Author: ArKaNeMaN
Tg: t.me/arkaneman

Functions & Macroses:

    Forwards_Init(const sPrefix[]);
    []Forwards_GetFullName(const sName[]);
    Forwards_Reg(const sName[], const iStopType, const ...?param_types);
    Forwards_DefaultReturn(const iDefaultReturnValue);
    Forwards_Call(const sName[]);
    Forwards_CallP(const sName[], ...param_values);
    Forwards_GetHandler(const sName[]);
    Forwards_RegAndCallP(const sName[], const iStopType, [...param_types], [...param_values]);
    Forwards_RegAndCall(const sName[], const iStopType);
*/

#include <amxmodx>

stock const FORWARDS_NAME_TEMPLATE[] = "%s_On%s";

stock Trie:Forwards_gMap = Invalid_Trie;
stock Forwards_gPrefix[32];

stock Forwards_Init(const sPrefix[]) {
    Forwards_gMap = TrieCreate();
    copy(Forwards_gPrefix, charsmax(Forwards_gPrefix), sPrefix);
}

// []Forwards_GetFullName(const sName[]);
#define Forwards_GetFullName(%1) \
    fmt(FORWARDS_NAME_TEMPLATE, Forwards_gPrefix, %1)

// Forwards_Reg(const sName[], const iStopType, const ...?param_types);
#define Forwards_Reg(%1,%2) \
    TrieSetCell(Forwards_gMap, %1, CreateMultiForward(Forwards_GetFullName(%1), %2), false)

stock __Forwards_Call_ret;

// Forwards_DefaultReturn(const iDefaultReturnValue);
#define Forwards_DefaultReturn(%1) \
    __Forwards_Call_ret = %1

// Forwards_CallP(const sName[], ...param_values);
#define Forwards_CallP(%1,%2) \
    (ExecuteForward(Forwards_GetHandler(%1), __Forwards_Call_ret, %2) ? __Forwards_Call_ret : __Forwards_Call_ret)

// Forwards_Call(const sName[]);
#define Forwards_Call(%1) \
    (ExecuteForward(Forwards_GetHandler(%1), __Forwards_Call_ret) ? __Forwards_Call_ret : __Forwards_Call_ret)

stock Forwards_GetHandler(const sName[]) {
    new iHandler;
    TrieGetCell(Forwards_gMap, sName, iHandler);
    return iHandler;
}

stock __Forwards_RegAndCall_handler;

// Forwards_RegAndCallP(const sName[], const iStopType, [...param_types], [...param_values]);
#define Forwards_RegAndCallP(%1,%2,[%3],[%4]) \
    (DestroyForward( \
        ExecuteForward( \
            __Forwards_RegAndCall_handler = CreateMultiForward( \
                Forwards_GetFullName(%1), \
                %2, %3 \
            ), \
            __Forwards_Call_ret, %4 \
        ) ? __Forwards_RegAndCall_handler : __Forwards_RegAndCall_handler \
    ) ? __Forwards_Call_ret : __Forwards_Call_ret)

// Forwards_RegAndCall(const sName[], const iStopType);
#define Forwards_RegAndCall(%1,%2) \
    (DestroyForward( \
        ExecuteForward( \
            __Forwards_RegAndCall_handler = CreateMultiForward( \
                Forwards_GetFullName(%1), \
                %2 \
            ), \
            __Forwards_Call_ret \
        ) ? __Forwards_RegAndCall_handler : __Forwards_RegAndCall_handler \
    ) ? __Forwards_Call_ret : __Forwards_Call_ret)
