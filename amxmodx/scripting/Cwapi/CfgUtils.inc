#if defined __UTILS_CfgUtils_included
    #endinput
#endif
#define __UTILS_CfgUtils_included

#include <amxmodx>
#include <json>
#include <ParamsController>

stock bool:CfgUtils_GetFileName(const sFilePath[], sOut[], const iOutLen) {
    new i = strlen(sFilePath) - 1;

    if (PCPath_IsPathSeparator(sFilePath[i])) {
        return false;
    }

    while (!PCPath_IsPathSeparator(sFilePath[i - 1])) {
        --i;
    }

    new j = 0, iLastDot = -1;
    while (j < iOutLen && sFilePath[i + j]) {
        sOut[j] = sFilePath[i + j];
        if (sOut[j] == '.') {
            iLastDot = j;
        }

        j++;
    }

    if (iLastDot >= 0 && iLastDot != j - 1) {
        sOut[iLastDot] = EOS;
    }

    return true;
}

#define JsonObject_ReadStringOr(%1,%2,%3,%4,%5) \
    if (json_object_has_value(%1, %2, JSONString, %5)) { \
        json_object_get_string(%1, %2, %3, %4, %5); \
    } else 

stock bool:JsonObject_ReadString(const JSON:jObject, const sKey[], sOut[], const iOutLen, const sDefault[] = "", const bool:bDotNot = false) {
    JsonObject_ReadStringOr(jObject, sKey, sOut, iOutLen, bDotNot) {
        copy(sOut, charsmax(iOutLen), sDefault);
        return false;
    }
    return true;
}

stock JsonObject_ReadInt(const JSON:jObject, const sKey[], const iDefault = 0, const bool:bDotNot = false) {
    if (!json_object_has_value(jObject, sKey, JSONNumber, bDotNot)) {
        return iDefault;
    }

    return json_object_get_number(jObject, sKey, bDotNot);
}

stock Float:JsonObject_ReadFloat(const JSON:jObject, const sKey[], const Float:fDefault = 0.0, const bool:bDotNot = false) {
    if (!json_object_has_value(jObject, sKey, JSONNumber, bDotNot)) {
        return fDefault;
    }

    return json_object_get_real(jObject, sKey, bDotNot);
}

stock bool:JsonObject_ReadBool(const JSON:jObject, const sKey[], const bool:bDefault = false, const bool:bDotNot = false) {
    if (!json_object_has_value(jObject, sKey, JSONBoolean, bDotNot)) {
        return bDefault;
    }

    return json_object_get_bool(jObject, sKey, bDotNot);
}

enum E_JsonUtilsReadStatus {
    JsonUtilsRead_Success,
    JsonUtilsRead_FieldIsEmpty,
    JsonUtilsRead_PrecacheFileNotFound,
}

stock bool:JsonObject_ReadAndPrecacheModel(const JSON:jObject, const sKey[], sOut[], const iOutLen, const bool:bDotNot = false) {
    JsonObject_ReadStringOr(jObject, sKey, sOut, iOutLen, bDotNot) {
        return true;
    }

    if (!file_exists(sOut)) {
        // sOut[0] = EOS;
        return false;
    }

    precache_model(sOut);
    return true;
}

stock E_JsonUtilsReadStatus:JsonObject_ReadAndPrecacheSound(const JSON:jObject, const sKey[], sOut[], const iOutLen, const bool:bDotNot = false) {
    sOut[0] = EOS;
    JsonObject_ReadStringOr(jObject, sKey, sOut, iOutLen, bDotNot) {
        return JsonUtilsRead_FieldIsEmpty;
    }

    if (!file_exists(fmt("sound/%s", sOut))) {
        return JsonUtilsRead_PrecacheFileNotFound;
    }

    precache_sound(sOut);
    return JsonUtilsRead_Success;
}
