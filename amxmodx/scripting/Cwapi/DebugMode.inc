#if defined __debug_mode_included
    #endinput
#endif
#define __debug_mode_included

/*
    Author: ArKaNeMaN
    Tg: t.me/arkaneman
*/

#include <amxmodx>

#if !defined DEBUG
    #define DEBUG 0
#endif

static const bool:DEBUG_MODE_STATE = bool:DEBUG;

stock Dbg_SetState(const bool:bNewState) {
    DEBUG_MODE_STATE = bNewState;
}

stock bool:Dbg_GetState() {
    return DEBUG_MODE_STATE;
}
#define IS_DEBUG Dbg_GetState()

// Thx: https://dev-cs.ru/threads/222/page-12#post-103174 & Garey
stock __DebugMode_CompositeMacros__always_false = false;
#define DebugMode_CompositeMacros(%1) \
    do { %1 } while(__DebugMode_CompositeMacros__always_false)

#define DebugMode_OnlyInDebugMode(%1) \
    DebugMode_CompositeMacros( \
        if(IS_DEBUG) { \
            %1 \
        } \
    )

stock const __DEBUG_MODE_MESSAGE_TEMPLATE[] = "[DEBUG] %s";
#define DebugMode_MessageFormat(%1) \
    fmt(__DEBUG_MODE_MESSAGE_TEMPLATE, fmt(%1))

#define Dbg_PrintServer(%1) \
    DebugMode_OnlyInDebugMode( \
        server_print(DebugMode_MessageFormat(%1)); \
    )
// Dbg_PrintServer(const fmt[], any:...)

#define Dbg_PrintClientC(%1,%2,%3) \
    DebugMode_OnlyInDebugMode( \
        client_print_color(%1, %2, DebugMode_MessageFormat(%3)); \
    )
// Dbg_PrintClientC(const UserId, const SenderId, const fmt[], any:...)

#define Dbg_Log(%1) \
    DebugMode_OnlyInDebugMode( \
        log_amx(DebugMode_MessageFormat(%1)); \
    )
// Dbg_Log(const fmt[], any:...)
