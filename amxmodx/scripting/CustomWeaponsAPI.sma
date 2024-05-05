#include <amxmodx>
#include <ParamsController>
#include <cwapi>
#include "Cwapi/Utils"
#include "Cwapi/ArrayTrieUtils"
#include "Cwapi/CfgUtils"
#include "Cwapi/Forwards"
#include "Cwapi/DebugMode"
#include "Cwapi/Core/Params"

#include "Cwapi/Core/CustomWeapons"
#include "Cwapi/Core/DebugCommands"

#pragma semicolon 1

public stock const PluginName[] = "Custom Weapons API";
public stock const PluginVersion[] = _CWAPI_VERSION;
public stock const PluginAuthor[] = "ArKaNeMaN";
public stock const PluginURL[] = "https://github.com/ArKaNeMaN/amxx-CustomWeaponsAPI";
public stock const PluginDescription[] = "API for creating custom weapons";

public plugin_precache() {
    PluginInit();
}

PluginInit() {
    CallOnce();
    
    Dbg_PrintServer("%s run in debug mode.", PluginName);

    RegisterPluginByVars();
    register_library(CWAPI_LIBRARY);
    CreateConstCvar(CWAPI_VERSION_CVAR, CWAPI_VERSION);

    Forwards_Init("CWAPI");
    CfgUtils_SetFolder(CWAPI_CONFIGS_FOLDER);
    CWeapons_Init();
    
    // Тут регаются абилки (Хотя мб надо под них создать отдельный форвард...)
    Forwards_RegAndCall("Load", ET_IGNORE);

    CWeapons_LoadFromFolder("Weapons");
    server_print("[%s v%s] %d weapons loaded.", PluginName, PluginVersion, CWeapons_Count());

    Forwards_RegAndCall("Loaded", ET_IGNORE);

    DbgCmds_Reg();

    // CWAPI_Srv_Give <UserId> <WeaponName>
    register_srvcmd("Cwapi_Srv_Give", "@SrvCmd_Give");
    register_srvcmd("cwapi_weapons", "@SrvCmd_Weapons");
    register_srvcmd("cwapi_weapon_abilities", "@SrvCmd_WeaponAbilities");
}

@SrvCmd_Give() {
    enum {Arg_UserId = 1, Arg_sWeaponName, Arg_iGiveType}
    new UserId = read_argv_int(Arg_UserId);

    if (!is_user_alive(UserId)) {
        return PLUGIN_HANDLED;
    }
    
    new sWeaponName[32];
    read_argv(Arg_sWeaponName, sWeaponName, charsmax(sWeaponName));
    new T_CustomWeapon:iWeapon = CWeapons_Find(sWeaponName);

    new CWeapon_GiveType:iGiveType = CWAPI_GT_SMART;
    if (read_argc() > Arg_iGiveType) {
        iGiveType = CWeapon_GiveType:read_argv_int(Arg_iGiveType);
    }

    if (iWeapon != Invalid_CustomWeapon) {
        CWeapons_Give(UserId, iWeapon, iGiveType);
    } else {
        log_amx("Cwapi_Srv_Give: Weapon '%s' not found.", UserId, sWeaponName);
    }

    return PLUGIN_HANDLED;
}

@SrvCmd_Weapons() {
    new T_CustomWeapon:iWeapon = Invalid_CustomWeapon;
    new Weapon[S_CustomWeapon];

    server_print("╔═════╤══════════════════════════════════╤══════════════════════════════════╤═════════════════╗");
    server_print("║ ID  │ Weapon name                      │ Reference                        │ Abilities count ║");
    server_print("╟─────┼──────────────────────────────────┼──────────────────────────────────┼─────────────────╢");
    while ((iWeapon = CWeapons_Iterate(iWeapon, Weapon)) != Invalid_CustomWeapon) {
        server_print("║ %-3d │ %-32s │ %-32s │ %-15d ║", iWeapon, Weapon[CWeapon_Name], Weapon[CWeapon_Reference], ArraySizeSafe(Weapon[CWeapon_Abilities]));
    }
    server_print("╟─────┴──────────────────────────────────┴──────────────────────────────────┴─────────────────╢");
    server_print("║ Total: %-4d                                                                                 ║", CWeapons_Count());
    server_print("╚═════════════════════════════════════════════════════════════════════════════════════════════╝");
}

@SrvCmd_WeaponAbilities() {
    enum {Arg_Weapon = 1}

    new sWeapon[CWAPI_WEAPON_NAME_MAX_LEN];
    read_argv(Arg_Weapon, sWeapon, charsmax(sWeapon));

    new T_CustomWeapon:iWeapon, Weapon[S_CustomWeapon];
    if (!CWeaponUtils_GetByNameOrId(sWeapon, Weapon, iWeapon)) {
        server_print("Weapon '%s' not found.", sWeapon);
        return PLUGIN_HANDLED;
    }

    server_print("╔═════╤════════════════════════════════════════╗");
    server_print("║  #  │ Ability                                ║");
    server_print("╟─────┼────────────────────────────────────────╢");
    ArrayForeachCell (Weapon[CWeapon_Abilities]: i => iAbilityUnit) {
        new WeaponAbilityUnit[S_WAbility_Unit];
        WAbilityUnit_Get(iAbilityUnit, WeaponAbilityUnit);

        new WeaponAbility[S_WeaponAbility];
        WAbility_Get(WeaponAbilityUnit[WAbilityUnit_Ability], WeaponAbility);
        
        server_print("║ %03d │ %-32s (#%-2d) ║", i + 1, WeaponAbility[WAbility_Name], WeaponAbilityUnit[WAbilityUnit_Ability]);
    }
    server_print("╟─────┴────────────────────────────────────────╢");
    server_print("║ Total: %-4d                                  ║", ArraySizeSafe(Weapon[CWeapon_Abilities]));
    server_print("╚══════════════════════════════════════════════╝");
    
    return PLUGIN_HANDLED;
}

#include "Cwapi/Core/Natives"
