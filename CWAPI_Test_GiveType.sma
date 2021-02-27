#include <amxmodx>
#include <cwapi>

new const PLUG_NAME[] = "[CWAPI][Test] GiveType";
new const PLUG_VER[] = "1.0.0";

public CWAPI_LoadWeaponsPost(){
    register_plugin(PLUG_NAME, PLUG_VER, "ArKaNeMaN");

    if(!CWAPI_CheckVersionV1(0.7.0))
        set_fail_state("[ERROR] Required CWAPI v0.7.0");

    register_clcmd("CWAPI_Give_Smart", "@Cmd_GiveSmart");
    register_clcmd("CWAPI_Give_Append", "@Cmd_GiveAppend");
    register_clcmd("CWAPI_Give_Replace", "@Cmd_GiveReplace");
    register_clcmd("CWAPI_Give_Drop", "@Cmd_GiveDrop");
}

@Cmd_GiveSmart(const UserId){
    new WeaponName[32];
    read_argv(1, WeaponName, charsmax(WeaponName));

    CWAPI_GiveWeapon(UserId, WeaponName, CWAPI_GT_SMART);
}

@Cmd_GiveAppend(const UserId){
    new WeaponName[32];
    read_argv(1, WeaponName, charsmax(WeaponName));

    CWAPI_GiveWeapon(UserId, WeaponName, CWAPI_GT_APPEND);
}

@Cmd_GiveReplace(const UserId){
    new WeaponName[32];
    read_argv(1, WeaponName, charsmax(WeaponName));

    CWAPI_GiveWeapon(UserId, WeaponName, CWAPI_GT_REPLACE);
}

@Cmd_GiveDrop(const UserId){
    new WeaponName[32];
    read_argv(1, WeaponName, charsmax(WeaponName));

    CWAPI_GiveWeapon(UserId, WeaponName, CWAPI_GT_DROP);
}