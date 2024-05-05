#if defined __cwapi_CustomWeapons_included
    #endinput
#endif
#define __cwapi_CustomWeapons_included

#include <amxmodx>
#include <reapi>
#include <json>
#include <regex>
#include <cwapi>
#include "Cwapi/ArrayMap"
#include "Cwapi/Events"
#include "Cwapi/Utils"
#include "Cwapi/ArrayTrieUtils"
#include "Cwapi/CfgUtils"

enum S_CWeapon_Models {
    CWeapon_Model_View[PLATFORM_MAX_PATH],
    CWeapon_Model_World[PLATFORM_MAX_PATH],
    CWeapon_Model_Player[PLATFORM_MAX_PATH],
}

enum _:S_CustomWeapon {
    T_CustomWeapon:CWeapon_Index,
    CWeapon_Reference[32],
    WeaponIdType:CWeapon_ReferenceId,
    CWeapon_Name[CWAPI_WEAPON_NAME_MAX_LEN],
    CWeapon_Models[S_CWeapon_Models],
    Float:CWeapon_DamageMult,
    CWeapon_Weight,
    CWeapon_MaxClip,
    CWeapon_MaxAmmo,
    Float:CWeapon_DeployTime,
    Float:CWeapon_ReloadTime,

    // Float:CWeapon_MaxWalkSpeed,
    // Float:CWeapon_PrimaryAttackRate,
    // Float:CWeapon_SecondaryAttackRate,
    // bool:CWeapon_HasSecondaryAttack,
    // bool:CWeapon_HasCustomHud,

    T_Events:CWeapon_Events,
    Array:CWeapon_Abilities, // T_WAbility_Unit[]
}

#include "Cwapi/Core/CWeapons/Abilities"

static ArrayMap(gCustomWeapons);
static bool:g_bIsInited = false;

#define CWeapons_Call(%1,%2,[%3]) CompositeMacros( \
    CWeapons_CallSelf(%1, %2, [%3]); \
    if (Events_IsRet(CWAPI_CONTINUE)) { \
        CWeapons_CallAbilities(%1, %2, [%3]); \
    } \
)

#define CWeapons_CallSelf(%1,%2,[%3]) CompositeMacros( \
    Events_SetReturnedValue(CWAPI_CONTINUE); \
    Events_CallPWhile(%1[CWeapon_Events], %2, [%3, Invalid_Trie], Events_IsRet(CWAPI_CONTINUE)); \
)

#define CWeapons_CallAbilities(%1,%2,[%3]) CompositeMacros( \
    for (new __CWeapons_CallAbilities_i = 0; __CWeapons_CallAbilities_i < ArraySizeSafe(%1[CWeapon_Abilities]); ++__CWeapons_CallAbilities_i) { \
        new __CWeapons_CallAbilities_iAbilityUnit = ArrayGetCell(%1[CWeapon_Abilities], __CWeapons_CallAbilities_i); \
        new __CWeapons_CallAbilities_AbilityUnit[S_WAbility_Unit]; \
        WAbilityUnit_Get(T_WAbility_Unit:__CWeapons_CallAbilities_iAbilityUnit, __CWeapons_CallAbilities_AbilityUnit); \
        WAbilityUnit_Call(__CWeapons_CallAbilities_AbilityUnit, %2, [%3]); \
    } \
)

CWeapons_Init() {
    CallOnce();
    g_bIsInited = true;

    InitArrayMap(gCustomWeapons, S_CustomWeapon, 4);
    CWeapons_Hooks_Init();
    WAbility_Init();
}

bool:CWeapons_IsInited() {
    return g_bIsInited;
}

CWeapons_Count() {
    return ArrayMapSize(gCustomWeapons);
}

T_CustomWeapon:CWeapons_Iterate(const T_CustomWeapon:iWeapon, Weapon[S_CustomWeapon]) {
    new iNextWeapon = _:iWeapon + 1;

    // Пушки кончились
    if (iNextWeapon >= CWeapons_Count()) {
        return Invalid_CustomWeapon;
    }

    CWeapons_Get(T_CustomWeapon:iNextWeapon, Weapon);

    return T_CustomWeapon:iNextWeapon;
}

static T_CustomWeapon:CWeapons__Create(const WeaponData[S_CustomWeapon]) {
    // TODO: Is ref weapon name valid
    // if () {
    //     return Invalid_CustomWeapon;
    // }

    new Weapon[S_CustomWeapon];
    Weapon = WeaponData;

    Weapon[CWeapon_Index] = T_CustomWeapon:ArrayMapSize(gCustomWeapons);
    Weapon[CWeapon_Events] = Events_Init(E_CWeapon_Event);
    CWeapons_Hooks_RegForWeapon(Weapon[CWeapon_Reference], Weapon[CWeapon_Name]);

    new T_CustomWeapon:iCustomWeapon = T_CustomWeapon:ArrayMapPushArray(gCustomWeapons, Weapon, Weapon[CWeapon_Name]);

    // Такое вообще может случиться?)
    if (iCustomWeapon != Weapon[CWeapon_Index]) {
        log_amx("ChZH?!");
        return Invalid_CustomWeapon;
    }

    return iCustomWeapon;
}

bool:CWeapons_IsValidIndex(const any:iWeapon) {
    return (
        iWeapon >= 0
        && iWeapon < CWeapons_Count()
    );
}

bool:CWeapons_IsValidName(const sWeaponName[]) {
    return ArrayMapHasKey(gCustomWeapons, sWeaponName);
}

T_CustomWeapon:CWeapons_Find(const sWeaponName[]) {
    if (!CWeapons_IsValidName(sWeaponName)) {
        return Invalid_CustomWeapon;
    }
    
    return T_CustomWeapon:ArrayMapGetIndex(gCustomWeapons, sWeaponName);
}

bool:CWeapons_GetByName(const sWeaponName[], Weapon[S_CustomWeapon]) {
    if (!CWeapons_IsValidName(sWeaponName)) {
        return false;
    }
    
    ArrayMapGetArray(gCustomWeapons, sWeaponName, Weapon);
    return true;
}

bool:CWeapons_Get(const T_CustomWeapon:iWeapon, Weapon[S_CustomWeapon]) {
    if (!CWeapons_IsValidIndex(iWeapon)) {
        ThrowError("Invalid custom weapon index (%d).", iWeapon);
        return false;
    }

    ArrayMapGetiArray(gCustomWeapons, iWeapon, Weapon);
    return true;
}

static T_CustomWeapon:CWeapons__GetItemIndex(const ItemId) {
    return T_CustomWeapon:(get_entvar(ItemId, var_impulse) - CWAPI_IMPULSE_OFFSET);
}

bool:CWeapons_IsValidItem(const ItemId) {
    if (is_nullent(ItemId)) {
        return false;
    }

    return CWeapons_IsValidIndex(CWeapons__GetItemIndex(ItemId));
}

T_CustomWeapon:CWeapons_FindByItem(const ItemId) {
    if (!CWeapons_IsValidItem(ItemId)) {
        return Invalid_CustomWeapon;
    }
    
    return CWeapons__GetItemIndex(ItemId);
}

bool:CWeapons_GetByItem(const ItemId, Weapon[S_CustomWeapon], &T_CustomWeapon:iWeapon = Invalid_CustomWeapon) {
    iWeapon = CWeapons_FindByItem(ItemId);
    if (iWeapon == Invalid_CustomWeapon) {
        return false;
    }
    
    return CWeapons_Get(iWeapon, Weapon);
}

CWeapons_AddEventListener(
    const T_CustomWeapon:iWeapon,
    const E_CWeapon_Event:iEvent,
    const iPlugin,
    const sCallback[]
) {
    new Weapon[S_CustomWeapon];
    CWeapons_Get(iWeapon, Weapon);

    Events_PushListener(Weapon[CWeapon_Events], iEvent, CWeaponUtils_MakeEventCallback(iEvent, iPlugin, sCallback));
}

static GiveType:CWeapons__ResolveGiveType(
    const WeaponIdType:WeaponId,
    const CWeapon_GiveType:iGiveType = CWAPI_GT_SMART
) {
    if (iGiveType != CWAPI_GT_SMART) {
        return GiveType:iGiveType;
    }
    
    if (WeaponId == WEAPON_KNIFE) {
        return GT_REPLACE;
    } else if (WEAPON_GRENADE & BIT(_:WeaponId)) {
        return GT_APPEND;
    } else {
        return GT_DROP_AND_REPLACE;
    }
}

CWeapons_Give(
    const UserId,
    const T_CustomWeapon:iWeapon,
    const CWeapon_GiveType:iGiveType = CWAPI_GT_SMART
) {
    new Weapon[S_CustomWeapon];
    CWeapons_Get(iWeapon, Weapon);

    // CWeapons_Call(...);

    new GiveType:iResolvedGiveType = CWeapons__ResolveGiveType(Weapon[CWeapon_ReferenceId], iGiveType);
    Dbg_Log("CWeapons_Give(%d, %d, %d): Resolved give type - %d", UserId, iWeapon, iGiveType, iResolvedGiveType);

    // Удаление/выбрасывание референсного оружия, если оно есть
    // Мб как-то можно более просто это сделать?)
    if (rg_has_item_by_name(UserId, Weapon[CWeapon_Reference])) {
        if (iResolvedGiveType == GT_DROP_AND_REPLACE) {
            rg_drop_item(UserId, Weapon[CWeapon_Reference]);
        } else {
            rg_remove_item(UserId, Weapon[CWeapon_Reference]);
        }
    }

    new ItemId = rg_give_custom_item(
        UserId,
        Weapon[CWeapon_Reference],
        iResolvedGiveType,
        _:iWeapon + CWAPI_IMPULSE_OFFSET
    );

    Dbg_Log("CWeapons_Give(%d, %d, %d): Item's index - %d", UserId, iWeapon, iGiveType, ItemId);

    if (!is_nullent(ItemId)) {
        FillBpAmmoByItem(UserId, ItemId);
    }

    return ItemId;
}

T_CustomWeapon:CWeapons_LoadFromCfg(const sFile[], const bool:bAbsolutePath = false) {
    new JSON:jWeapon = CfgUtils_GetJson(sFile, true, bAbsolutePath);

    if (jWeapon == Invalid_JSON) {
        ThrowError("[ERROR] Invalid JSON syntax. File '%s'.", CfgUtils_GetPath(sFile));
        return Invalid_CustomWeapon;
    }

    if (!json_is_object(jWeapon)) {
        // TODO: Прикрутить оболочку жисона и слать ошибки через неё
        ThrowError("[ERROR] Invalid config structure. File '%s'.", CfgUtils_GetPath(sFile));
        CfgUtils_JsonFree(jWeapon);
        return Invalid_CustomWeapon;
    }

    new WeaponData[S_CustomWeapon];

    JsonObject_ReadStringOr(jWeapon, "Name", WeaponData[CWeapon_Name], charsmax(WeaponData[CWeapon_Name]), false) {
        CfgUtils_GetFileName(sFile, WeaponData[CWeapon_Name], charsmax(WeaponData[CWeapon_Name]));
    }

    Dbg_Log("CWeapons_LoadFromCfg('%s', %s): Read weapon '%s'.", sFile, bAbsolutePath ? "true" : "false", WeaponData[CWeapon_Name]);

    JsonObject_ReadStringOr(jWeapon, "Reference", WeaponData[CWeapon_Reference], charsmax(WeaponData[CWeapon_Reference]), false) {
        ThrowError("[ERROR] Invalid config structure. File '%s'.", CfgUtils_GetPath(sFile));
        CfgUtils_JsonFree(jWeapon);
        return Invalid_CustomWeapon;
    }
    WeaponData[CWeapon_ReferenceId] = GetWeaponIdByName(WeaponData[CWeapon_Reference]);

    WeaponData[CWeapon_MaxClip] = JsonObject_ReadInt(jWeapon, "MaxClip", -1);
    WeaponData[CWeapon_MaxAmmo] = JsonObject_ReadInt(jWeapon, "MaxAmmo", -1);
    WeaponData[CWeapon_Weight] = JsonObject_ReadInt(jWeapon, "Weight", -1);
    WeaponData[CWeapon_DamageMult] = JsonObject_ReadFloat(jWeapon, "DamageMult", 1.0);
    WeaponData[CWeapon_DeployTime] = JsonObject_ReadFloat(jWeapon, "DeployTime", -1.0);
    WeaponData[CWeapon_ReloadTime] = JsonObject_ReadFloat(jWeapon, "ReloadTime", -1.0);

    new Models[S_CWeapon_Models];
    if (!JsonObject_ReadAndPrecacheModel(
        jWeapon, "Models.View",
        Models[CWeapon_Model_View], charsmax(Models[CWeapon_Model_View]),
        .bDotNot = true
    )) {
        log_amx("[WARNING] Model file '%s' not found. File '%s'.", Models[CWeapon_Model_View], CfgUtils_GetPath(sFile));
    }
    
    if (!JsonObject_ReadAndPrecacheModel(
        jWeapon, "Models.World",
        Models[CWeapon_Model_World], charsmax(Models[CWeapon_Model_World]),
        .bDotNot = true
    )) {
        log_amx("[WARNING] Model file '%s' not found. File '%s'.", Models[CWeapon_Model_World], CfgUtils_GetPath(sFile));
    }
    
    if (!JsonObject_ReadAndPrecacheModel(
        jWeapon, "Models.Player",
        Models[CWeapon_Model_Player], charsmax(Models[CWeapon_Model_Player]),
        .bDotNot = true
    )) {
        log_amx("[WARNING] Model file '%s' not found. File '%s'.", Models[CWeapon_Model_Player], CfgUtils_GetPath(sFile));
    }

    WeaponData[CWeapon_Models] = Models;

    if (json_object_has_value(jWeapon, "Abilities", JSONObject)) {
        new JSON:jAbilities = json_object_get_value(jWeapon, "Abilities");
        WeaponData[CWeapon_Abilities] = WAbilityUnit_ReadJsonObject(jAbilities);
        CfgUtils_JsonFree(jAbilities);
    }
    
    CfgUtils_JsonFree(jWeapon);

    return CWeapons__Create(WeaponData);
}

CWeapons_LoadFromFolder(const sFolder[]) {
    Dbg_Log("CWeapons_LoadFromFolder('%s'): Call func.", sFolder);

    new sFile[PLATFORM_MAX_PATH], iDirHandler, FileType:iType;
    iDirHandler = open_dir(CfgUtils_GetPath(sFolder), sFile, charsmax(sFile), iType);
    if (!iDirHandler) {
        ThrowError("[ERROR] Can't open folder '%s'.", CfgUtils_GetPath(sFolder));
        return;
    }
    
    Dbg_Log("CWeapons_LoadFromFolder('%s'): Read weapons from folder '%s'.", sFolder, sFolder);

    new Regex:iRegEx_FileName, ret;
    iRegEx_FileName = regex_compile("(.+).json$", ret, "", 0, "i");

    do {
        if (
            sFile[0] == '!'
            || iType != FileType_File
            || regex_match_c(sFile, iRegEx_FileName) <= 0
        ) {
            continue;
        }

        regex_substr(iRegEx_FileName, 1, sFile, charsmax(sFile));
        CWeapons_LoadFromCfg(fmt("%s/%s", sFolder, sFile));

    } while (next_file(iDirHandler, sFile, charsmax(sFile), iType));

    regex_free(iRegEx_FileName);
    close_dir(iDirHandler);
}

#include "Cwapi/Core/CWeapons/Hooks"
#include "Cwapi/Core/CWeapons/Natives"
