#if defined __cwapi_CustomWeapons_Abilities_included
    #endinput
#endif
#define __cwapi_CustomWeapons_Abilities_included

#include <amxmodx>
#include <json>
#include <ParamsController>
#include "Cwapi/Events"
#include "Cwapi/ArrayMap"
#include "Cwapi/Core/CWeapons/Utils"

#define WAbility_Call(%1,%2,[%3]) \
    Events_CallPWhile(%1[WAbility_Events], %2, [%3], Events_IsRet(CWAPI_CONTINUE))

#define WAbilityUnit_Call(%1,%2,[%3]) CompositeMacros( \
    new __WAbilityUnit_Call_Ability[S_WeaponAbility]; \
    WAbility_Get(%1[WAbilityUnit_Ability], __WAbilityUnit_Call_Ability); \
    WAbility_Call(__WAbilityUnit_Call_Ability, %2, [%3, %1[WAbilityUnit_Params]]); \
)

// TODO: Как-то всё переусложнено немного))

enum _:S_WeaponAbility {
    WAbility_Name[CWAPI_ABILITY_NAME_MAX_LEN],
    T_Events:WAbility_Events,
    Array:WAbility_Params,
}

enum _:S_WAbility_Unit {
    T_WeaponAbility:WAbilityUnit_Ability,
    Trie:WAbilityUnit_Params,
}

enum T_WAbility_Unit { Invalid_WAbility_Unit = -1}

static ArrayMap(gAbilities); // S_WeaponAbility[]
static Array:gAbilityUnits; // S_WAbility_Unit[]

WAbility_Init() {
    InitArrayMap(gAbilities, S_WeaponAbility, 1);
    gAbilityUnits = ArrayCreate(S_WAbility_Unit, 1);

    ParamsController_Init();
}

T_WeaponAbility:WAbility_Create(const sAbilityName[]) {
    new Ability[S_WeaponAbility];

    copy(Ability[WAbility_Name], charsmax(Ability[WAbility_Name]), sAbilityName);
    Ability[WAbility_Events] = Events_Init(E_CWeapon_Event);
    Ability[WAbility_Params] = Invalid_Array;

    return T_WeaponAbility:ArrayMapPushArray(gAbilities, Ability, Ability[WAbility_Name]);
}

bool:WAbility_IsExists(const sAbilityName[]) {
    return ArrayMapHasKey(gAbilities, sAbilityName);
}

T_WeaponAbility:WAbility_Find(const sAbilityName[]) {
    if (!WAbility_IsExists(sAbilityName)) {
        return Invalid_WeaponAbility;
    }
    
    return T_WeaponAbility:ArrayMapGetIndex(gAbilities, sAbilityName);
}

WAbility_Get(const T_WeaponAbility:iAbility, Ability[]) {
    ArrayMapGetiArray(gAbilities, iAbility, Ability);
}

WAbility_Update(const Ability[]) {
    ArrayMapSetArray(gAbilities, Ability[WAbility_Name], Ability);
}

WAbility_AddEventListener(
    const T_WeaponAbility:iAbility,
    const E_CWeapon_Event:iEvent,
    const iPlugin,
    const sCallback[]
) {
    new Ability[S_WeaponAbility];
    WAbility_Get(iAbility, Ability);

    Events_PushListener(Ability[WAbility_Events], iEvent, CWeaponUtils_MakeEventCallback(iEvent, iPlugin, sCallback));
}

WAbilityUnit_Get(const T_WAbility_Unit:iAbilityUnit, AbilityUnit[]) {
    ArrayGetArray(gAbilityUnits, _:iAbilityUnit, AbilityUnit);
}

Array:WAbilityUnit_ReadJsonObject(const JSON:jObj, &Array:aAbilities = Invalid_Array) {
    if (aAbilities == Invalid_Array) {
        aAbilities = ArrayCreate(1, 1);
    }

    for (new i = 0, ii = json_object_get_count(jObj); i < ii; ++i) {
        new sAbilityName[CWAPI_ABILITY_NAME_MAX_LEN];
        json_object_get_name(jObj, i, sAbilityName, charsmax(sAbilityName));

        new T_WeaponAbility:iAbility = WAbility_Find(sAbilityName);
        if (iAbility == Invalid_WeaponAbility) {
            log_amx("[WARNING] Weapon ability '%s' not found", sAbilityName);
            continue;
        }
        // TODO: Переделать на JsonUtils от випки, когда они будут готовы

        new JSON:jAbility = json_object_get_value(jObj, sAbilityName);
        new T_WAbility_Unit:iAbilityUnit = WAbilityUnit_LoadFromJsonObject(jAbility, iAbility);
        json_free(jAbility);

        if (iAbilityUnit != Invalid_WAbility_Unit) {
            ArrayPushCell(aAbilities, iAbilityUnit);
        }
    }

    return aAbilities;
}

T_WAbility_Unit:WAbilityUnit_LoadFromJsonObject(const JSON:jAbilityUnit, const T_WeaponAbility:iAbility) {
    new Ability[S_WeaponAbility];
    WAbility_Get(iAbility, Ability);

    new AbilityUnit[S_WAbility_Unit];
    AbilityUnit[WAbilityUnit_Ability] = iAbility;

    AbilityUnit[WAbilityUnit_Params] = Invalid_Trie;
    new E_ParamsReadErrorType:iErrType, sErrParamName[PARAM_KEY_MAX_LEN];
    AbilityUnit[WAbilityUnit_Params] = ParamsController_Param_ReadList(
        Ability[WAbility_Params], jAbilityUnit, AbilityUnit[WAbilityUnit_Params],
        iErrType, sErrParamName, charsmax(sErrParamName)
    );

    if (iErrType != ParamsReadError_None) {
        // TODO: Написать обёртку для вывода ошибки параметров c учётом её типа
        log_amx("[WARNING] Ability param '%s' not presented or invalid.", sErrParamName);
        return Invalid_WAbility_Unit;
    }

    return T_WAbility_Unit:ArrayPushArray(gAbilityUnits, AbilityUnit);
}

#include "Cwapi/Core/CWeapons/Abilities/Natives"
