#if defined __UTILS_CfgUtils_included
    #endinput
#endif
#define __UTILS_CfgUtils_included

#include <amxmodx>
#include <json>

stock static g_sCfgFolder[32];

static stock CfgUtils__IsPathSeparator(const iChar) {
    return (
        iChar == '/'
        || iChar == '\'
    );
}

stock CfgUtils_SetFolder(const sFolder[]) {
    new start = 0;
    while (CfgUtils__IsPathSeparator(sFolder[start])) {
        ++start;
    }

    new end = copy(g_sCfgFolder, charsmax(g_sCfgFolder), sFolder[start]);

    while (CfgUtils__IsPathSeparator(g_sCfgFolder[end - 1])) {
        --end;
    }
    g_sCfgFolder[end] = EOS;
}

stock CfgUtils_MakePath(const sFilePath[], sOut[], const iOutLen) {
    static __amxx_configsdir[PLATFORM_MAX_PATH];
    if (!__amxx_configsdir[0]) {
        get_localinfo("amxx_configsdir", __amxx_configsdir, charsmax(__amxx_configsdir));
    }

    if (CfgUtils__IsPathSeparator(sFilePath[0])) {
        return formatex(sOut, iOutLen, "%s%s", __amxx_configsdir, sFilePath)
    } else {
        return formatex(sOut, iOutLen, "%s/plugins/%s/%s", __amxx_configsdir, g_sCfgFolder, sFilePath);
    }
}

stock CfgUtils_GetPath(const sFilePath[]) {
    new sPath[PLATFORM_MAX_PATH];
    CfgUtils_MakePath(sFilePath, sPath, charsmax(sPath));
    return sPath;
}

stock bool:CfgUtils_GetFileName(const sFilePath[], sOut[], const iOutLen) {
    new i = strlen(sFilePath) - 1;

    if (CfgUtils__IsPathSeparator(sFilePath[i])) {
        return false;
    }

    while (!CfgUtils__IsPathSeparator(sFilePath[i - 1])) {
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

stock JSON:CfgUtils_GetJson(const sFilePath[], const bool:bWithComments = false, const bool:bAbsolutePath = false) {
    // TODO: Сделать обёртку как в випке (хотя мб и не понадобится...)
    return json_parse(bAbsolutePath ? sFilePath : fmt("%s.json", CfgUtils_GetPath(sFilePath)), true, bWithComments);
}

stock bool:CfgUtils_JsonFree(&JSON:jValue) {
    // Буду сразу использовать эту чистилку, чтобы не переделывать потом для обёртки
    return json_free(jValue);
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

stock bool:JsonObject_ReadAndPrecacheModel(const JSON:jObject, const sKey[], sOut[], const iOutLen, const bool:bDotNot = false) {
    JsonObject_ReadStringOr(jObject, sKey, sOut, iOutLen, bDotNot) {
        return false;
    }

    if (!file_exists(sOut)) {
        sOut[0] = EOS;
        return false;
    }

    precache_model(sOut);

    return true;
}
