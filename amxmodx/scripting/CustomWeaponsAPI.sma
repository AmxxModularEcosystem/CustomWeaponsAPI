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
    
    Forwards_RegAndCall("Load", ET_IGNORE);

    CWeapons_LoadFromFolder("Weapons");
    server_print("[%s v%s] %d weapons loaded.", PluginName, PluginVersion, CWeapons_Count());

    Forwards_RegAndCall("Loaded", ET_IGNORE);

    DbgCmds_Reg();

    // CWAPI_Srv_Give <UserId> <WeaponName>
    register_srvcmd("Cwapi_Srv_Give", "@SrvCmd_Give");
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

#include "Cwapi/Core/Natives"
