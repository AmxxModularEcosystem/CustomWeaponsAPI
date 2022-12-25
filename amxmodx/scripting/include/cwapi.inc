#if defined _cwapi_included
	#endinput
#endif
#define _cwapi_included

#include <amxmodx>

#define _CWAPI_VERSION "0.8.0-pre-alpha"
stock const CWAPI_VERSION[] = _CWAPI_VERSION;
stock const CWAPI_LIBRARY[] = "CustomWeaponsAPI";
stock const CWAPI_VERSION_CVAR[] = "CWAPI_VERSION";
stock const CWAPI_CONFIGS_FOLDER[] = "CustomWeaponsAPI";

#define CWAPI_WEAPON_NAME_MAX_LEN 32
#define CWAPI_CALLBACK_MAX_LEN 32
#define CWAPI_IMPULSE_OFFSET 4354
#define var_CWAPI_ItemOwner var_iuser4

enum T_CustomWeapon { Invalid_CustomWeapon = -1 }

enum E_CWeapon_Event {
    CWeapon_OnSpawn,
    CWeapon_OnSetWeaponBoxModel,
    CWeapon_OnAddPlayerItem,
    CWeapon_OnDeploy,
}

/**
 * Публиные параметры кастомного оружия
 */
enum E_CWeapon_Data {
    CWeaponData_Reference,
    CWeaponData_Name,
}

enum {
    // Продолжить вызов обработчиков и обработать событие
    CWAPI_CONTINUE = 0,

    // Прекратить вызов обработчиков и прервать действия CWAPI, если они есть
    CWAPI_STOP,

    // Прекратить вызов обработчиков, попытаться прервать действия по-умолчанию и прервать действия CWAPI, если они есть
    CWAPI_STOP_MAIN,
};

enum CWeapon_GiveType {
    CWAPI_GT_SMART = -1, // Для ножей GT_REPLACE, для гранат GT_APPEND, для остального GT_DROP_AND_REPLACE
    CWAPI_GT_APPEND, // Аналог GT_APPEND
    CWAPI_GT_REPLACE, // Аналог GT_REPLACE
    CWAPI_GT_DROP, // Аналог GT_DROP_AND_REPLACE
}

native CWAPI_Init();

forward CWAPI_OnLoad();
forward CWAPI_OnLoaded();


native CWAPI_Weapons_Find(const sWeaponName[]);
native CWAPI_Weapons_FindByItem(const ItemId);
native CWAPI_Weapons_AddEventListener(const T_CustomWeapon:iWeapon, const E_CWeapon_Event:iEvent, const iPlugin, const sCallback[]);
native CWAPI_Weapons_Give(const UserId, const T_CustomWeapon:iWeapon, const CWeapon_GiveType:iGiveType = CWAPI_GT_SMART);
