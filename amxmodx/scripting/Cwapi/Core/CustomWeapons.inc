#if defined __cwapi_CustomWeapons_included
    #endinput
#endif
#define __cwapi_CustomWeapons_included

#include <amxmodx>
#include <reapi>
#include <hamsandwich>
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

enum S_CWeapon_Sounds {
    CWeapon_Sound_Shot[PLATFORM_MAX_PATH],
    CWeapon_Sound_ShotSilenced[PLATFORM_MAX_PATH],
    CWeapon_Sound_ShotBurst[PLATFORM_MAX_PATH],
}

enum _:S_CustomWeapon {
    T_CustomWeapon:CWeapon_Index,
    CWeapon_Reference[32],
    WeaponIdType:CWeapon_ReferenceId,
    CWeapon_Name[CWAPI_WEAPON_NAME_MAX_LEN],
    CWeapon_Models[S_CWeapon_Models],
    bool:CWeapon_HasCustomSounds,
    CWeapon_Sounds[S_CWeapon_Sounds],
    Float:CWeapon_DamageMult,
    CWeapon_Weight,
    CWeapon_MaxClip,
    CWeapon_MaxAmmo,
    Float:CWeapon_DeployTime,
    Float:CWeapon_ReloadTime,
    bool:CWeapon_Grenade_PreventFITH,

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

any:CWeapons_GetProp(const T_CustomWeapon:iWeapon, const /*S_CustomWeapon:*/iPropKey) {
    if (!CWeapons_IsValidIndex(iWeapon)) {
        ThrowError("Invalid custom weapon index (%d).", iWeapon);
        return 0;
    }

    return ArrayMapGetiCell(gCustomWeapons, iWeapon, iPropKey);
}

static T_CustomWeapon:CWeapons_GetWeaponFromEntity(const entityIndex) {
    if (is_nullent(entityIndex)) {
        return Invalid_CustomWeapon;
    }

    return CWeapons_GetWeaponFromImpulse(get_entvar(entityIndex, var_impulse));
}

T_CustomWeapon:CWeapons_GetWeaponFromImpulse(const impulse) {
    new T_CustomWeapon:weapon = T_CustomWeapon:(impulse - CWAPI_IMPULSE_OFFSET);
    if (!CWeapons_IsValidIndex(weapon)) {
        return Invalid_CustomWeapon;
    }

    return weapon;
}

CWeapons_GetImpulse(const T_CustomWeapon:weapon) {
    return (_:weapon + CWAPI_IMPULSE_OFFSET);
}

T_CustomWeapon:CWeapons_FindByItem(const ItemId) {
    return CWeapons_GetWeaponFromEntity(ItemId);
}

bool:CWeapons_GetByItem(const ItemId, Weapon[S_CustomWeapon], &T_CustomWeapon:iWeapon = Invalid_CustomWeapon) {
    iWeapon = CWeapons_FindByItem(ItemId);
    if (iWeapon == Invalid_CustomWeapon) {
        return false;
    }
    
    return CWeapons_Get(iWeapon, Weapon);
}

T_CustomWeapon:CWeapons_FindByInflictor(const inflictorIndex) {
    new entityIndex = 0;
    if (FClassnameIs(inflictorIndex, "player")) {
        entityIndex = get_member(inflictorIndex, m_pActiveItem);
        Dbg_Log("CWeapons_FindByInflictor: Player %d has active item %d", inflictorIndex, entityIndex);
    } else if (FClassnameIs(inflictorIndex, "grenade")) {
        entityIndex = inflictorIndex;
        Dbg_Log("CWeapons_FindByInflictor: Grenade %d", inflictorIndex);
    } else {
        Dbg_Log("CWeapons_FindByInflictor: Unknown inflictor %d", inflictorIndex);
    }

    return CWeapons_GetWeaponFromEntity(entityIndex);
}

bool:CWeapons_GetByInflictor(const inflictorIndex, weaponObject[S_CustomWeapon], &T_CustomWeapon:weapon = Invalid_CustomWeapon) {
    weapon = CWeapons_FindByInflictor(inflictorIndex);
    if (weapon == Invalid_CustomWeapon) {
        return false;
    }
    
    return CWeapons_Get(weapon, weaponObject);
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
    const CWeapon_GiveType:iGiveType = CWAPI_GT_SMART,
    const bool:bForceActive = false,
    const initBpAmmo = -1
) {
    new Weapon[S_CustomWeapon];
    CWeapons_Get(iWeapon, Weapon);

    new GiveType:iResolvedGiveType = CWeapons__ResolveGiveType(Weapon[CWeapon_ReferenceId], iGiveType);
    Dbg_Log("CWeapons_Give(%d, %d, %d, %s, %d): iResolvedGiveType = %d", UserId, iWeapon, iGiveType, bForceActive ? "true" : "false", initBpAmmo, iResolvedGiveType);
    
    // TODO: CWeapon_Call(CWeapon_OnGive)

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
        CWeapons_GetImpulse(iWeapon)
    );

    if (is_nullent(ItemId) && IsWeaponGrenade(Weapon[CWeapon_ReferenceId])) {
        Dbg_Log("CWeapons_Give(%d, %d, %d, %s, %d): Find exists grenade (%d)", UserId, iWeapon, iGiveType, bForceActive ? "true" : "false", initBpAmmo, ItemId);
        ItemId = rg_find_weapon_bpack_by_name(UserId, Weapon[CWeapon_Reference]);
    }

    Dbg_Log("CWeapons_Give(%d, %d, %d, %s, %d): ItemId = %d", UserId, iWeapon, iGiveType, bForceActive ? "true" : "false", initBpAmmo, ItemId);

    if (!is_nullent(ItemId)) {
        FillBpAmmoByItem(UserId, ItemId, initBpAmmo);
        
        if (bForceActive) {
            rg_switch_weapon(UserId, ItemId);
        }
    }

    return ItemId;
}

Float:CWeapons_EmitDamage(
    const T_CustomWeapon:weaponIndex,
    const victimIndex,
    const inflictorIndex,
    const attackerIndex,
    const Float:damage,
    const damageBits = DMG_GENERIC,
    const itemIndex = 0,
    const bool:ignoreDamageMult = false
) {
    new weaponObject[S_CustomWeapon];
    CWeapons_Get(weaponIndex, weaponObject);

    new Float:finalDamage = damage;
    new finalDamageBits = damageBits;

    if (!ignoreDamageMult) {
        finalDamage *= weaponObject[CWeapon_DamageMult];
    }

    CWeapons_Call(weaponObject, CWeapon_OnDamage, [weaponIndex, itemIndex, victimIndex, inflictorIndex, attackerIndex, finalDamage, finalDamageBits]);
    if (Events_IsRet(CWAPI_STOP_MAIN)) {
        return 0.0;
    }

    new res = ExecuteHamB(Ham_TakeDamage, victimIndex, inflictorIndex, attackerIndex, finalDamage, finalDamageBits);
    
    return res > 0 ? finalDamage : 0.0;
}

Trie:CWeapons_GetAbilityParams(const T_CustomWeapon:weaponIndex, const T_WeaponAbility:ability) {
    static weaponObject[S_CustomWeapon];
    CWeapons_Get(weaponIndex, weaponObject);

    if (weaponObject[CWeapon_Abilities] == Invalid_Array) {
        return Invalid_Trie;
    }

    for (new i = 0, ii = ArraySize(weaponObject[CWeapon_Abilities]); i < ii; ++i) {
        new T_WAbility_Unit:unit = ArrayGetCell(weaponObject[CWeapon_Abilities], i);

        if (unit == Invalid_WAbility_Unit) {
            continue;
        }

        static unitObject[S_WAbility_Unit];
        WAbilityUnit_Get(unit, unitObject);

        if (unitObject[WAbilityUnit_Ability] != ability) {
            continue;
        }

        // todo стоит ли клонировать trie?
        return unitObject[WAbilityUnit_Params];
    }

    return Invalid_Trie;
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

    if (IsWeaponGrenade(WeaponData[CWeapon_ReferenceId])) {
        WeaponData[CWeapon_Grenade_PreventFITH] = JsonObject_ReadBool(jWeapon, "Grenade.PreventFITH", false, true);
    }

    new Models[S_CWeapon_Models];
    if (!JsonObject_ReadAndPrecacheModel(
        jWeapon, "Models.View",
        Models[CWeapon_Model_View], charsmax(Models[CWeapon_Model_View]),
        .bDotNot = true
    )) {
        log_amx("[WARNING] Model file '%s' not found. File '%s'.", Models[CWeapon_Model_View], CfgUtils_GetPath(sFile));
        Models[CWeapon_Model_View][0] = EOS;
    }
    
    if (!JsonObject_ReadAndPrecacheModel(
        jWeapon, "Models.World",
        Models[CWeapon_Model_World], charsmax(Models[CWeapon_Model_World]),
        .bDotNot = true
    )) {
        log_amx("[WARNING] Model file '%s' not found. File '%s'.", Models[CWeapon_Model_World], CfgUtils_GetPath(sFile));
        Models[CWeapon_Model_World][0] = EOS;
    }
    
    if (!JsonObject_ReadAndPrecacheModel(
        jWeapon, "Models.Player",
        Models[CWeapon_Model_Player], charsmax(Models[CWeapon_Model_Player]),
        .bDotNot = true
    )) {
        log_amx("[WARNING] Model file '%s' not found. File '%s'.", Models[CWeapon_Model_Player], CfgUtils_GetPath(sFile));
        Models[CWeapon_Model_Player][0] = EOS;
    }

    WeaponData[CWeapon_Models] = Models;
    
    CWeapons__ReadSounds(jWeapon, WeaponData, CfgUtils_GetPath(sFile));

    if (json_object_has_value(jWeapon, "Abilities", JSONObject)) {
        new JSON:jAbilities = json_object_get_value(jWeapon, "Abilities");
        WeaponData[CWeapon_Abilities] = WAbilityUnit_ReadJsonObject(jAbilities);
        CfgUtils_JsonFree(jAbilities);
    }
    
    CfgUtils_JsonFree(jWeapon);

    return CWeapons__Create(WeaponData);
}

static CWeapons__ReadSounds(const JSON:jWeapon, WeaponData[S_CustomWeapon], const sFileNameForErrors[]) {
    WeaponData[CWeapon_HasCustomSounds] = false;
    if (!json_object_has_value(jWeapon, "Sounds", JSONObject)) {
        return;
    }

    new JSON:jSounds = json_object_get_value(jWeapon, "Sounds");
    if (!json_object_has_value(jSounds, "Shot", JSONString)) {
        log_amx("[ERROR] 'Sounds.Shot' field required if 'Sounds' field exists. File '%s'.", sFileNameForErrors);
        return;
    }

    new Sounds[S_CWeapon_Sounds];

    if (JsonObject_ReadAndPrecacheSound(
        jSounds, "Shot",
        Sounds[CWeapon_Sound_Shot], charsmax(Sounds[CWeapon_Sound_Shot])
    ) == JsonUtilsRead_PrecacheFileNotFound) {
        log_amx("[ERROR] Sound file '%s' not found. File '%s'.", Sounds[CWeapon_Sound_Shot], sFileNameForErrors);
        return;
    }
    
    switch (JsonObject_ReadAndPrecacheSound(jSounds, "ShotSilenced", Sounds[CWeapon_Sound_ShotSilenced], charsmax(Sounds[CWeapon_Sound_ShotSilenced]))) {
        case JsonUtilsRead_PrecacheFileNotFound: {
            log_amx("[WARNING] Sound file '%s' not found. File '%s'.", Sounds[CWeapon_Sound_ShotSilenced], sFileNameForErrors);
            Sounds[CWeapon_Sound_ShotSilenced][0] = EOS;
        }
        case JsonUtilsRead_FieldIsEmpty: {
            Sounds[CWeapon_Sound_ShotSilenced] = Sounds[CWeapon_Sound_Shot];
        }
    }
    
    switch (JsonObject_ReadAndPrecacheSound(jSounds, "ShotBurst", Sounds[CWeapon_Sound_ShotBurst], charsmax(Sounds[CWeapon_Sound_ShotBurst]))) {
        case JsonUtilsRead_PrecacheFileNotFound: {
            log_amx("[WARNING] Sound file '%s' not found. File '%s'.", Sounds[CWeapon_Sound_ShotBurst], sFileNameForErrors);
            Sounds[CWeapon_Sound_ShotBurst][0] = EOS;
        }
        case JsonUtilsRead_FieldIsEmpty: {
            Sounds[CWeapon_Sound_ShotBurst] = Sounds[CWeapon_Sound_Shot];
        }
    }

    WeaponData[CWeapon_Sounds] = Sounds;
    WeaponData[CWeapon_HasCustomSounds] = true;
}

CWeapons_LoadFromFolder(const sFolder[]) {
    Dbg_Log("CWeapons_LoadFromFolder('%s'): Call func.", sFolder);

    new sFile[PLATFORM_MAX_PATH], iDirHandler, FileType:iType;
    iDirHandler = open_dir(CfgUtils_GetPath(sFolder), sFile, charsmax(sFile), iType);
    if (!iDirHandler) {
        ThrowError("[ERROR] Can't open folder '%s'.", CfgUtils_GetPath(sFolder));
        return;
    }
    
    Dbg_Log("CWeapons_LoadFromFolder('%s'): Read weapons from folder '%s'.", sFolder, CfgUtils_GetPath(sFolder));

    new Regex:iRegEx_FileName, ret;
    iRegEx_FileName = regex_compile("(.+).json$", ret, "", 0, "i");

    do {
        if (sFile[0] == '!' || sFile[0] == '.') {
            continue;
        }

        if (iType == FileType_Directory) {
            CWeapons_LoadFromFolder(fmt("%s/%s", sFolder, sFile));
        } else if (iType == FileType_File) {
            if (regex_match_c(sFile, iRegEx_FileName) > 0) {
                regex_substr(iRegEx_FileName, 1, sFile, charsmax(sFile));
                CWeapons_LoadFromCfg(fmt("%s/%s", sFolder, sFile));
            }
        }
    } while (next_file(iDirHandler, sFile, charsmax(sFile), iType));

    regex_free(iRegEx_FileName);
    close_dir(iDirHandler);
}

#include "Cwapi/Core/CWeapons/Hooks"
#include "Cwapi/Core/CWeapons/Natives"
