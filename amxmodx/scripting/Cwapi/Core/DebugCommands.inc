#include <amxmodx>
#include <cwapi>

DbgCmds_Reg() {
    register_clcmd("Cwapi_Dbg_Give", "@DbgCmds_Give");
    register_clcmd("Cwapi_Dbg_List", "@DbgCmds_List");
}

@DbgCmds_List(const UserId) {
    log_amx("@DbgCmds_List(%n): Exec cmd.", UserId);
    if (!IS_DEBUG) {
        return PLUGIN_CONTINUE;
    }

    client_print(UserId, print_console, "Custom weapons:");

    new Weapon[S_CustomWeapon], T_CustomWeapon:iWeapon = Invalid_CustomWeapon;
    while ((iWeapon = CWeapons_Iterate(iWeapon, Weapon)) != Invalid_CustomWeapon) {
        client_print(UserId, print_console, "  #%d - %s (%s)", iWeapon, Weapon[CWeapon_Name], Weapon[CWeapon_Reference]);
    }

    client_print(UserId, print_console, "Total: %d weapons.", CWeapons_Count());

    return PLUGIN_HANDLED;
}

@DbgCmds_Give(const UserId) {
    enum {Arg_sWeaponName = 1, Arg_iGiveType}

    log_amx("@DbgCmds_Give(%n): Exec cmd.", UserId);
    if (!IS_DEBUG) {
        return PLUGIN_CONTINUE;
    }
    
    if (!is_user_alive(UserId)) {
        log_amx("@DbgCmds_Give(%n): Player is dead.", UserId);
        return PLUGIN_HANDLED;
    }

    new sWeaponName[CWAPI_WEAPON_NAME_MAX_LEN];
    read_argv(Arg_sWeaponName, sWeaponName, charsmax(sWeaponName));

    new T_CustomWeapon:iWeapon = CWeapons_Find(sWeaponName);

    new CWeapon_GiveType:iGiveType = CWAPI_GT_SMART;
    if (read_argc() > Arg_iGiveType) {
        iGiveType = CWeapon_GiveType:read_argv_int(Arg_iGiveType);
    }

    if (iWeapon != Invalid_CustomWeapon) {
        log_amx("@DbgCmds_Give(%n): Giving weapon '%s' (#%d).", UserId, sWeaponName, iWeapon);
        CWeapons_Give(UserId, iWeapon, iGiveType);
    } else {
        log_amx("@DbgCmds_Give(%n): Weapon '%s' not found.", UserId, sWeaponName);
    }

    return PLUGIN_HANDLED;
}
