#include <amxmodx>
#include <ParamsController>
#include "Cwapi/Natives"

WAbilities_Natives_Reg() {
    Natives_Reg("Abilities_Register");
    Natives_Reg("Abilities_AddParams");
    Natives_Reg("Abilities_AddEventListener");
}

T_WeaponAbility:@_Abilities_Register() {
    enum {Arg_sAbilityName = 1}

    new sAbilityName[CWAPI_ABILITY_NAME_MAX_LEN];
    get_string(Arg_sAbilityName, sAbilityName, charsmax(sAbilityName));

    return WAbility_Create(sAbilityName);
}

@_Abilities_AddParams(const PluginId, const iParamsCount) {
    enum {Arg_iAbility = 1, Arg_Params}

    new T_WeaponAbility:iAbility = T_WeaponAbility:get_param(Arg_iAbility);

    new Ability[S_WeaponAbility];
    WAbility_Get(iAbility, Ability);

    Ability[WAbility_Params] = ParamsController_Param_ListFromNativeParams(Arg_Params, iParamsCount, Ability[WAbility_Params]);

    WAbility_Update(Ability);
}

@_Abilities_AddEventListener(const PluginId) {
    enum {Arg_iAbility = 1, Arg_iEvent, Arg_sCallback}

    new T_WeaponAbility:iAbility = T_WeaponAbility:get_param(Arg_iAbility);
    new E_CWeapon_Event:iEvent = E_CWeapon_Event:get_param(Arg_iEvent);

    new sCallback[CWAPI_CALLBACK_MAX_LEN];
    get_string(Arg_sCallback, sCallback, charsmax(sCallback));

    WAbility_AddEventListener(iAbility, iEvent, PluginId, sCallback);
}
