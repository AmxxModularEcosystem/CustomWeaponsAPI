#include <amxmodx>
#include <reapi>
#include <hamsandwich>
#include <json>

#pragma semicolon 1

#define WEAPONS_IMPULSE_OFFSET 4354
#define GetWeapFullName(%0) fmt("weapon_%s",%0)
#define json_object_get_type(%0,%1) json_get_type(json_object_get_value(%0,%1));
#define CUSTOM_WEAPONS_COUNT ArraySize(CustomWeapons)
#define GetWeapId(%0) get_entvar(%0,var_impulse)-WEAPONS_IMPULSE_OFFSET
#define IsCustomWeapon(%0) (0 <= %0 <= CUSTOM_WEAPONS_COUNT)


enum E_WeaponModels{
    WM_V[PLATFORM_MAX_PATH],
    WM_P[PLATFORM_MAX_PATH],
    WM_W[PLATFORM_MAX_PATH],
}

enum E_WeaponHandlers{
    WH_Shoot,
    WH_Reload,
    WH_Deploy,
    WH_Holster,
}

enum E_CustomHandlerData{
    CHD_Plugin[64],
    CHD_Function[64],
}

enum _:E_WeaponData{
    WD_Name[32],
    WD_DefaultName[32],
    WD_Models[E_WeaponModels],
    WD_ClipSize,
    WD_MaxAmmo,
    Float:WD_MaxWalkSpeed,
    WD_Weight,
    Trie:WD_CustomHandlers[E_WeaponHandlers],
    Float:WD_DamageMult,
}

new Trie:WeaponsNames;
new Array:CustomWeapons;

new const PLUG_NAME[] = "Custom Weapons API";
new const PLUG_VER[] = "0.1";

public plugin_init(){
    register_plugin(PLUG_NAME, PLUG_VER, "ArKaNeMaN");

    register_clcmd("CWAPI_Give", "Cmd_GiveCustomWeapon");

    server_print("[%s v%s] loaded.", PLUG_NAME, PLUG_VER);
}

public plugin_precache(){
    LoadWeapons();
}

public Cmd_GiveCustomWeapon(const Id){
    static WeaponName[32]; read_argv(1, WeaponName, charsmax(WeaponName));
    if(TrieKeyExists(WeaponsNames, WeaponName)){
        static WeaponId; TrieGetCell(WeaponsNames, WeaponName, WeaponId);
        GiveCustomWeapon(Id, WeaponId);
    }
    else client_print_color(Id, print_team_default, "Оружие ^4%s ^3не найдено", WeaponName);
}

public PlayerItemDeploy(const ItemId){
    if(!IsCustomWeapon(GetWeapId(ItemId))) return HAM_IGNORED;
    static Id; Id = get_member(ItemId, m_pPlayer);
    if(!is_user_connected(Id)) return HAM_IGNORED;
    
    static Data[E_WeaponData]; ArrayGetArray(CustomWeapons, GetWeapId(ItemId), Data);

    if(Data[WD_Models][WM_V][0]) set_entvar(Id, var_viewmodel, Data[WD_Models][WM_V][0]);
    if(Data[WD_Models][WM_P][0]) set_entvar(Id, var_weaponmodel, Data[WD_Models][WM_P][0]);

    return HAM_IGNORED;
}

//public PlayerItemHolster(const ItemId){
//    if(!IsCustomWeapon(GetWeapId(ItemId))) return HAM_IGNORED;
//    static Id; Id = get_member(ItemId, m_pPlayer);
//    if(!is_user_connected(Id)) return HAM_IGNORED;
//
//
//    return HAM_IGNORED;
//}
//
//public PlayerItemReloaded(const ItemId){
//    if(!IsCustomWeapon(GetWeapId(ItemId))) return HAM_IGNORED;
//    static Id; Id = get_member(ItemId, m_pPlayer);
//    if(!is_user_connected(Id)) return HAM_IGNORED;
//
//
//    return HAM_IGNORED;
//}

public PlayerGetMaxSpeed(const ItemId){
    if(!IsCustomWeapon(GetWeapId(ItemId))) return HAM_IGNORED;
    static Id; Id = get_member(ItemId, m_pPlayer);
    if(!is_user_connected(Id)) return HAM_IGNORED;
    
    static Data[E_WeaponData]; ArrayGetArray(CustomWeapons, GetWeapId(ItemId), Data);

    if(Data[WD_MaxWalkSpeed]){
        SetHamReturnFloat(Data[WD_MaxWalkSpeed]);
        return HAM_SUPERCEDE;
    }

    return HAM_IGNORED;
}

public Cmd_ChooseCustomWeapon(const Id){
    static Cmd[32]; read_argv(0, Cmd, charsmax(Cmd));
    if(equal(Cmd, "weapon_", 7) && TrieKeyExists(WeaponsNames, Cmd[7])){
        static WeaponId; TrieGetCell(WeaponsNames, Cmd[7], WeaponId);
        static Data[E_WeaponData]; ArrayGetArray(CustomWeapons, WeaponId, Data);
        engclient_cmd(Id, Data[WD_DefaultName]);
    }
}

GiveCustomWeapon(const Id, const WeaponId){
    if(!IsCustomWeapon(Id)) return -1;

    new Data[E_WeaponData]; ArrayGetArray(CustomWeapons, WeaponId, Data);

    new ItemId = rg_give_custom_item(Id, GetWeapFullName(Data[WD_DefaultName]), GT_DROP_AND_REPLACE, WeaponId+WEAPONS_IMPULSE_OFFSET);
    if(is_nullent(ItemId)) return -1;

    static WeaponIdType:DefaultWeaponId; DefaultWeaponId = WeaponIdType:rg_get_iteminfo(ItemId, ItemInfo_iId);

    if(Data[WD_Weight]) rg_set_iteminfo(ItemId, ItemInfo_iWeight, Data[WD_Weight]);
    
    if(Data[WD_ClipSize]){
        rg_set_iteminfo(ItemId, ItemInfo_iMaxClip, Data[WD_ClipSize]);
        rg_set_user_ammo(Id, DefaultWeaponId, Data[WD_ClipSize]);
    }

    if(Data[WD_MaxAmmo]){
        rg_set_iteminfo(ItemId, ItemInfo_iMaxAmmo1, Data[WD_MaxAmmo]);
        rg_set_user_bpammo(Id, DefaultWeaponId, Data[WD_MaxAmmo]);
    }

    if(Data[WD_DamageMult]){
        set_member(ItemId, m_Weapon_flBaseDamage, Float:get_member(ItemId, m_Weapon_flBaseDamage)*Data[WD_DamageMult]);

        if(DefaultWeaponId == WEAPON_M4A1) set_member(ItemId, m_M4A1_flBaseDamageSil, Float:get_member(ItemId, m_M4A1_flBaseDamageSil)*Data[WD_DamageMult]);
        else if(DefaultWeaponId == WEAPON_USP) set_member(ItemId, m_USP_flBaseDamageSil, Float:get_member(ItemId, m_USP_flBaseDamageSil)*Data[WD_DamageMult]);
        else if(DefaultWeaponId == WEAPON_FAMAS) set_member(ItemId, m_Famas_flBaseDamageBurst, Float:get_member(ItemId, m_Famas_flBaseDamageBurst)*Data[WD_DamageMult]);
    }

    return ItemId;
}

GetItemFromWeaponBox(const WeaponBox) {
    for(new i = 0, ItemId; i < MAX_ITEM_TYPES; i++){
        ItemId = get_member(WeaponBox, m_WeaponBox_rgpPlayerItems, i);
        if(!is_nullent(ItemId)) return ItemId;
    }
    return NULLENT;
}

LoadWeapons(){
    CustomWeapons = ArrayCreate(E_WeaponData);
    WeaponsNames = TrieCreate();
    
    new file[PLATFORM_MAX_PATH];
    get_localinfo("amxx_configsdir", file, charsmax(file));
    add(file, charsmax(file), "/plugins/CustomWeaponsAPI/Weapons.json");
    if(!file_exists(file)){
        set_fail_state("[ERROR] Config file '%s' not found", file);
        return;
    }
    new JSON:List = json_parse(file, true);
    if(!json_is_array(List)){
        json_free(List);
        set_fail_state("[ERROR] Invalid config structure. File '%s'", file);
        return;
    }
    new Data[E_WeaponData], JSON:Item;
    for(new i = 0; i < json_array_get_count(List); i++){
        Item = json_array_get_value(List, i);
        if(!json_is_object(Item)){
            json_free(Item);
            set_fail_state("[WARNING] Invalid config structure. File '%s'. Item #%d", file, i);
            continue;
        }

        json_object_get_string(Item, "Name", Data[WD_Name], charsmax(Data[WD_Name]));
        if(TrieKeyExists(WeaponsNames, Data[WD_Name])){
            json_free(Item);
            set_fail_state("[WARNING] Duplicate weapon name '%s'. File '%s'. Item #%d", Data[WD_Name], file, i);
            continue;
        }

        json_object_get_string(Item, "DefaultName", Data[WD_DefaultName], charsmax(Data[WD_DefaultName]));

        if(json_object_has_value(Item, "Models", JSONObject)){
            new JSON:Models = json_object_get_value(Item, "Models");

            json_object_get_string(Models, "v", Data[WD_Models][WM_V], PLATFORM_MAX_PATH-1);
            if(file_exists(Data[WD_Models][WM_V])) precache_model(Data[WD_Models][WM_V]);
            else formatex(Data[WD_Models][WM_V], PLATFORM_MAX_PATH-1, "");
            log_amx("Loading models: v - %s", Data[WD_Models][WM_V]);

            json_object_get_string(Models, "p", Data[WD_Models][WM_P], PLATFORM_MAX_PATH-1);
            if(file_exists(Data[WD_Models][WM_P])) precache_model(Data[WD_Models][WM_P]);
            else formatex(Data[WD_Models][WM_P], PLATFORM_MAX_PATH-1, "");
            log_amx("Loading models: p - %s", Data[WD_Models][WM_P]);

            json_object_get_string(Models, "w", Data[WD_Models][WM_W], PLATFORM_MAX_PATH-1);
            if(file_exists(Data[WD_Models][WM_W])) precache_model(Data[WD_Models][WM_W]);
            else formatex(Data[WD_Models][WM_W], PLATFORM_MAX_PATH-1, "");
            log_amx("Loading models: w - %s", Data[WD_Models][WM_W]);

            json_free(Models);
        }

        Data[WD_ClipSize] = json_object_get_number(Item, "ClipSize");
        Data[WD_MaxAmmo] = json_object_get_number(Item, "MaxAmmo");
        Data[WD_Weight] = json_object_get_number(Item, "Weight");

        Data[WD_MaxWalkSpeed] = json_object_get_real(Item, "MaxWalkSpeed");
        Data[WD_DamageMult] = json_object_get_real(Item, "DamageMult");

        register_clcmd(GetWeapFullName(Data[WD_Name]), "Cmd_ChooseCustomWeapon");

        RegisterHam(Ham_Item_Deploy, GetWeapFullName(Data[WD_DefaultName]), "PlayerItemDeploy", true);
        //RegisterHam(Ham_Item_Holster, GetWeapFullName(Data[WD_DefaultName]), "PlayerItemHolster", true);
        //RegisterHam(Ham_Weapon_Reload, GetWeapFullName(Data[WD_DefaultName]), "PlayerItemReloaded", false);
        RegisterHam(Ham_CS_Item_GetMaxSpeed, GetWeapFullName(Data[WD_DefaultName]), "PlayerGetMaxSpeed", false);
        
        TrieSetCell(WeaponsNames, Data[WD_Name], ArrayPushArray(CustomWeapons, Data));
        json_free(Item);
    }
    json_free(List);
}