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

    if (IS_DEBUG) {
        // CWAPI_Give <WeaponName>
        register_clcmd("CWAPI_Give", "Cmd_GiveCustomWeapon");
    }

    // CWAPI_Srv_Give <UserId> <WeaponName>
    register_srvcmd("CWAPI_Srv_Give", "@SrvCmd_Give");
}

// #if DEBUG
//     public Cmd_GiveCustomWeapon(const Id) {
//         new WeaponName[32];
//         read_argv(1, WeaponName, charsmax(WeaponName));

//         if (TrieKeyExists(WeaponsNames, WeaponName)) {
//             new WeaponId;
//             TrieGetCell(WeaponsNames, WeaponName, WeaponId);

//             if (GiveCustomWeapon(Id, WeaponId, CWAPI_GT_SMART) != -1) {
//                 client_print_color(Id, print_team_default, "%L", LANG_PLAYER, "WEAPON_GIVE_SUCCESS", WeaponName);
//             } else {
//                 client_print_color(Id, print_team_default, "%L", LANG_PLAYER, "WEAPON_GIVE_ERROR");
//             }

//             return PLUGIN_HANDLED;
//         }

//         client_print_color(Id, print_team_default, "%L", LANG_PLAYER, "WEAPON_NOT_FOUND", WeaponName);
//         return PLUGIN_CONTINUE;
//     }
// #endif

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

// public Cmd_Select(const UserId) {
//     if (!is_user_alive(UserId)) {
//         return PLUGIN_HANDLED;
//     }

//     new WeaponName[40];
//     read_argv(0, WeaponName, charsmax(WeaponName));

//     if (TrieKeyExists(WeaponsNames, WeaponName[7])) {
//         new WeaponId, Data[CWAPI_WeaponData];
//         TrieGetCell(WeaponsNames, WeaponName[7], WeaponId);
//         ArrayGetArray(CustomWeapons, WeaponId, Data);

//         engclient_cmd(UserId, GetWeapFullName(Data[CWAPI_WD_DefaultName]));
//         return PLUGIN_HANDLED_MAIN;
//     }

//     return PLUGIN_CONTINUE;
// }

// Выдача пушки
stock GiveCustomWeapon(
    const UserId,
    const T_CustomWeapon:iWeapon,
    const CWAPI_GiveType:iGiveType = CWAPI_GT_SMART
) {
    if (!CallWeaponEvent(WeaponId, CWAPI_WE_Take, WeaponId, Id)) {
        return -1;
    }

    new Weapon[S_CustomWeapon];
    CWeapons_Get(iWeapon, Weapon);

    new GiveType:WeaponGiveType;
    if (iGiveType == CWAPI_GT_SMART) {
        if (equal(Data[CWAPI_WD_DefaultName], "knife")) {
            WeaponGiveType = GT_REPLACE;
        } else if (IsGrenade(Data[CWAPI_WD_DefaultName])) {
            WeaponGiveType = GT_APPEND;
        } else {
            WeaponGiveType = GT_DROP_AND_REPLACE;
        }
    } else {
        WeaponGiveType = GiveType:iGiveType;
    }

    new ItemId = rg_give_custom_item(
        UserId,
        Weapon[CWeapon_Reference],
        WeaponGiveType,
        WeaponId + CWAPI_IMPULSE_OFFSET
    );

    if (is_nullent(ItemId)) {
        return -1;
    }

    new WeaponIdType:DefaultWeaponId = WeaponIdType:rg_get_iteminfo(ItemId, ItemInfo_iId);
    
    rg_set_iteminfo(ItemId, ItemInfo_pszName, GetWeapFullName(Data[CWAPI_WD_Name]));

    if (Data[CWAPI_WD_HasSecondaryAttack]) {
        set_member(ItemId, m_Weapon_bHasSecondaryAttack, Data[CWAPI_WD_HasSecondaryAttack]);
    }

    if (Data[CWAPI_WD_Weight]) {
        rg_set_iteminfo(ItemId, ItemInfo_iWeight, Data[CWAPI_WD_Weight]);
    }
    
    if (DefaultWeaponId != WEAPON_KNIFE) {
        if (Data[CWAPI_WD_ClipSize]) {
            rg_set_iteminfo(ItemId, ItemInfo_iMaxClip, Data[CWAPI_WD_ClipSize]);
            rg_set_user_ammo(Id, DefaultWeaponId, Data[CWAPI_WD_ClipSize]);
        }

        if (Data[CWAPI_WD_MaxAmmo] >= 0) {
            rg_set_iteminfo(ItemId, ItemInfo_iMaxAmmo1, Data[CWAPI_WD_MaxAmmo]);
            rg_set_user_bpammo(Id, DefaultWeaponId, Data[CWAPI_WD_MaxAmmo]);
        } else {
            rg_set_user_bpammo(Id, DefaultWeaponId, rg_get_iteminfo(ItemId, ItemInfo_iMaxAmmo1));
        }
    }

    // if (Data[CWAPI_WD_Damage] >= 0.0) {
    //     set_member(
    //         ItemId, m_Weapon_flBaseDamage,
    //         Data[CWAPI_WD_Damage]
    //     );
    // }

    if (Data[CWAPI_WD_DamageMult] >= 0.0) {
        mult_member_f(ItemId, m_Weapon_flBaseDamage, Data[CWAPI_WD_DamageMult]);
        set_member(
            ItemId, m_Weapon_flBaseDamage,
            Float:get_member(ItemId, m_Weapon_flBaseDamage)*Data[CWAPI_WD_DamageMult]
        );

        if (DefaultWeaponId == WEAPON_KNIFE) {
            set_member(
                ItemId, m_Knife_flStabBaseDamage,
                Float:get_member(ItemId, m_Knife_flStabBaseDamage)*Data[CWAPI_WD_DamageMult]
            );

            set_member(
                ItemId, m_Knife_flSwingBaseDamage,
                Float:get_member(ItemId, m_Knife_flSwingBaseDamage)*Data[CWAPI_WD_DamageMult]
            );

            set_member(
                ItemId, m_Knife_flSwingBaseDamage_Fast,
                Float:get_member(ItemId, m_Knife_flSwingBaseDamage_Fast)*Data[CWAPI_WD_DamageMult]
            );
        } else if (DefaultWeaponId == WEAPON_M4A1) {
            set_member(
                ItemId, m_M4A1_flBaseDamageSil,
                Float:get_member(ItemId, m_M4A1_flBaseDamageSil)*Data[CWAPI_WD_DamageMult]
            );
        } else if (DefaultWeaponId == WEAPON_USP) {
            set_member(
                ItemId, m_USP_flBaseDamageSil,
                Float:get_member(ItemId, m_USP_flBaseDamageSil)*Data[CWAPI_WD_DamageMult]
            );
        } else if (DefaultWeaponId == WEAPON_FAMAS) {
            set_member(
                ItemId, m_Famas_flBaseDamageBurst,
                Float:get_member(ItemId, m_Famas_flBaseDamageBurst)*Data[CWAPI_WD_DamageMult]
            );
        }
    }


    return ItemId;
}

#include "Cwapi/Core/Natives"
