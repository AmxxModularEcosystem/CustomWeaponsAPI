#include <amxmodx>
#include "Cwapi/Natives"
#include "Cwapi/Utils"

#include "Cwapi/Core/CustomWeapons"

#define API_CWeapons_CheckInitP(%1) CompositeMacros( \
    if (!CWeapons_IsInited()) { \
        log_error(1, "Attempt interact with custom weapons before init them."); \
        return %1; \
    } \
)
#define API_CWeapons_CheckInit() API_CWeapons_CheckInitP( )

CWeapons_Natives_Reg() {
    Natives_Reg("Weapons_Find");
    Natives_Reg("Weapons_FindByItem");
    Natives_Reg("Weapons_AddEventListener");
    Natives_Reg("Weapons_Give");
    Natives_Reg("Weapons_GetName");
    // Natives_Reg("Weapons_GetAbilityParams");
    Natives_Reg("Weapons_GetAttribute");
}

T_CustomWeapon:@_Weapons_Find() {
    enum {Arg_sWeaponName = 1}
    
    API_CWeapons_CheckInitP(Invalid_CustomWeapon);

    new sWeaponName[CWAPI_WEAPON_NAME_MAX_LEN];
    get_string(Arg_sWeaponName, sWeaponName, charsmax(sWeaponName));

    return CWeapons_Find(sWeaponName);
}

T_CustomWeapon:@_Weapons_FindByItem() {
    enum {Arg_ItemId = 1}
    
    API_CWeapons_CheckInitP(Invalid_CustomWeapon);

    new ItemId = get_param(Arg_ItemId);

    return CWeapons_FindByItem(ItemId);
}

@_Weapons_AddEventListener(const iPlugin) {
    enum {Arg_iWeapon = 1, Arg_iEvent, Arg_sCallback}
    
    API_CWeapons_CheckInit();

    new T_CustomWeapon:iWeapon = T_CustomWeapon:get_param(Arg_iWeapon);
    new E_CWeapon_Event:iEvent = E_CWeapon_Event:get_param(Arg_iEvent);

    new sCallback[CWAPI_CALLBACK_MAX_LEN];
    get_string(Arg_sCallback, sCallback, charsmax(sCallback));

    CWeapons_AddEventListener(iWeapon, iEvent, iPlugin, sCallback);
}

@_Weapons_Give(const iPlugin) {
    enum {Arg_UserId = 1, Arg_iWeapon, Arg_iGiveType}
    
    API_CWeapons_CheckInitP(0);

    new T_CustomWeapon:iWeapon = T_CustomWeapon:get_param(Arg_iWeapon);
    new UserId = get_param(Arg_UserId);
    new CWeapon_GiveType:iGiveType = CWeapon_GiveType:get_param(Arg_iGiveType);

    return CWeapons_Give(UserId, iWeapon, iGiveType);
}

@_Weapons_GetName(const iPlugin) {
    enum {Arg_iWeapon = 1, Arg_sOut, Arg_iOutLen}
    
    API_CWeapons_CheckInitP(0);

    new T_CustomWeapon:iWeapon = T_CustomWeapon:get_param(Arg_iWeapon);
    new iOutLen = get_param(Arg_iOutLen);
    
    new Weapon[S_CustomWeapon];
    CWeapons_Get(iWeapon, Weapon);

    return set_string(Arg_sOut, Weapon[CWeapon_Name], iOutLen);
}

// TODO: Получение способности оружия по её хендлеру или названию
// @_Weapons_GetAbilityParams(const iPlugin) {
//     enum {Arg_iWeapon = 1, Arg_iAbility}
    
//     API_CWeapons_CheckInitP(Invalid_Trie);

//     new T_CustomWeapon:iWeapon = T_CustomWeapon:get_param(Arg_iWeapon);
//     new T_WeaponAbility:iAbility = T_WeaponAbility:get_param(Arg_iAbility);
    
//     new Weapon[S_CustomWeapon];
//     CWeapons_Get(iWeapon, Weapon);

//     return set_string(Arg_sOut, Weapon[CWeapon_Name], iOutLen);
// }

@_Weapons_GetAttribute(const iPlugin) {
    enum {Arg_iWeapon = 1, Arg_iAttribute, Arg_sOut, Arg_iOutLen}
    
    API_CWeapons_CheckInitP(0);

    new T_CustomWeapon:iWeapon = T_CustomWeapon:get_param(Arg_iWeapon);
    new E_CWeapon_Attribute:iAttribute = E_CWeapon_Attribute:get_param(Arg_iAttribute);
    
    new Weapon[S_CustomWeapon];
    CWeapons_Get(iWeapon, Weapon);
    
    switch (iAttribute) {
        case CWeaponAttr_Reference:
            return set_string(Arg_sOut, Weapon[CWeapon_Reference], get_param(Arg_iOutLen));

        case CWeaponAttr_Name:
            return set_string(Arg_sOut, Weapon[CWeapon_Name], get_param(Arg_iOutLen));
    }
    
    log_error(1, "Invalid custom weapon attribute type: %d", iAttribute);
    return 0;
}
