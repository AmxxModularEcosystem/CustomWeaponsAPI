#include <amxmodx>
#include <reapi>
#include <json>
#include "Cwapi/ArrayMap"
#include "Cwapi/Events"
#include "Cwapi/Utils"
#include "Cwapi/CfgUtils"

enum S_CWeapon_Models {
    CWeapon_Model_View[PLATFORM_MAX_PATH],
    CWeapon_Model_World[PLATFORM_MAX_PATH],
    CWeapon_Model_Player[PLATFORM_MAX_PATH],
}

enum _:S_CustomWeapon {
    CWeapon_Reference[32],
    CWeapon_Name[CWAPI_WEAPON_NAME_MAX_LEN],
    CWeapon_Models[S_CWeapon_Models],
    Float:CWeapon_DamageMult,
    CWeapon_Weight,
    CWeapon_MaxClip,
    CWeapon_MaxAmmo,

    // Float:CWeapon_MaxWalkSpeed,
    // Float:CWeapon_DeployTime,
    // Float:CWeapon_ReloadTime,
    // Float:CWeapon_PrimaryAttackRate,
    // Float:CWeapon_SecondaryAttackRate,
    // bool:CWeapon_HasSecondaryAttack,
    // bool:CWeapon_HasCustomHud,

    T_Events:CWeapon_Events,
}

static ArrayMap(gCustomWeapons);

#define CWeapons_Call(%1,%2,[%3]) CompositeMacros ( \
    Events_SetReturnedValue(CWAPI_CONTINUE); \
    Events_CallPWhile( \
        %1[CWeapon_Events], \
        %2, \
        [%3], \
        (Events_IsRet(CWAPI_CONTINUE)) \
    ); \
)

CWeapons_Init() {
    InitArrayMap(gCustomWeapons, S_CustomWeapon, 8);
    CWeapons_Hooks_Init();
}

CWeapons_Count() {
    return ArrayMapSize(gCustomWeapons);
}

static T_CustomWeapon:CWeapons__Create(const WeaponData[S_CustomWeapon]) {
    // TODO: Is ref weapon name valid
    // if () {
    //     return Invalid_CustomWeapon;
    // }

    new Weapon[S_CustomWeapon];
    Weapon = WeaponData;

    Weapon[CWeapon_Events] = Events_Init(E_CWeapon_Event);
    CWeapons_Hooks_RegForWeapon(Weapon[CWeapon_Reference]);

    return T_CustomWeapon:ArrayMapPushArray(gCustomWeapons, Weapon, Weapon[CWeapon_Name]);
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

// bool:CWeapons_GetByName(const sWeaponName[], Weapon[S_CustomWeapon]) {
//     if (!CWeapons_IsValidName(sWeaponName)) {
//         return false;
//     }
    
//     ArrayMapGetArray(gCustomWeapons, sWeaponName, Weapon);
//     return true;
// }

bool:CWeapons_Get(const T_CustomWeapon:iWeapon, Weapon[S_CustomWeapon]) {
    if (!CWeapons_IsValidIndex(iWeapon)) {
        log_error(0, "Invalid custom weapon index (%d).", iWeapon);
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

    new iListener = -1;
    switch (iEvent) {
        case CWeapon_OnSpawn:
            // (const T_CustomWeapon:iWeapon, const ItemId)
            iListener = CreateOneForward(iPlugin, sCallback, FP_CELL, FP_CELL);
        case CWeapon_OnSetWeaponBoxModel:
            // (const T_CustomWeapon:iWeapon, const iWeaponBox, const ItemId)
            iListener = CreateOneForward(iPlugin, sCallback, FP_CELL, FP_CELL, FP_CELL);
    }

    if (iListener < 0) {
        log_error(0, "Can`t create forward for function '%s' in plugin #%d.", sCallback, iPlugin);
    }

    Events_PushListener(Weapon[CWeapon_Events], iEvent, iListener);
}

T_CustomWeapon:CWeapons_LoadFromCfg(const sFile[], const bool:bAbsolutePath = false) {
    new JSON:jWeapon = CfgUtils_GetJson(sFile, true, bAbsolutePath);

    if (jWeapon == Invalid_JSON) {
        log_error(0, "[ERROR] Invalid JSON syntax. File '%s'.", CfgUtils_GetPath(sFile));
        return Invalid_CustomWeapon;
    }

    if (!json_is_object(jWeapon)) {
        // TODO: Прикрутить оболочку жисона и слать ошибки через неё
        log_error(0, "[ERROR] Invalid config structure. File '%s'.", CfgUtils_GetPath(sFile));
        CfgUtils_JsonFree(jWeapon);
        return Invalid_CustomWeapon;
    }

    new WeaponData[S_CustomWeapon];

    CfgUtils_GetFileName(sFile, WeaponData[CWeapon_Name], charsmax(WeaponData[CWeapon_Name]));

    JsonObject_ReadStringOr(jWeapon, "Reference", WeaponData[CWeapon_Reference], charsmax(WeaponData[CWeapon_Reference])) {
        log_error(0, "[ERROR] Invalid config structure. File '%s'.", CfgUtils_GetPath(sFile));
        CfgUtils_JsonFree(jWeapon);
        return Invalid_CustomWeapon;
    }

    WeaponData[CWeapon_MaxClip] = JsonObject_ReadInt(jWeapon, "MaxClip", -1);
    WeaponData[CWeapon_MaxAmmo] = JsonObject_ReadInt(jWeapon, "MaxAmmo", -1);
    WeaponData[CWeapon_Weight] = JsonObject_ReadInt(jWeapon, "Weight", -1);
    WeaponData[CWeapon_DamageMult] = JsonObject_ReadFloat(jWeapon, "DamageMult", -1.0);

    new Models[S_CWeapon_Models];
    JsonObject_ReadAndPrecacheModel(
        jWeapon, "Models.View",
        Models[CWeapon_Model_View], charsmax(Models[CWeapon_Model_View]),
        .bDotNot = true
    );

    JsonObject_ReadAndPrecacheModel(
        jWeapon, "Models.World",
        Models[CWeapon_Model_World], charsmax(Models[CWeapon_Model_World]),
        .bDotNot = true
    );

    JsonObject_ReadAndPrecacheModel(
        jWeapon, "Models.Player",
        Models[CWeapon_Model_Player], charsmax(Models[CWeapon_Model_Player]),
        .bDotNot = true
    );

    WeaponData[CWeapon_Models] = Models;
    
    CfgUtils_JsonFree(jWeapon);

    return CWeapons__Create(WeaponData);
}

// CWeapons_LoadFromDir(const sPath[]) {
//     // ...
// }

#include "Cwapi/Core/CWeapons/Hooks"
#include "Cwapi/Core/CWeapons/Natives"
