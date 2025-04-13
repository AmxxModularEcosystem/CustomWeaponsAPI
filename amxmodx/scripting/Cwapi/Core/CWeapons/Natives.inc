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
    Natives_Reg("Weapons_EmitDamage");
    register_native("CWAPI_Weapons_GetAbilityParams", "@CWAPI_Weapons_GetAbilityParams");
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
    enum {Arg_UserId = 1, Arg_iWeapon, Arg_iGiveType, Arg_bForceActive, Arg_InitBpAmmo}
    
    API_CWeapons_CheckInitP(0);

    new T_CustomWeapon:iWeapon = T_CustomWeapon:get_param(Arg_iWeapon);
    new UserId = get_param(Arg_UserId);
    new CWeapon_GiveType:iGiveType = CWeapon_GiveType:get_param(Arg_iGiveType);
    new bool:bForceActive = bool:get_param(Arg_bForceActive);
    new initBpAmmo = get_param(Arg_InitBpAmmo);

    return CWeapons_Give(UserId, iWeapon, iGiveType, bForceActive, initBpAmmo);
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

any:@_Weapons_GetAttribute(const iPlugin) {
    enum {Arg_iWeapon = 1, Arg_iAttribute, Arg_sOut, Arg_iOutLen}
    
    API_CWeapons_CheckInitP(0);

    new T_CustomWeapon:iWeapon = T_CustomWeapon:get_param(Arg_iWeapon);
    new E_CWeapon_Attribute:iAttribute = E_CWeapon_Attribute:get_param(Arg_iAttribute);

    switch (iAttribute) {
        case CWeaponAttr_DamageMult:
            return CWeapons_GetProp(iWeapon, CWeapon_DamageMult);
        case CWeaponAttr_ReferenceId:
            return CWeapons_GetProp(iWeapon, CWeapon_ReferenceId);
        case CWeaponAttr_Weight:
            return CWeapons_GetProp(iWeapon, CWeapon_Weight);
        case CWeaponAttr_MaxClip:
            return CWeapons_GetProp(iWeapon, CWeapon_MaxClip);
        case CWeaponAttr_MaxAmmo:
            return CWeapons_GetProp(iWeapon, CWeapon_MaxAmmo);
        case CWeaponAttr_DeployTime:
            return CWeapons_GetProp(iWeapon, CWeapon_DeployTime);
        case CWeaponAttr_ReloadTime:
            return CWeapons_GetProp(iWeapon, CWeapon_ReloadTime);
        case CWeaponAttr_MaxWalkSpeed:
            return CWeapons_GetProp(iWeapon, CWeapon_MaxWalkSpeed);
    }
    
    new Weapon[S_CustomWeapon];
    if (!CWeapons_Get(iWeapon, Weapon)) {
        log_error(1, "Invalid weapon handler: %d", iWeapon);
        return 0;
    }
    
    switch (iAttribute) {
        case CWeaponAttr_Reference:
            return set_string(Arg_sOut, Weapon[CWeapon_Reference], get_param(Arg_iOutLen));

        case CWeaponAttr_Name:
            return set_string(Arg_sOut, Weapon[CWeapon_Name], get_param(Arg_iOutLen));
    }
    
    log_error(1, "Invalid custom weapon attribute type: %d", iAttribute);
    return 0;
}

Float:@_Weapons_EmitDamage() {
    enum {Arg_WeaponIndex = 1, Arg_VictimIndex, Arg_InflictorIndex, Arg_AttackerIndex, Arg_Damage, Arg_DamageBits, Arg_ItemIndex, Arg_IgnoreDamageMult}
    
    return CWeapons_EmitDamage(
        T_CustomWeapon:get_param(Arg_WeaponIndex),
        get_param(Arg_VictimIndex),
        get_param(Arg_InflictorIndex),
        get_param(Arg_AttackerIndex),
        Float:get_param(Arg_Damage),
        get_param(Arg_DamageBits),
        get_param(Arg_ItemIndex),
        bool:get_param(Arg_IgnoreDamageMult)
    );
}

Trie:@CWAPI_Weapons_GetAbilityParams() {
    enum {Arg_WeaponIndex = 1, Arg_Ability}
    
    return CWeapons_GetAbilityParams(
        T_CustomWeapon:get_param(Arg_WeaponIndex),
        T_WeaponAbility:get_param(Arg_Ability)
    );
}
