#include <amxmodx>
#include <reapi>
#include <hamsandwich>
#include <json>
#include <cwapi>

#pragma semicolon 1

#define DEBUG // Закомментировать чтобы запретить бесплатную выдачу пушек
//#define SUPPORT_RESTRICT // Поддержка запрещалки пушек

#define WEAPONS_IMPULSE_OFFSET 4354
#define GetWeapFullName(%0) fmt("weapon_%s",%0)
#define CUSTOM_WEAPONS_COUNT ArraySize(CustomWeapons)
#define GetWeapId(%0) get_entvar(%0,var_impulse)-WEAPONS_IMPULSE_OFFSET
#define IsCustomWeapon(%0) (0 <= %0 < CUSTOM_WEAPONS_COUNT)
#define IsGrenade(%0) (equal(%0, "hegrenade") || equal(%0, "smokegrenade") || equal(%0, "flashbang"))

#if defined SUPPORT_RESTRICT
    forward WeaponsRestrict_LoadingWeapons_Post();
    native WeaponsRestrict_AddWeapon(const WeaponId, const WeaponName[32]);
#endif

enum E_Fwds{
    F_LoadWeaponsPost,
};

new Trie:WeaponsNames;
new Array:CustomWeapons;

new Fwds[E_Fwds];

new const PLUG_NAME[] = "Custom Weapons API";
new const PLUG_VER[] = "0.1.1";

public plugin_init(){
    
    RegisterHookChain(RG_CWeaponBox_SetModel, "Hook_WeaponBoxSetModel", false);
    RegisterHookChain(RG_CWeaponBox_SetModel, "Hook_WeaponBoxSetModel_Post", true);
    RegisterHookChain(RG_CBasePlayer_AddPlayerItem, "Hook_PlayerAddItem", true);
    RegisterHookChain(RG_CBasePlayer_TakeDamage, "Hook_PlayerTakeDamage", false);

    // Покупка пушки (Только если указана цена)
    register_clcmd("CWAPI_Buy", "Cmd_Buy");
    #if defined DEBUG
        // Бесплатная выдача пушки (Для тестов)
        register_clcmd("CWAPI_Give", "Cmd_GiveCustomWeapon");
    #endif

    server_print("[%s v%s] loaded.", PLUG_NAME, PLUG_VER);
}

public plugin_precache(){
    register_plugin(PLUG_NAME, PLUG_VER, "ArKaNeMaN");
    
    InitForwards();
    LoadWeapons();
    if(CUSTOM_WEAPONS_COUNT < 1) set_fail_state("[WARNING] No loaded weapons");
}

public plugin_natives(){
    register_native("CWAPI_RegisterHook", "Native_RegisterHook");
    register_native("CWAPI_GiveWeapon", "Native_GiveWeapon");
    register_native("CWAPI_GetWeaponsList", "Native_GetWeaponsList");
    register_native("CWAPI_GetWeaponData", "Native_GetWeaponData");
}

public Native_GiveWeapon(){
    static UserId; UserId = get_param(1);
    static WeaponName[32]; get_string(2, WeaponName, charsmax(WeaponName));
    if(!TrieKeyExists(WeaponsNames, WeaponName)){
        log_error(1, "Weapon '%s' not found", WeaponName);
        return -1;
    }
    static WeaponId; TrieGetCell(WeaponsNames, WeaponName, WeaponId);
    return GiveCustomWeapon(UserId, WeaponId);
}

public Native_RegisterHook(const PluginId, const Params){
    new WeaponName[32]; get_string(1, WeaponName, charsmax(WeaponName));
    new CWAPI_WeaponEvents:Event = CWAPI_WeaponEvents:get_param(2);
    new FuncName[64]; get_string(3, FuncName, charsmax(FuncName));

    //log_amx("[Native_RegisterHook] 1 [PluginId= %d || WeaponName = %s | Event = %d | FuncName = %s]", PluginId, WeaponName, _:Event, FuncName);

    if(!TrieKeyExists(WeaponsNames, WeaponName)) return -1;
    new WeaponId; TrieGetCell(WeaponsNames, WeaponName, WeaponId);
    new Data[CWAPI_WeaponData]; ArrayGetArray(CustomWeapons, WeaponId, Data);

    //log_amx("[Native_RegisterHook] 2 [WeaponId = %d]", WeaponId);

    if(Data[CWAPI_WD_CustomHandlers][Event] == Invalid_Array){
        Data[CWAPI_WD_CustomHandlers][Event] = ArrayCreate();
        ArraySetArray(CustomWeapons, WeaponId, Data);
    }
    
    new FwdId = _CreateOneForward(PluginId, FuncName, Event);
    new HandlerId = ArrayPushCell(Data[CWAPI_WD_CustomHandlers][Event], _:FwdId);

    //log_amx("[Native_RegisterHook] 3 [HandlerId = %d | FwdId = %d] [From Array: FwdId = %d]", HandlerId, FwdId, ArrayGetCell(Data[CWAPI_WD_CustomHandlers][Event], HandlerId));

    return HandlerId;
}

public Array:Native_GetWeaponsList(){
    return ArrayClone(CustomWeapons);
}

public Native_GetWeaponData(){
    static WeaponData[CWAPI_WeaponData];
    ArrayGetArray(CustomWeapons, get_param(1), WeaponData);
    return set_array(2, WeaponData, CWAPI_WeaponData);
}

CallWeaponEvent(const WeaponId, const CWAPI_WeaponEvents:Event, const ItemId, Array:Params = Invalid_Array){
    static Data[CWAPI_WeaponData]; ArrayGetArray(CustomWeapons, WeaponId, Data);

    if(Data[CWAPI_WD_CustomHandlers][Event] == Invalid_Array) return true;

    //log_amx("[CallWeaponEvent] 1 [WeaponId = %d | Event = %d | ItemId = %d]", WeaponId, _:Event, ItemId);

    static FwdId, Return;
    for(new i = 0; i < ArraySize(Data[CWAPI_WD_CustomHandlers][Event]); i++){
        FwdId = ArrayGetCell(Data[CWAPI_WD_CustomHandlers][Event], i);
        //log_amx("[CallWeaponEvent] 2 - %d [FwdId = %d]", i, FwdId);
        _ExecuteForward(FwdId, Return, Event, ItemId, Params);
        //log_amx("[CallWeaponEvent] 3 - %d [Return = %d | Status = %s]", i, Return, Status);
        if(Return == CWAPI_RET_HANDLED) break;
    }
    if(Params != Invalid_Array) ArrayDestroy(Params);
    if(Return == CWAPI_RET_HANDLED) return false;
    return true;
}

_CreateOneForward(const PluginId, const FuncName[], const CWAPI_WeaponEvents:Event){
    switch(Event){
        case CWAPI_WE_PrimaryAttack: return CreateOneForward(PluginId, FuncName, FP_CELL);
        case CWAPI_WE_SecondaryAttack: return CreateOneForward(PluginId, FuncName, FP_CELL);
        case CWAPI_WE_Reload: return CreateOneForward(PluginId, FuncName, FP_CELL);
        case CWAPI_WE_Deploy: return CreateOneForward(PluginId, FuncName, FP_CELL);
        case CWAPI_WE_Holster: return CreateOneForward(PluginId, FuncName, FP_CELL);
        case CWAPI_WE_Damage: return CreateOneForward(PluginId, FuncName, FP_CELL, FP_CELL, FP_FLOAT, FP_CELL);
        case CWAPI_WE_Droped: return CreateOneForward(PluginId, FuncName, FP_CELL, FP_CELL);
        case CWAPI_WE_AddItem: return CreateOneForward(PluginId, FuncName, FP_CELL, FP_CELL);
        case CWAPI_WE_Take: return CreateOneForward(PluginId, FuncName, FP_CELL, FP_CELL);
    }
    return log_error(0, "Undefined weapon event '%d'", _:Event);
}

_ExecuteForward(const FwdId, &Return, const CWAPI_WeaponEvents:Event, const ItemId, const Array:Params = Invalid_Array){
    Return = CWAPI_RET_CONTINUE;
    switch(Event){
        case CWAPI_WE_PrimaryAttack: return ExecuteForward(FwdId, Return, ItemId);
        case CWAPI_WE_SecondaryAttack: return ExecuteForward(FwdId, Return, ItemId);
        case CWAPI_WE_Reload: return ExecuteForward(FwdId, Return, ItemId);
        case CWAPI_WE_Deploy: return ExecuteForward(FwdId, Return, ItemId);
        case CWAPI_WE_Holster: return ExecuteForward(FwdId, Return, ItemId);
        case CWAPI_WE_Damage: return ExecuteForward(FwdId, Return, ItemId, ArrayGetCell(Params, 0), ArrayGetCell(Params, 1), ArrayGetCell(Params, 2));
        case CWAPI_WE_Droped: return ExecuteForward(FwdId, Return, ItemId, ArrayGetCell(Params, 0));
        case CWAPI_WE_AddItem: return ExecuteForward(FwdId, Return, ItemId, ArrayGetCell(Params, 0));
        case CWAPI_WE_Take: return ExecuteForward(FwdId, Return, ItemId, ArrayGetCell(Params, 0));
    }
    return 1;
}

public Hook_WeaponBoxSetModel(const WeaponBox){
    static ItemId;
    if(!(ItemId = GetItemFromWeaponBox(WeaponBox))) return HC_CONTINUE;
    static WeaponId; WeaponId = GetWeapId(ItemId);
    if(!IsCustomWeapon(WeaponId)) return HC_CONTINUE;

    static Data[CWAPI_WeaponData]; ArrayGetArray(CustomWeapons, WeaponId, Data);

    if(Data[CWAPI_WD_Models][CWAPI_WM_W][0]) SetHookChainArg(2, ATYPE_STRING, Data[CWAPI_WD_Models][CWAPI_WM_W], PLATFORM_MAX_PATH-1);

    return HC_SUPERCEDE;
}

public Hook_WeaponBoxSetModel_Post(const WeaponBox){
    static ItemId;
    if(!(ItemId = GetItemFromWeaponBox(WeaponBox))) return HC_CONTINUE;
    static WeaponId; WeaponId = GetWeapId(ItemId);
    if(!IsCustomWeapon(WeaponId)) return HC_CONTINUE;

    static Array:Params; Params = ArrayCreate();
    ArrayPushCell(Params, WeaponBox);

    CallWeaponEvent(WeaponId, CWAPI_WE_Droped, ItemId, Params);

    return HC_CONTINUE;
}

#if defined DEBUG
    public Cmd_GiveCustomWeapon(const Id){
        static WeaponName[32]; read_argv(1, WeaponName, charsmax(WeaponName));
        if(TrieKeyExists(WeaponsNames, WeaponName)){
            static WeaponId; TrieGetCell(WeaponsNames, WeaponName, WeaponId);
            if(GiveCustomWeapon(Id, WeaponId) != -1) client_print_color(Id, print_team_default, "^3Вы взяли ^4%s", WeaponName);
            else client_print_color(Id, print_team_default, "^3При выдаче возникла ошибка");
            return PLUGIN_HANDLED;
        }
        client_print_color(Id, print_team_default, "^3Оружие ^4%s ^3не найдено", WeaponName);
        return PLUGIN_CONTINUE;
    }
#endif

public Cmd_Buy(const Id){
    if(!is_user_alive(Id)){
        client_print(Id, print_center, "Вы мертвы");
        return PLUGIN_HANDLED;
    }
    if(!IsUserInBuyZone(Id)){
        client_print(Id, print_center, "Вы не в зоне покупки");
        return PLUGIN_HANDLED;
    }
    static WeaponName[32]; read_argv(1, WeaponName, charsmax(WeaponName));
    if(!TrieKeyExists(WeaponsNames, WeaponName)) return PLUGIN_CONTINUE;
    static WeaponId; TrieGetCell(WeaponsNames, WeaponName, WeaponId);
    static Data[CWAPI_WeaponData]; ArrayGetArray(CustomWeapons, WeaponId, Data);
    if(!Data[CWAPI_WD_Price]){
        client_print_color(Id, print_team_default, "^3Оружие ^4%s ^3нельзя купить", WeaponName);
        return PLUGIN_HANDLED;
    }
    if(get_member(Id, m_iAccount) < Data[CWAPI_WD_Price]){
        client_print_color(Id, print_team_default, "^3У вас недостаточно средств для покупки ^4%s", WeaponName);
        return PLUGIN_HANDLED;
    }
    if(GiveCustomWeapon(Id, WeaponId) != -1){
        rg_add_account(Id, -Data[CWAPI_WD_Price], AS_ADD);
        client_print_color(Id, print_team_default, "^3Вы купили ^4%s ^3за ^4$%d", WeaponName, Data[CWAPI_WD_Price]);
    }
    else client_print_color(Id, print_team_default, "^3При покупке возникла ошибка");
    return PLUGIN_HANDLED;
}

public Hook_PlayerAddItem(const UserId, const ItemId){
    static WeaponId; WeaponId = GetWeapId(ItemId);
    if(!IsCustomWeapon(WeaponId)) return HC_CONTINUE;
    if(!is_user_connected(UserId)) return HC_CONTINUE;

    static Array:Params; Params = ArrayCreate();
    ArrayPushCell(Params, UserId);

    CallWeaponEvent(GetWeapId(ItemId), CWAPI_WE_AddItem, ItemId, Params);

    return HC_CONTINUE;
}

public Hook_PlayerTakeDamage(const Victim, Inflictor, Attacker, Float:Damage, DamageBits){
    if(DamageBits & DMG_GRENADE) return HC_CONTINUE;
    if(!is_user_connected(Victim) || !is_user_connected(Attacker)) return HC_CONTINUE;
    static ItemId; ItemId = get_member(Attacker, m_pActiveItem);
    if(is_nullent(ItemId)) return HC_CONTINUE;
    static WeaponId; WeaponId = GetWeapId(ItemId);
    if(!IsCustomWeapon(WeaponId)) return HC_CONTINUE;

    static Array:Params; Params = ArrayCreate();
    ArrayPushCell(Params, Victim);
    ArrayPushCell(Params, Damage);
    ArrayPushCell(Params, DamageBits);

    if(!CallWeaponEvent(WeaponId, CWAPI_WE_Damage, ItemId, Params)){
        SetHookChainReturn(ATYPE_INTEGER, 0);
        return HC_SUPERCEDE;
    }

    return HC_CONTINUE;
}

public Hook_PlayerItemDeploy(const ItemId){
    if(!IsCustomWeapon(GetWeapId(ItemId))) return HAM_IGNORED;
    static Id; Id = get_member(ItemId, m_pPlayer);
    if(!is_user_connected(Id)) return HAM_IGNORED;
    
    static Data[CWAPI_WeaponData]; ArrayGetArray(CustomWeapons, GetWeapId(ItemId), Data);

    if(Data[CWAPI_WD_Models][CWAPI_WM_V][0]) set_entvar(Id, var_viewmodel, Data[CWAPI_WD_Models][CWAPI_WM_V][0]);
    if(Data[CWAPI_WD_Models][CWAPI_WM_P][0]) set_entvar(Id, var_weaponmodel, Data[CWAPI_WD_Models][CWAPI_WM_P][0]);
    
    if(Data[CWAPI_WD_DeployTime] > 0.0) SetWeaponNextAttack(ItemId, Data[CWAPI_WD_DeployTime]);

    CallWeaponEvent(GetWeapId(ItemId), CWAPI_WE_Deploy, ItemId);

    return HAM_IGNORED;
}

public Hook_PlayerItemHolster(const ItemId){
    if(!IsCustomWeapon(GetWeapId(ItemId))) return HAM_IGNORED;

    CallWeaponEvent(GetWeapId(ItemId), CWAPI_WE_Holster, ItemId);

    return HAM_IGNORED;
}

public Hook_PlayerItemReloaded(const ItemId){
    if(!IsCustomWeapon(GetWeapId(ItemId))) return HAM_IGNORED;
    if(get_member(ItemId, m_Weapon_iClip) >= rg_get_iteminfo(ItemId, ItemInfo_iMaxClip)) return HAM_SUPERCEDE;

    CallWeaponEvent(GetWeapId(ItemId), CWAPI_WE_Reload, ItemId);

    log_amx("Hook_PlayerItemReloaded: m_Weapon_flNextPrimaryAttack = %.2f", get_member(ItemId, m_Weapon_flNextPrimaryAttack));
    log_amx("Hook_PlayerItemReloaded: m_Weapon_fInReload = %d", get_member(ItemId, m_Weapon_fInReload));
    log_amx("Hook_PlayerItemReloaded: m_Weapon_iClip = %d", get_member(ItemId, m_Weapon_iClip));
    log_amx("Hook_PlayerItemReloaded: m_Weapon_iClientClip = %d", get_member(ItemId, m_Weapon_iClientClip));
    log_amx("Hook_PlayerItemReloaded: m_Weapon_flNextReload = %d", get_member(ItemId, m_Weapon_flNextReload));
    log_amx("Hook_PlayerItemReloaded: ===================================================");

    return HAM_IGNORED;
}

public Hook_PlayerGetMaxSpeed(const ItemId){
    if(!IsCustomWeapon(GetWeapId(ItemId))) return HAM_IGNORED;
    
    static Data[CWAPI_WeaponData]; ArrayGetArray(CustomWeapons, GetWeapId(ItemId), Data);

    if(Data[CWAPI_WD_MaxWalkSpeed]){
        SetHamReturnFloat(Data[CWAPI_WD_MaxWalkSpeed]);
        return HAM_SUPERCEDE;
    }

    return HAM_IGNORED;
}

// Выдача пушки
GiveCustomWeapon(const Id, const WeaponId){
    if(!is_user_alive(Id)) return -1;
    if(!IsCustomWeapon(WeaponId)) return -1;

    static Array:Params; Params = ArrayCreate();
    ArrayPushCell(Params, Id);

    if(!CallWeaponEvent(WeaponId, CWAPI_WE_Take, WeaponId, Params)) return -1;

    static Data[CWAPI_WeaponData]; ArrayGetArray(CustomWeapons, WeaponId, Data);

    static GiveType:WeaponGiveType;
    if(equal(Data[CWAPI_WD_DefaultName], "knife")) WeaponGiveType = GT_REPLACE;
    else if(IsGrenade(Data[CWAPI_WD_DefaultName])) WeaponGiveType = GT_APPEND;
    else WeaponGiveType = GT_DROP_AND_REPLACE;

    static ItemId; ItemId = rg_give_custom_item(
        Id,
        GetWeapFullName(Data[CWAPI_WD_DefaultName]),
        WeaponGiveType,
        WeaponId+WEAPONS_IMPULSE_OFFSET
    );

    if(is_nullent(ItemId)) return -1;

    static WeaponIdType:DefaultWeaponId; DefaultWeaponId = WeaponIdType:rg_get_iteminfo(ItemId, ItemInfo_iId);

    if(Data[CWAPI_WD_HasSecondaryAttack]) set_member(ItemId, m_Weapon_bHasSecondaryAttack, Data[CWAPI_WD_HasSecondaryAttack]);
    if(Data[CWAPI_WD_Weight]) rg_set_iteminfo(ItemId, ItemInfo_iWeight, Data[CWAPI_WD_Weight]);
    
    if(DefaultWeaponId != WEAPON_KNIFE){
        if(Data[CWAPI_WD_ClipSize]){
            rg_set_iteminfo(ItemId, ItemInfo_iMaxClip, Data[CWAPI_WD_ClipSize]);
            rg_set_user_ammo(Id, DefaultWeaponId, Data[CWAPI_WD_ClipSize]);
        }

        if(Data[CWAPI_WD_MaxAmmo]){
            rg_set_iteminfo(ItemId, ItemInfo_iMaxAmmo1, Data[CWAPI_WD_MaxAmmo]);
            rg_set_user_bpammo(Id, DefaultWeaponId, Data[CWAPI_WD_MaxAmmo]);
        }
    }

    if(Data[CWAPI_WD_DamageMult]){
        set_member(ItemId, m_Weapon_flBaseDamage, Float:get_member(ItemId, m_Weapon_flBaseDamage)*Data[CWAPI_WD_DamageMult]);

        if(DefaultWeaponId == WEAPON_M4A1)
            set_member(ItemId, m_M4A1_flBaseDamageSil, Float:get_member(ItemId, m_M4A1_flBaseDamageSil)*Data[CWAPI_WD_DamageMult]);
        else if(DefaultWeaponId == WEAPON_USP)
            set_member(ItemId, m_USP_flBaseDamageSil, Float:get_member(ItemId, m_USP_flBaseDamageSil)*Data[CWAPI_WD_DamageMult]);
        else if(DefaultWeaponId == WEAPON_FAMAS)
            set_member(ItemId, m_Famas_flBaseDamageBurst, Float:get_member(ItemId, m_Famas_flBaseDamageBurst)*Data[CWAPI_WD_DamageMult]);
    }

    return ItemId;
}

// Получение ID итема из WeaponBox'а
GetItemFromWeaponBox(const WeaponBox){
    for(new i = 0, ItemId; i < MAX_ITEM_TYPES; i++){
        ItemId = get_member(WeaponBox, m_WeaponBox_rgpPlayerItems, i);
        if(!is_nullent(ItemId)) return ItemId;
    }
    return NULLENT;
}

// Надет ли глушитель
bool:IsWeaponSilenced(const ItemId){
    static WeaponState:State; State = get_member(ItemId, m_Weapon_iWeaponState);
    static bool:IsSilenced; IsSilenced = (State & WPNSTATE_USP_SILENCED || State & WPNSTATE_M4A1_SILENCED);
    return IsSilenced;
}

// В зоне закупки ли игрок
bool:IsUserInBuyZone(const Id){
    static Signal[UnifiedSignals]; get_member(Id, m_signals, Signal);
    return (Signal[US_Signal] == _:SIGNAL_BUY);
}

// Установка времени до следующего выстрела
SetWeaponNextAttack(ItemId, Float:Rate){
    set_member(ItemId, m_Weapon_flNextPrimaryAttack, Rate);
    set_member(ItemId, m_Weapon_flNextSecondaryAttack, Rate);
    //set_member(ItemId, m_Weapon_flTimeWeaponIdle, Rate);
}

// Загрузка пушек из кфг
LoadWeapons(){
    CustomWeapons = ArrayCreate(CWAPI_WeaponData);
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
    new Trie:DefWeaponsNamesList = TrieCreate();
    new Data[CWAPI_WeaponData], JSON:Item;
    for(new i = 0; i < json_array_get_count(List); i++){
        Item = json_array_get_value(List, i);
        if(!json_is_object(Item)){
            json_free(Item);
            set_fail_state("[WARNING] Invalid config structure. File '%s'. Item #%d", file, i);
            continue;
        }

        json_object_get_string(Item, "Name", Data[CWAPI_WD_Name], charsmax(Data[CWAPI_WD_Name]));
        if(TrieKeyExists(WeaponsNames, Data[CWAPI_WD_Name])){
            json_free(Item);
            set_fail_state("[WARNING] Duplicate weapon name '%s'. File '%s'. Item #%d", Data[CWAPI_WD_Name], file, i);
            continue;
        }

        json_object_get_string(Item, "DefaultName", Data[CWAPI_WD_DefaultName], charsmax(Data[CWAPI_WD_DefaultName]));

        if(json_object_has_value(Item, "Models", JSONObject)){
            new JSON:Models = json_object_get_value(Item, "Models");

            json_object_get_string(Models, "v", Data[CWAPI_WD_Models][CWAPI_WM_V], PLATFORM_MAX_PATH-1);
            if(file_exists(Data[CWAPI_WD_Models][CWAPI_WM_V])) precache_model(Data[CWAPI_WD_Models][CWAPI_WM_V]);
            else formatex(Data[CWAPI_WD_Models][CWAPI_WM_V], PLATFORM_MAX_PATH-1, "");

            json_object_get_string(Models, "p", Data[CWAPI_WD_Models][CWAPI_WM_P], PLATFORM_MAX_PATH-1);
            if(file_exists(Data[CWAPI_WD_Models][CWAPI_WM_P])) precache_model(Data[CWAPI_WD_Models][CWAPI_WM_P]);
            else formatex(Data[CWAPI_WD_Models][CWAPI_WM_P], PLATFORM_MAX_PATH-1, "");

            json_object_get_string(Models, "w", Data[CWAPI_WD_Models][CWAPI_WM_W], PLATFORM_MAX_PATH-1);
            if(file_exists(Data[CWAPI_WD_Models][CWAPI_WM_W])) precache_model(Data[CWAPI_WD_Models][CWAPI_WM_W]);
            else formatex(Data[CWAPI_WD_Models][CWAPI_WM_W], PLATFORM_MAX_PATH-1, "");
            
            json_free(Models);
        }

        if(json_object_has_value(Item, "Sounds", JSONObject)){
            new JSON:Sounds = json_object_get_value(Item, "Sounds");

            json_object_get_string(Sounds, "ShotSilent", Data[CWAPI_WD_Sounds][CWAPI_WS_ShotSilent], PLATFORM_MAX_PATH-1);
            if(file_exists(fmt("sound/%s", Data[CWAPI_WD_Sounds][CWAPI_WS_ShotSilent]))) precache_sound(Data[CWAPI_WD_Sounds][CWAPI_WS_ShotSilent]);
            else if(Data[CWAPI_WD_Sounds][CWAPI_WS_Shot][0]){
                log_amx("[WARNING] Sound file '%s' not found.", Data[CWAPI_WD_Sounds][CWAPI_WS_ShotSilent]);
                formatex(Data[CWAPI_WD_Sounds][CWAPI_WS_ShotSilent], PLATFORM_MAX_PATH-1, "");
            }

            json_object_get_string(Sounds, "Shot", Data[CWAPI_WD_Sounds][CWAPI_WS_Shot], PLATFORM_MAX_PATH-1);
            if(file_exists(fmt("sound/%s", Data[CWAPI_WD_Sounds][CWAPI_WS_Shot]))) precache_sound(Data[CWAPI_WD_Sounds][CWAPI_WS_Shot]);
            else if(Data[CWAPI_WD_Sounds][CWAPI_WS_Shot][0]){
                log_amx("[WARNING] Sound file '%s' not found.", Data[CWAPI_WD_Sounds][CWAPI_WS_Shot]);
                formatex(Data[CWAPI_WD_Sounds][CWAPI_WS_Shot], PLATFORM_MAX_PATH-1, "");
            }

            if(json_object_has_value(Sounds, "OnlyPrecache", JSONArray)){
                new JSON:OnlyPrecache = json_object_get_value(Sounds, "OnlyPrecache");
                
                new Temp[PLATFORM_MAX_PATH];
                for(new k = 0; k < json_array_get_count(OnlyPrecache); k++){
                    json_array_get_string(OnlyPrecache, k, Temp, PLATFORM_MAX_PATH-1);
                    if(file_exists(fmt("sound/%s", Temp))) precache_sound(Temp);
                    else log_amx("[WARNING] Sound file '%s' not found.", fmt("sound/%s", Temp));
                }

                json_free(OnlyPrecache);
            }

            json_free(Sounds);
        }

        Data[CWAPI_WD_ClipSize] = json_object_get_number(Item, "ClipSize");
        Data[CWAPI_WD_MaxAmmo] = json_object_get_number(Item, "MaxAmmo");
        Data[CWAPI_WD_Weight] = json_object_get_number(Item, "Weight");
        Data[CWAPI_WD_Price] = json_object_get_number(Item, "Price");

        Data[CWAPI_WD_MaxWalkSpeed] = json_object_get_real(Item, "MaxWalkSpeed");
        Data[CWAPI_WD_DamageMult] = json_object_get_real(Item, "DamageMult");
        Data[CWAPI_WD_Accuracy] = json_object_get_real(Item, "Accuracy");
        Data[CWAPI_WD_DeployTime] = json_object_get_real(Item, "DeployTime");
        Data[CWAPI_WD_PrimaryAttackRate] = json_object_get_real(Item, "PrimaryAttackRate");
        Data[CWAPI_WD_SecondaryAttackRate] = json_object_get_real(Item, "SecondaryAttackRate");

        Data[CWAPI_WD_HasSecondaryAttack] = json_object_get_bool(Item, "HasSecondaryAttack");

        if(!TrieKeyExists(DefWeaponsNamesList, Data[CWAPI_WD_DefaultName])){
            RegisterHam(
                Ham_Item_Deploy,
                GetWeapFullName(Data[CWAPI_WD_DefaultName]),
                "Hook_PlayerItemDeploy", true
            );
            RegisterHam(
                Ham_Item_Holster,
                GetWeapFullName(Data[CWAPI_WD_DefaultName]),
                "Hook_PlayerItemHolster", true
            );
            RegisterHam(
                Ham_Weapon_Reload,
                GetWeapFullName(Data[CWAPI_WD_DefaultName]),
                "Hook_PlayerItemReloaded", false
            );
            RegisterHam(
                Ham_CS_Item_GetMaxSpeed,
                GetWeapFullName(Data[CWAPI_WD_DefaultName]),
                "Hook_PlayerGetMaxSpeed", false
            );

            RegisterHam(
                Ham_Weapon_PrimaryAttack,
                GetWeapFullName(Data[CWAPI_WD_DefaultName]),
                "Hook_PrimaryAttack", true
            );
            RegisterHam(
                Ham_Weapon_SecondaryAttack,
                GetWeapFullName(Data[CWAPI_WD_DefaultName]),
                "Hook_SecondaryAttack", true
            );

            TrieSetCell(DefWeaponsNamesList, Data[CWAPI_WD_DefaultName], 0);
        }
        
        TrieSetCell(WeaponsNames, Data[CWAPI_WD_Name], ArrayPushArray(CustomWeapons, Data));
        json_free(Item);
    }
    json_free(List);

    TrieDestroy(DefWeaponsNamesList);

    //log_amx("Before call 'FCWAPI_WD_LoadWeaponsPost'");

    ExecuteForward(Fwds[F_LoadWeaponsPost]);

    //log_amx("After call 'FCWAPI_WD_LoadWeaponsPost' - Status = %d", Status);

    server_print("[%s v%s] %d custom weapons loaded from '%s'", PLUG_NAME, PLUG_VER, CUSTOM_WEAPONS_COUNT, file);
}

public Hook_PrimaryAttack(ItemId){
    if(!IsCustomWeapon(GetWeapId(ItemId))) return HAM_IGNORED;
    if(
        get_member(ItemId, m_Weapon_iClientClip) < 1 || 
        get_member(ItemId, m_Weapon_iClientClip) == get_member(ItemId, m_Weapon_iClip) || 
        get_member(ItemId, m_Weapon_fInReload)
    ) return HAM_IGNORED;
    static Data[CWAPI_WeaponData]; ArrayGetArray(CustomWeapons, GetWeapId(ItemId), Data);

    if(!CallWeaponEvent(GetWeapId(ItemId), CWAPI_WE_PrimaryAttack, ItemId)) return HAM_SUPERCEDE;

    //log_amx("Hook_PrimaryAttack: m_Weapon_flNextPrimaryAttack = %.2f", get_member(ItemId, m_Weapon_flNextPrimaryAttack));
    //log_amx("Hook_PrimaryAttack: m_Weapon_fInReload = %d", get_member(ItemId, m_Weapon_fInReload));
    //log_amx("Hook_PrimaryAttack: m_Weapon_iClip = %d", get_member(ItemId, m_Weapon_iClip));
    //log_amx("Hook_PrimaryAttack: m_Weapon_iClientClip = %d", get_member(ItemId, m_Weapon_iClientClip));
    //log_amx("Hook_PrimaryAttack: m_Weapon_iShotsFired = %d", get_member(ItemId, m_Weapon_iShotsFired));
    //log_amx("Hook_PrimaryAttack: m_Weapon_flNextReload = %d", get_member(ItemId, m_Weapon_flNextReload));
    //log_amx("Hook_PrimaryAttack: m_Weapon_bHasSecondaryAttack = %d", get_member(ItemId, m_Weapon_bHasSecondaryAttack));
    //log_amx("Hook_PrimaryAttack: ===================================================");

    if(Data[CWAPI_WD_PrimaryAttackRate] > 0.0) SetWeaponNextAttack(ItemId, Data[CWAPI_WD_PrimaryAttackRate]);

    static UserId; UserId = get_member(ItemId, m_pPlayer);

    if(IsWeaponSilenced(ItemId)) if(Data[CWAPI_WD_Sounds][CWAPI_WS_ShotSilent][0]) rh_emit_sound2(UserId, 0, CHAN_WEAPON, Data[CWAPI_WD_Sounds][CWAPI_WS_ShotSilent]);
    else if(Data[CWAPI_WD_Sounds][CWAPI_WS_Shot][0]) rh_emit_sound2(UserId, 0, CHAN_WEAPON, Data[CWAPI_WD_Sounds][CWAPI_WS_Shot]);

    return HAM_IGNORED;
}

public Hook_SecondaryAttack(ItemId){
    if(
        get_member(ItemId, m_Weapon_fInReload)
    ) return HAM_IGNORED;
    if(!IsCustomWeapon(GetWeapId(ItemId))) return HAM_IGNORED;
    static Data[CWAPI_WeaponData]; ArrayGetArray(CustomWeapons, GetWeapId(ItemId), Data);

    if(!CallWeaponEvent(GetWeapId(ItemId), CWAPI_WE_SecondaryAttack, ItemId)) return HAM_SUPERCEDE;

    //log_amx("Hook_PrimaryAttack: m_Weapon_flNextSecondaryAttack = %.2f", get_member(ItemId, m_Weapon_flNextSecondaryAttack));
    //log_amx("Hook_PrimaryAttack: m_Weapon_fInReload = %d", get_member(ItemId, m_Weapon_fInReload));
    //log_amx("Hook_PrimaryAttack: m_Weapon_iClip = %d", get_member(ItemId, m_Weapon_iClip));
    //log_amx("Hook_PrimaryAttack: m_Weapon_iClientClip = %d", get_member(ItemId, m_Weapon_iClientClip));
    //log_amx("Hook_PrimaryAttack: m_Weapon_iShotsFired = %d", get_member(ItemId, m_Weapon_iShotsFired));
    //log_amx("Hook_PrimaryAttack: m_Weapon_flNextReload = %d", get_member(ItemId, m_Weapon_flNextReload));
    //log_amx("Hook_PrimaryAttack: m_Weapon_bHasSecondaryAttack = %d", get_member(ItemId, m_Weapon_bHasSecondaryAttack));
    //log_amx("Hook_PrimaryAttack: ===================================================");

    if(Data[CWAPI_WD_SecondaryAttackRate] > 0.0) SetWeaponNextAttack(ItemId, Data[CWAPI_WD_SecondaryAttackRate]);
    
    return HAM_IGNORED;
}

InitForwards(){
    Fwds[F_LoadWeaponsPost] = CreateMultiForward("CWAPI_LoawWeaponsPost", ET_IGNORE);
}

#if defined SUPPORT_RESTRICT
    public WeaponsRestrict_LoadingWeapons_Post(){
        new WeaponName[32], WeaponId, TrieIter:IterHandler;
        IterHandler = TrieIterCreate(WeaponsNames);
        while(!TrieIterEnded(IterHandler)){
            TrieIterGetKey(IterHandler, WeaponName, charsmax(WeaponName));
            TrieIterGetCell(IterHandler, WeaponId);
            TrieIterNext(IterHandler);
        }
        TrieIterDestroy(IterHandler);
        WeaponsRestrict_AddWeapon(WeaponId+WEAPONS_IMPULSE_OFFSET, WeaponName);
    }
#endif