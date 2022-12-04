#include <amxmodx>
#include <reapi>
#include <hamsandwich>
#include <json>
#include <regex>
#include <cwapi>

#include "Cwapi/ArrayTrieUtils"
#include "Cwapi/Utils"
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
    CWeapons_Init();
    
    Forwards_RegAndCall("Load", ET_IGNORE);

    // LoadWeapons();

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
GiveCustomWeapon(const Id, const WeaponId, const CWAPI_GiveType:Type = CWAPI_GT_SMART) {
    if (!is_user_alive(Id)) {
        return -1;
    }

    if (!IsCustomWeapon(WeaponId)) {
        return -1;
    }

    if (!CallWeaponEvent(WeaponId, CWAPI_WE_Take, WeaponId, Id)) {
        return -1;
    }

    new Data[CWAPI_WeaponData]; ArrayGetArray(CustomWeapons, WeaponId, Data);

    new GiveType:WeaponGiveType;
    if (Type == CWAPI_GT_SMART) {
        if (equal(Data[CWAPI_WD_DefaultName], "knife")) {
            WeaponGiveType = GT_REPLACE;
        } else if (IsGrenade(Data[CWAPI_WD_DefaultName])) {
            WeaponGiveType = GT_APPEND;
        } else {
            WeaponGiveType = GT_DROP_AND_REPLACE;
        }
    } else {
        WeaponGiveType = GiveType:Type;
    }

    new ItemId = rg_give_custom_item(
        Id,
        GetWeapFullName(Data[CWAPI_WD_DefaultName]),
        WeaponGiveType,
        WeaponId+CWAPI_IMPULSE_OFFSET
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

    if (Data[CWAPI_WD_Damage] >= 0.0) {
        set_member(
            ItemId, m_Weapon_flBaseDamage,
            Data[CWAPI_WD_Damage]
        );
    }

    if (Data[CWAPI_WD_DamageMult] >= 0.0) {
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

    set_entvar(ItemId, var_CWAPI_ItemOwner, Id);

    return ItemId;
}

// CONFIGS

LoadWeapons() {
    CustomWeapons = ArrayCreate(CWAPI_WeaponData, 8);
    WeaponsNames = TrieCreate();
    WeaponAbilities = TrieCreate();
    new Path[PLATFORM_MAX_PATH];
    get_localinfo("amxx_configsdir", Path, charsmax(Path));
    add(Path, charsmax(Path), "/plugins/CustomWeaponsAPI/Weapons/");
    if (!dir_exists(Path)) {
        set_fail_state("[ERROR] Weapons folder '%s' not found.", Path);
        return;
    }
    
    new File[PLATFORM_MAX_PATH], DirHandler, FileType:Type;
    DirHandler = open_dir(Path, File, charsmax(File), Type);
    if (!DirHandler) {
        set_fail_state("[ERROR] Can't open weapons folder '%s'.", Path);
        return;
    }

    new Regex:RegEx_FileName, ret; RegEx_FileName = regex_compile("(.+).json$", ret, "", 0, "i");
    new JSON:Item, JSON:AbilsList;
    new Trie:DefWeaponsNamesList = TrieCreate();
    do{
        if (
            File[0] == '!'
            || Type != FileType_File
            || regex_match_c(File, RegEx_FileName) <= 0
        ) {
            continue;
        }

        new Data[CWAPI_WeaponData];
        Data[CWAPI_WD_CustomHandlers] = Invalid_Array;

        regex_substr(RegEx_FileName, 1, Data[CWAPI_WD_Name], charsmax(Data[CWAPI_WD_Name]));

        format(File, charsmax(File), "%s%s", Path, File);
        Item = json_parse(File, true, true);
        if (Item == Invalid_JSON) {
            log_amx("[WARNING] Invalid JSON syntax. File '%s'.", File);
            continue;
        }

        if (!json_is_object(Item)) {
            json_free(Item);
            log_amx("[WARNING] Invalid config structure. File '%s'.", File);
            continue;
        }

        json_object_get_string(Item, "DefaultName", Data[CWAPI_WD_DefaultName], charsmax(Data[CWAPI_WD_DefaultName]));

        if (file_exists(fmt("sprites/weapon_%s.txt", Data[CWAPI_WD_Name]))) {
            precache_generic(fmt("sprites/weapon_%s.txt", Data[CWAPI_WD_Name]));
            Data[CWAPI_WD_HasCustomHud] = true;
            register_clcmd(GetWeapFullName(Data[CWAPI_WD_Name]), "Cmd_Select");
        } else {
            Data[CWAPI_WD_HasCustomHud] = false;
        }

        if (json_object_has_value(Item, "Hud", JSONArray)) {
            new JSON:Hud = json_object_get_value(Item, "Hud");

            #define GetFullSprName(%1) fmt("sprites/%s.spr",%1)
            for (new i = 0; i < json_array_get_count(Hud); i++) {
                new SprName[32]; json_array_get_string(Hud, i, SprName, charsmax(SprName));
                if (file_exists(GetFullSprName(SprName))) {
                    precache_generic(GetFullSprName(SprName));
                } else {
                    log_amx("[WARNING] Sprite file '%s' not found.", GetFullSprName(SprName));
                }
            }
            
            json_free(Hud);
        }

        if (json_object_has_value(Item, "Models", JSONObject)) {
            new JSON:Models = json_object_get_value(Item, "Models");

            json_object_get_string(Models, "v", Data[CWAPI_WD_Models][CWAPI_WM_V], PLATFORM_MAX_PATH-1);
            if (Data[CWAPI_WD_Models][CWAPI_WM_V][0])
                if (file_exists(Data[CWAPI_WD_Models][CWAPI_WM_V])) {
                    precache_model(Data[CWAPI_WD_Models][CWAPI_WM_V]);
                } else {
                    log_amx("[WARNING] Model file `%s` not found. Weapon `%s`.", Data[CWAPI_WD_Models][CWAPI_WM_V], Data[CWAPI_WD_Name]);
                    formatex(Data[CWAPI_WD_Models][CWAPI_WM_V], PLATFORM_MAX_PATH-1, "");
                }

            json_object_get_string(Models, "p", Data[CWAPI_WD_Models][CWAPI_WM_P], PLATFORM_MAX_PATH-1);
            if (Data[CWAPI_WD_Models][CWAPI_WM_P][0]) {
                if (file_exists(Data[CWAPI_WD_Models][CWAPI_WM_P])) {
                    precache_model(Data[CWAPI_WD_Models][CWAPI_WM_P]);
                } else{
                    log_amx("[WARNING] Model file `%s` not found. Weapon `%s`.", Data[CWAPI_WD_Models][CWAPI_WM_P], Data[CWAPI_WD_Name]);
                    formatex(Data[CWAPI_WD_Models][CWAPI_WM_P], PLATFORM_MAX_PATH-1, "");
                }
            }

            json_object_get_string(Models, "w", Data[CWAPI_WD_Models][CWAPI_WM_W], PLATFORM_MAX_PATH-1);
            if (Data[CWAPI_WD_Models][CWAPI_WM_W][0]) {
                if (file_exists(Data[CWAPI_WD_Models][CWAPI_WM_W])) {
                    precache_model(Data[CWAPI_WD_Models][CWAPI_WM_W]);
                } else{
                    log_amx("[WARNING] Model file `%s` not found. Weapon `%s`.", Data[CWAPI_WD_Models][CWAPI_WM_W], Data[CWAPI_WD_Name]);
                    formatex(Data[CWAPI_WD_Models][CWAPI_WM_W], PLATFORM_MAX_PATH-1, "");
                }
            }
            
            json_free(Models);
        }

        if (json_object_has_value(Item, "Sounds", JSONObject)) {
            new JSON:Sounds = json_object_get_value(Item, "Sounds");

            json_object_get_string(Sounds, "ShotSilent", Data[CWAPI_WD_Sounds][CWAPI_WS_ShotSilent], PLATFORM_MAX_PATH-1);
            if (file_exists(fmt("sound/%s", Data[CWAPI_WD_Sounds][CWAPI_WS_ShotSilent]))) {
                precache_sound(Data[CWAPI_WD_Sounds][CWAPI_WS_ShotSilent]);
            } else if (Data[CWAPI_WD_Sounds][CWAPI_WS_ShotSilent][0]) {
                log_amx("[WARNING] Sound file '%s' not found.", Data[CWAPI_WD_Sounds][CWAPI_WS_ShotSilent]);
                formatex(Data[CWAPI_WD_Sounds][CWAPI_WS_ShotSilent], PLATFORM_MAX_PATH-1, "");
            }

            json_object_get_string(Sounds, "Shot", Data[CWAPI_WD_Sounds][CWAPI_WS_Shot], PLATFORM_MAX_PATH-1);
            if (file_exists(fmt("sound/%s", Data[CWAPI_WD_Sounds][CWAPI_WS_Shot]))) {
                precache_sound(Data[CWAPI_WD_Sounds][CWAPI_WS_Shot]);
            } else if (Data[CWAPI_WD_Sounds][CWAPI_WS_Shot][0]) {
                log_amx("[WARNING] Sound file '%s' not found.", Data[CWAPI_WD_Sounds][CWAPI_WS_Shot]);
                formatex(Data[CWAPI_WD_Sounds][CWAPI_WS_Shot], PLATFORM_MAX_PATH-1, "");
            }

            if (json_object_has_value(Sounds, "OnlyPrecache", JSONArray)) {
                new JSON:OnlyPrecache = json_object_get_value(Sounds, "OnlyPrecache");
                
                new Temp[PLATFORM_MAX_PATH];
                for (new k = 0; k < json_array_get_count(OnlyPrecache); k++) {
                    json_array_get_string(OnlyPrecache, k, Temp, PLATFORM_MAX_PATH-1);
                    if (file_exists(fmt("sound/%s", Temp))) {
                        precache_sound(Temp);
                    } else {
                        log_amx("[WARNING] Sound file '%s' not found.", fmt("sound/%s", Temp));
                    }
                }

                json_free(OnlyPrecache);
            }

            json_free(Sounds);
        }

        Data[CWAPI_WD_ClipSize] = json_object_get_number(Item, "ClipSize");
        Data[CWAPI_WD_Weight] = json_object_get_number(Item, "Weight");

        Data[CWAPI_WD_Price] = json_object_get_num_def(Item, "Price", -1);
        Data[CWAPI_WD_MaxAmmo] = json_object_get_num_def(Item, "MaxAmmo", -1);

        Data[CWAPI_WD_MaxWalkSpeed] = json_object_get_real_def(Item, "MaxWalkSpeed", -1.0);
        Data[CWAPI_WD_DamageMult] = json_object_get_real_def(Item, "DamageMult", -1.0);
        Data[CWAPI_WD_Damage] = json_object_get_real_def(Item, "Damage", -1.0);
        Data[CWAPI_WD_Accuracy] = json_object_get_real_def(Item, "Accuracy", -1.0);
        Data[CWAPI_WD_DeployTime] = json_object_get_real_def(Item, "DeployTime", -1.0);
        Data[CWAPI_WD_ReloadTime] = json_object_get_real_def(Item, "ReloadTime", -1.0);
        Data[CWAPI_WD_PrimaryAttackRate] = json_object_get_real_def(Item, "PrimaryAttackRate", 0.0);
        Data[CWAPI_WD_SecondaryAttackRate] = json_object_get_real_def(Item, "SecondaryAttackRate", 0.0);

        Data[CWAPI_WD_HasSecondaryAttack] = json_object_get_bool(Item, "HasSecondaryAttack");

        if (!TrieKeyExists(DefWeaponsNamesList, Data[CWAPI_WD_DefaultName])) {
            for (new i = 0; i < sizeof _WEAPON_HOOKS; i++) {
                RegisterHam(
                    _WEAPON_HOOKS[i][Ham_WH_Hook],
                    GetWeapFullName(Data[CWAPI_WD_DefaultName]),
                    _WEAPON_HOOKS[i][Ham_WH_Func], _WEAPON_HOOKS[i][Ham_WH_Post]
                );
            }

            TrieSetCell(DefWeaponsNamesList, Data[CWAPI_WD_DefaultName], 0);
        }

        TrieSetCell(WeaponsNames, Data[CWAPI_WD_Name], ArrayPushArray(CustomWeapons, Data));

        if (json_object_has_value(Item, "Abilities")) {
            AbilsList = json_object_get_value(Item, "Abilities");
            LoadWeaponAbilities(Data[CWAPI_WD_Name], AbilsList);
            json_free(AbilsList);
        }
        
        json_free(Item);
    } while (next_file(DirHandler, File, charsmax(File), Type));

    regex_free(RegEx_FileName);
    close_dir(DirHandler);
    TrieDestroy(DefWeaponsNamesList);
}

LoadWeaponAbilities(const WeaponName[], const JSON:List) {
    new AbilData[CWAPI_WeaponAbilityData];
    copy(AbilData[CWAPI_WAD_WeaponName], charsmax(AbilData[CWAPI_WAD_WeaponName]), WeaponName);

    if (json_is_array(List)) {
        AbilData[CWAPI_WAD_CustomData] = Invalid_Trie;
        new AbilityName[32], Array:WeaponsList = Invalid_Array;

        for (new i = 0; i < json_array_get_count(List); i++) {
            json_array_get_string(List, i, AbilityName, charsmax(AbilityName));
            if (!TrieGetCell(WeaponAbilities, AbilityName, WeaponsList)) {
                WeaponsList = ArrayCreate(CWAPI_WeaponAbilityData, 2);
            }
            
            ArrayPushArray(WeaponsList, AbilData);
            TrieSetCell(WeaponAbilities, AbilityName, WeaponsList);
        }
    }
    else if (json_is_object(List)) {
        AbilData[CWAPI_WAD_CustomData] = TrieCreate();
        new JSON:Abil, AbilityName[32], Array:WeaponsList = Invalid_Array, ParamName[32], ParamValue[32];

        for (new i = 0; i < json_object_get_count(List); i++) {
            json_object_get_name(List, i, AbilityName, charsmax(AbilityName));

            if (!TrieGetCell(WeaponAbilities, AbilityName, WeaponsList)) {
                WeaponsList = ArrayCreate(CWAPI_WeaponAbilityData, 4);
            }

            Abil = json_object_get_value(List, AbilityName);
            if (json_is_object(Abil)) {
                for (new j = 0; j < json_object_get_count(Abil); j++) {
                    json_object_get_name(Abil, j, ParamName, charsmax(ParamName));

                    json_object_get_string(Abil, ParamName, ParamValue, charsmax(ParamValue));
                    TrieSetString(AbilData[CWAPI_WAD_CustomData], ParamName, ParamValue);
                }
            }
            json_free(Abil);

            ArrayPushArray(WeaponsList, AbilData);
            TrieSetCell(WeaponAbilities, AbilityName, WeaponsList);
        }
    }
}

#include "Cwapi/Core/Natives"
