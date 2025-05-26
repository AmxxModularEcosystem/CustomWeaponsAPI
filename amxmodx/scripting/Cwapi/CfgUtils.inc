#if defined __UTILS_CfgUtils_included
    #endinput
#endif
#define __UTILS_CfgUtils_included

#include <amxmodx>
#include <json>

#define JsonObject_ReadStringOr(%1,%2,%3,%4,%5) \
    if (json_object_has_value(%1, %2, JSONString, %5)) { \
        json_object_get_string(%1, %2, %3, %4, %5); \
    } else 
