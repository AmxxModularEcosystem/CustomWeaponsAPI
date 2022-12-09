#include <amxmodx>
#include <cwapi>

#include "Cwapi/Utils"
#include "Cwapi/ArrayTrieUtils"
#include "Cwapi/CfgUtils"
#include "Cwapi/Forwards"
#include "Cwapi/DebugMode"

#include "Cwapi/Core/CustomWeapons"

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
    
    Forwards_RegAndCall("Load", ET_IGNORE);

    CWeapons_LoadFromFolder("Weapons");

    // if (CWeapons_Count() < 1) {
    //     set_fail_state("[WARNING] No loaded weapons");
    //     return;
    // }

    server_print("[%s v%s] %d custom weapons loaded.", PluginName, PluginVersion, CWeapons_Count());
    Forwards_RegAndCall("Loaded", ET_IGNORE);

    register_clcmd("CWAPI_Give", "Cmd_GiveCustomWeapon");

    // CWAPI_Srv_Give <UserId> <WeaponName>
    // register_srvcmd("CWAPI_Srv_Give", "@SrvCmd_Give");
}

public Cmd_GiveCustomWeapon(const UserId) {
    enum {Arg_sWeaponName = 1, Arg_iGiveType}

    Dbg_PrintServer("Cmd_GiveCustomWeapon(%n): Exec cmd.", UserId);
    if (!IS_DEBUG) {
        return PLUGIN_CONTINUE;
    }
    
    if (!is_user_alive(UserId)) {
        Dbg_PrintServer("Cmd_GiveCustomWeapon(%n): Player is dead.", UserId);
        return PLUGIN_HANDLED;
    }

    new sWeaponName[CWAPI_WEAPON_NAME_MAX_LEN];
    read_argv(Arg_sWeaponName, sWeaponName, charsmax(sWeaponName));

    new T_CustomWeapon:iWeapon = CWeapons_Find(sWeaponName);

    new CWeapon_GiveType:iGiveType = CWAPI_GT_SMART;
    if (read_argc() > 2) {
        iGiveType = CWeapon_GiveType:read_argv_int(Arg_iGiveType);
    }

    if (iWeapon != Invalid_CustomWeapon) {
        Dbg_PrintServer("Cmd_GiveCustomWeapon(%n): Giving weapon '%s' (#%d).", UserId, sWeaponName, iWeapon);
        CWeapons_Give(UserId, iWeapon, iGiveType);
    } else {
        Dbg_PrintServer("Cmd_GiveCustomWeapon(%n): Weapon '%s' not found.", UserId, sWeaponName);
    }

    return PLUGIN_HANDLED;
}

// @SrvCmd_Give() {
//     enum {Arg_UserId = 1, Arg_WeaponName}
//     new UserId = read_argv_int(Arg_UserId);
//     new WeaponName[32];
//     read_argv(Arg_WeaponName, WeaponName, charsmax(WeaponName));

//     if (!is_user_alive(UserId)) {
//         log_amx("[ERROR] [CMD] User #%d not found or not alive.", UserId);
//         return PLUGIN_HANDLED;
//     }

//     if (!TrieKeyExists(WeaponsNames, WeaponName)) {
//         log_amx("[ERROR] [CMD] Weapon `%s` not found.", WeaponName);
//         return PLUGIN_HANDLED;
//     }

//     new WeaponId;
//     TrieGetCell(WeaponsNames, WeaponName, WeaponId);
//     GiveCustomWeapon(UserId, WeaponId, CWAPI_GT_SMART);

//     return PLUGIN_CONTINUE;
// }

#include "Cwapi/Core/Natives"
