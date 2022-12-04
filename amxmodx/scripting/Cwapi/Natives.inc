#if defined __UTILS_natives_included
    #endinput
#endif
#define __UTILS_natives_included

/*
Author: ArKaNeMaN
Tg: t.me/arkaneman

Functions & Macroses:

    Natives_Init(const sPrefix[]);
    Natives_Reg(const sName[]);
    Natives_AutoReg(const sPrefix[], const aNativeNames[][]);
*/

#include <amxmodx>

stock const __NATIVE_NAME_TPL[] = "%s_%s";
stock const __NATIVE_FUNC_TPL[] = "@_%s";
stock Natives_gPrefix[32];

#define Natives_Init(%1) \
    copy(Natives_gPrefix, charsmax(Natives_gPrefix), %1)

// Natives_Reg(const sName[]);
#define Natives_Reg(%1) \
    register_native(fmt(__NATIVE_NAME_TPL, Natives_gPrefix, %1), fmt(__NATIVE_FUNC_TPL, %1))
