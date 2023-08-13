#include <amxmodx>
#include "Cwapi/Natives"

CWeapons_Natives_Reg() {
    Natives_Reg("Weapons_Find");
    Natives_Reg("Weapons_FindByItem");
    Natives_Reg("Weapons_AddEventListener");
    Natives_Reg("Weapons_Give");
    Natives_Reg("Weapons_GetName");
}

T_CustomWeapon:@_Weapons_Find() {
    enum {Arg_sWeaponName = 1}

    new sWeaponName[CWAPI_WEAPON_NAME_MAX_LEN];
    get_string(Arg_sWeaponName, sWeaponName, charsmax(sWeaponName));

    return CWeapons_Find(sWeaponName);
}

T_CustomWeapon:@_Weapons_FindByItem() {
    enum {Arg_ItemId = 1}

    new ItemId = get_param(Arg_ItemId);

    return CWeapons_FindByItem(ItemId);
}

@_Weapons_AddEventListener(const iPlugin) {
    enum {Arg_iWeapon = 1, Arg_iEvent, Arg_sCallback}

    new T_CustomWeapon:iWeapon = T_CustomWeapon:get_param(Arg_iWeapon);
    new E_CWeapon_Event:iEvent = E_CWeapon_Event:get_param(Arg_iEvent);

    new sCallback[CWAPI_CALLBACK_MAX_LEN];
    get_string(Arg_sCallback, sCallback, charsmax(sCallback));

    CWeapons_AddEventListener(iWeapon, iEvent, iPlugin, sCallback);
}

@_Weapons_Give(const iPlugin) {
    enum {Arg_UserId = 1, Arg_iWeapon, Arg_iGiveType}

    new T_CustomWeapon:iWeapon = T_CustomWeapon:get_param(Arg_iWeapon);
    new UserId = get_param(Arg_UserId);
    new CWeapon_GiveType:iGiveType = CWeapon_GiveType:get_param(Arg_iGiveType);

    CWeapons_Give(UserId, iWeapon, iGiveType);
}

@_Weapons_GetName(const iPlugin) {
    enum {Arg_iWeapon = 1, Arg_sOut, Arg_iOutLen}

    new T_CustomWeapon:iWeapon = T_CustomWeapon:get_param(Arg_iWeapon);
    new iOutLen = get_param(Arg_iOutLen);
    
    new Weapon[S_CustomWeapon];
    CWeapons_Get(iWeapon, Weapon);

    return set_string(Arg_sOut, Weapon[CWeapon_Name], iOutLen);
}
