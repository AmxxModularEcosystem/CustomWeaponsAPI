#include <amxmodx>
#include <ParamsController>
#include "Cwapi/Natives"

#define API_WAbility_CheckInitP(%1) CompositeMacros( \
    if (!WAbility_IsInited()) { \
        log_error(1, "Attempt interact with custom weapon abilities before init them."); \
        return %1; \
    } \
)
#define API_WAbility_CheckInit() API_WAbility_CheckInitP( )

WAbilities_Natives_Reg() {
    Natives_Reg("Abilities_Register");
    Natives_Reg("Abilities_AddParams");
    Natives_Reg("Abilities_AddEventListener");
}

T_WeaponAbility:@_Abilities_Register() {
    enum {Arg_sAbilityName = 1}

    API_WAbility_CheckInitP(Invalid_WeaponAbility);

    new sAbilityName[CWAPI_ABILITY_NAME_MAX_LEN];
    get_string(Arg_sAbilityName, sAbilityName, charsmax(sAbilityName));

    return WAbility_Create(sAbilityName);
}

@_Abilities_AddParams(const PluginId, const iParamsCount) {
    enum {Arg_iAbility = 1, Arg_Params}

    API_WAbility_CheckInit();

    new T_WeaponAbility:iAbility = T_WeaponAbility:get_param(Arg_iAbility);

    new Ability[S_WeaponAbility];
    WAbility_Get(iAbility, Ability);

    Ability[WAbility_Params] = ParamsController_Param_ListFromNativeParams(Arg_Params, iParamsCount, Ability[WAbility_Params]);

    WAbility_Update(Ability);
}

@_Abilities_AddEventListener(const PluginId) {
    enum {Arg_iAbility = 1, Arg_iEvent, Arg_sCallback}

    API_WAbility_CheckInit();

    new T_WeaponAbility:iAbility = T_WeaponAbility:get_param(Arg_iAbility);
    new E_CWeapon_Event:iEvent = E_CWeapon_Event:get_param(Arg_iEvent);

    new sCallback[CWAPI_CALLBACK_MAX_LEN];
    get_string(Arg_sCallback, sCallback, charsmax(sCallback));

    WAbility_AddEventListener(iAbility, iEvent, PluginId, sCallback);
}
