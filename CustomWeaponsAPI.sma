#include <amxmodx>
#define MEMBER_UNSAFE
#include <reapi>
#include <hamsandwich>
#include <json>
#include <regex>
#include <cwapi>

#pragma semicolon 1

// Поставить тут 0 чтобы нельзя было выдавать пушки
#define DEBUG 1

// Использование новых хуков в ReAPI (Почему-то работает криво)
#define USE_NEW_REAPI_HOOKS 0

// Проверка на нахождение в зоне закупки при покупке командой CWAPI_Buy
#define CHECK_BUYZONE 1

#define WEAPON_PISTOLS_BITSUMM (BIT(_:WEAPON_P228)|BIT(_:WEAPON_GLOCK)|BIT(_:WEAPON_ELITE)|BIT(_:WEAPON_FIVESEVEN)|BIT(_:WEAPON_USP)|BIT(_:WEAPON_GLOCK18)|BIT(_:WEAPON_DEAGLE))
#define GetWeapFullName(%0) fmt("weapon_%s",%0)
#define CUSTOM_WEAPONS_COUNT ArraySizeSafe(CustomWeapons)
#define GetWeapId(%0) get_entvar(%0,var_impulse)-CWAPI_IMPULSE_OFFSET
#define IsCustomWeapon(%0) (0 <= %0 < CUSTOM_WEAPONS_COUNT)
#define IsWeaponSilenced(%0) bool:((WPNSTATE_M4A1_SILENCED|WPNSTATE_USP_SILENCED)&get_member(%0,m_Weapon_iWeaponState))
#define IsPistol(%0) (WEAPON_PISTOLS_BITSUMM&BIT(rg_get_iteminfo(%0,ItemInfo_iId)))
#define IsGrenade(%0) (equal(%0, "hegrenade") || equal(%0, "smokegrenade") || equal(%0, "flashbang"))
new const _STR_NUM[] = "%d";
#define IntToStr(%0) fmt(_STR_NUM,%0)
#define json_object_get_real_def(%1,%2,%3) json_object_has_value(%1,%2,JSONNumber)?json_object_get_real(%1,%2):%3
#define json_object_get_num_def(%1,%2,%3) json_object_has_value(%1,%2,JSONNumber)?json_object_get_number(%1,%2):%3

enum {
    CWAPI_ERR_UNDEFINED_EVENT = 0,
    CWAPI_ERR_WEAPON_NOT_FOUND,
    CWAPI_ERR_CANT_EXECUTE_FWD,
    CWAPI_ERR_DUPLICATE_WEAPON_NAME,
    CWAPI_ERR_UNDEFINED_WEAPON_FIELD,
}

enum E_Fwds{
    F_LoadWeaponsPost,
};

enum E_UserMsgs{
    UM_WeaponList,
}

enum _E_Ham_WeaponHook{Ham:Ham_WH_Hook, Ham_WH_Func[64], bool:Ham_WH_Post,}
new const _WEAPON_HOOKS[][_E_Ham_WeaponHook] = {
    #if !USE_NEW_REAPI_HOOKS
    {Ham_Item_Deploy, "Hook_PlayerItemDeploy", true},
    {Ham_Weapon_Reload, "Hook_PlayerItemReloaded", false},
    {Ham_Weapon_Reload, "Hook_PlayerItemReloaded_Post", true},
    #endif
    {Ham_Item_Holster, "Hook_PlayerItemHolster", true},
    {Ham_CS_Item_GetMaxSpeed, "Hook_PlayerGetMaxSpeed", false},
    {Ham_Weapon_PrimaryAttack, "Hook_PrimaryAttack_Post", true},
    {Ham_Weapon_PrimaryAttack, "Hook_PrimaryAttack_Pre", false},
    {Ham_Weapon_SecondaryAttack, "Hook_SecondaryAttack", true},
    {Ham_Item_AddToPlayer, "Hook_AddItemToPlayer_Post", true},
};

new Trie:WeaponAbilities = Invalid_Trie;
new Trie:WeaponsNames = Invalid_Trie;
new Array:CustomWeapons = Invalid_Array;

new Fwds[E_Fwds];
new UserMsgs[E_UserMsgs];

public stock const PluginName[] = "Custom Weapons API";
public stock const PluginAuthor[] = "ArKaNeMaN";
public stock const PluginURL[] = "https://github.com/ArKaNeMaN/amxx-CustomWeaponsAPI";
public stock const PluginDescription[] = "API for create custom weapons";

public plugin_init() {
    register_dictionary("cwapi.txt");
    
    RegisterHookChain(RG_CWeaponBox_SetModel, "Hook_WeaponBoxSetModel", false);
    RegisterHookChain(RG_CWeaponBox_SetModel, "Hook_WeaponBoxSetModel_Post", true);
    RegisterHookChain(RG_CBasePlayer_AddPlayerItem, "Hook_PlayerAddItem", true);
    RegisterHookChain(RG_CBasePlayer_TakeDamage, "Hook_PlayerTakeDamage", false);
    RegisterHookChain(RG_CSGameRules_PlayerKilled, "Hook_PlayerKilled", true);
    #if USE_NEW_REAPI_HOOKS
        RegisterHookChain(RG_CBasePlayerWeapon_DefaultDeploy, "Hook_DefaultDeploy_Pre", false);
        RegisterHookChain(RG_CBasePlayerWeapon_DefaultDeploy, "Hook_DefaultDeploy_Post", true);
        RegisterHookChain(RG_CBasePlayerWeapon_DefaultReload, "Hook_DefaultReload_Pre", false);
        RegisterHookChain(RG_CBasePlayerWeapon_DefaultReload, "Hook_DefaultReload_Post", true);
        RegisterHookChain(RG_CBasePlayerWeapon_DefaultShotgunReload, "Hook_DefaultShotgunReload", false);
    #endif

    // Покупка пушки (Только если указана цена)
    register_clcmd("CWAPI_Buy", "Cmd_Buy");
    #if DEBUG
        // Бесплатная выдача пушки (Для тестов)
        register_clcmd("CWAPI_Give", "Cmd_GiveCustomWeapon");
    #endif

    // CWAPI_Srv_Give <UserId> <WeaponName>
    register_srvcmd("CWAPI_Srv_Give", "@SrvCmd_Give");

    UserMsgs[UM_WeaponList] = get_user_msgid("WeaponList");

    server_print("[%s v%s] loaded.", PluginName, CWAPI_VERSION);
}

public plugin_precache() {
    register_plugin(PluginName, CWAPI_VERSION, PluginAuthor);
    set_pcvar_string(create_cvar(CWAPI_VERSION_CVAR, CWAPI_VERSION, FCVAR_SERVER), CWAPI_VERSION);
    set_pcvar_num(create_cvar(CWAPI_VERSION_NUM_CVAR, IntToStr(CWAPI_VERSION_NUM), FCVAR_SERVER), CWAPI_VERSION_NUM);
    
    InitForwards();
    LoadWeapons();
    if (CUSTOM_WEAPONS_COUNT < 1) {
        set_fail_state("[WARNING] No loaded weapons");
    }

    server_print("[%s v%s] %d custom weapons loaded.", PluginName, CWAPI_VERSION, CUSTOM_WEAPONS_COUNT);
}

#if DEBUG
    public Cmd_GiveCustomWeapon(const Id) {
        new WeaponName[32];
        read_argv(1, WeaponName, charsmax(WeaponName));

        if (TrieKeyExists(WeaponsNames, WeaponName)) {
            new WeaponId;
            TrieGetCell(WeaponsNames, WeaponName, WeaponId);

            if (GiveCustomWeapon(Id, WeaponId, CWAPI_GT_SMART) != -1) {
                client_print_color(Id, print_team_default, "%L", LANG_PLAYER, "WEAPON_GIVE_SUCCESS", WeaponName);
            } else {
                client_print_color(Id, print_team_default, "%L", LANG_PLAYER, "WEAPON_GIVE_ERROR");
            }

            return PLUGIN_HANDLED;
        }

        client_print_color(Id, print_team_default, "%L", LANG_PLAYER, "WEAPON_NOT_FOUND", WeaponName);
        return PLUGIN_CONTINUE;
    }
#endif

@SrvCmd_Give() {
    enum {Arg_UserId = 1, Arg_WeaponName}
    new UserId = read_argv_int(Arg_UserId);
    new WeaponName[32];
    read_argv(Arg_WeaponName, WeaponName, charsmax(WeaponName));

    if (!is_user_alive(UserId)) {
        log_amx("[ERROR] [CMD] User #%d not found or not alive.", UserId);
        return PLUGIN_HANDLED;
    }

    if (!TrieKeyExists(WeaponsNames, WeaponName)) {
        log_amx("[ERROR] [CMD] Weapon `%s` not found.", WeaponName);
        return PLUGIN_HANDLED;
    }

    new WeaponId;
    TrieGetCell(WeaponsNames, WeaponName, WeaponId);
    GiveCustomWeapon(UserId, WeaponId, CWAPI_GT_SMART);

    return PLUGIN_CONTINUE;
}

public Cmd_Select(const UserId) {
    if (!is_user_alive(UserId)) {
        return PLUGIN_HANDLED;
    }

    new WeaponName[40];
    read_argv(0, WeaponName, charsmax(WeaponName));

    if (TrieKeyExists(WeaponsNames, WeaponName[7])) {
        new WeaponId, Data[CWAPI_WeaponData];
        TrieGetCell(WeaponsNames, WeaponName[7], WeaponId);
        ArrayGetArray(CustomWeapons, WeaponId, Data);

        engclient_cmd(UserId, GetWeapFullName(Data[CWAPI_WD_DefaultName]));
        return PLUGIN_HANDLED_MAIN;
    }

    return PLUGIN_CONTINUE;
}

public Cmd_Buy(const Id) {
    if (!is_user_alive(Id)) {
        client_print(Id, print_center, "%L", LANG_PLAYER, "YOU_DEAD");
        return PLUGIN_HANDLED;
    }

    #if CHECK_BUYZONE
    if (!IsUserInBuyZone(Id)) {
        client_print(Id, print_center, "%L", LANG_PLAYER, "OUT_OF_BUYZONE");
        return PLUGIN_HANDLED;
    }
    #endif

    new WeaponName[32];
    read_argv(1, WeaponName, charsmax(WeaponName));

    if (!TrieKeyExists(WeaponsNames, WeaponName)) {
        client_print_color(Id, print_team_default, "%L", LANG_PLAYER, "WEAPON_NOT_FOUND", WeaponName);
        return PLUGIN_CONTINUE;
    }

    new WeaponId;
    TrieGetCell(WeaponsNames, WeaponName, WeaponId);
    new Data[CWAPI_WeaponData];
    ArrayGetArray(CustomWeapons, WeaponId, Data);

    if (Data[CWAPI_WD_Price] < 0) {
        client_print_color(Id, print_team_default, "%L", LANG_PLAYER, "WEAPON_BUY_NO_PRICE", WeaponName);
        return PLUGIN_HANDLED;
    }

    if (get_member(Id, m_iAccount) < Data[CWAPI_WD_Price]) {
        client_print_color(Id, print_team_default, "%L", LANG_PLAYER, "WEAPON_BUY_NO_MONEY", WeaponName);
        return PLUGIN_HANDLED;
    }

    if (GiveCustomWeapon(Id, WeaponId, CWAPI_GT_SMART) != -1) {
        rg_add_account(Id, -Data[CWAPI_WD_Price], AS_ADD);
        client_print_color(Id, print_team_default, "%L", LANG_PLAYER, "WEAPON_BUY_SUCCESS", WeaponName, Data[CWAPI_WD_Price]);
    } else {
        client_print_color(Id, print_team_default, "%L", LANG_PLAYER, "WEAPON_BUY_ERROR");
    }

    return PLUGIN_HANDLED;
}

public Hook_WeaponBoxSetModel(const WeaponBox) {
    new ItemId = GetItemFromWeaponBox(WeaponBox);
    if (ItemId == NULLENT) {
        return;
    }

    new WeaponId = GetWeapId(ItemId);
    if (!IsCustomWeapon(WeaponId)) {
        return;
    }

    new Data[CWAPI_WeaponData];
    ArrayGetArray(CustomWeapons, WeaponId, Data);

    if (Data[CWAPI_WD_Models][CWAPI_WM_W][0]) {
        SetHookChainArg(2, ATYPE_STRING, Data[CWAPI_WD_Models][CWAPI_WM_W], PLATFORM_MAX_PATH-1);
    }

    return;
}

public Hook_WeaponBoxSetModel_Post(const WeaponBox) {
    new ItemId = GetItemFromWeaponBox(WeaponBox);
    if (ItemId == NULLENT) {
        return;
    }

    new WeaponId = GetWeapId(ItemId);
    if (!IsCustomWeapon(WeaponId)) {
        return;
    }

    CallWeaponEvent(WeaponId, CWAPI_WE_Droped, ItemId, WeaponBox);
    return;
}

public Hook_PlayerAddItem(const UserId, const ItemId) {
    new WeaponId = GetWeapId(ItemId);

    if (
        !IsCustomWeapon(WeaponId)
        || !is_user_connected(UserId)
    ) {
        return;
    }

    CallWeaponEvent(GetWeapId(ItemId), CWAPI_WE_AddItem, ItemId, UserId);
    return;
}

public Hook_PlayerTakeDamage(const Victim, Inflictor, Attacker, Float:Damage, DamageBits) {
    if (
        !is_user_connected(Victim)
        || !is_user_connected(Attacker)
    ) {
        return HC_CONTINUE;
    }
        
    new ItemId = GetAttackerWeapon(Attacker, Inflictor);

    if (is_nullent(ItemId)) {
        return HC_CONTINUE;
    }

    new WeaponId = GetWeapId(ItemId);
    if (!IsCustomWeapon(WeaponId)) {
        return HC_CONTINUE;
    }

    if (!CallWeaponEvent(WeaponId, CWAPI_WE_Damage, ItemId, Victim, Damage, DamageBits)) {
        SetHookChainReturn(ATYPE_INTEGER, 0);
        return HC_SUPERCEDE;
    }

    return HC_CONTINUE;
}

public Hook_PlayerKilled(const Victim, const Attacker, const Inflictor) {
    if (
        !is_user_connected(Victim)
        || !is_user_connected(Attacker)
    ) {
        return HC_CONTINUE;
    }
        
    new ItemId = GetAttackerWeapon(Attacker, Inflictor);

    if (is_nullent(ItemId)) {
        return HC_CONTINUE;
    }

    new WeaponId = GetWeapId(ItemId);
    if (!IsCustomWeapon(WeaponId)) {
        return HC_CONTINUE;
    }

    CallWeaponEvent(WeaponId, CWAPI_WE_Kill, ItemId, Victim);
    return HC_CONTINUE;
}

#if USE_NEW_REAPI_HOOKS
public Hook_DefaultDeploy_Pre(const ItemId, szViewModel[], szWeaponModel[], iAnim, szAnimExt[], skiplocal) {
    if (!IsCustomWeapon(GetWeapId(ItemId))) {
        return;
    }
    
    new Data[CWAPI_WeaponData];
    ArrayGetArray(CustomWeapons, GetWeapId(ItemId), Data);

    if (Data[CWAPI_WD_Models][CWAPI_WM_V][0]) {
        SetHookChainArg(2, ATYPE_STRING, Data[CWAPI_WD_Models][CWAPI_WM_V]);
    }
    
    if (Data[CWAPI_WD_Models][CWAPI_WM_P][0]) {
        SetHookChainArg(3, ATYPE_STRING, Data[CWAPI_WD_Models][CWAPI_WM_P]);
    }
    
    if (Data[CWAPI_WD_DeployTime] >= 0.0) {
        SetWeaponNextAttack(ItemId, Data[CWAPI_WD_DeployTime]);
    }

    if (Data[CWAPI_WD_Accuracy] >= 0.0) {
        set_member(ItemId, m_Weapon_flAccuracy, Data[CWAPI_WD_Accuracy]);
    }

    // CallWeaponEvent(GetWeapId(ItemId), CWAPI_WE_Deploy, ItemId);
}

public Hook_DefaultReload_Pre(const ItemId, iClipSize, iAnim, Float:fDelay) {
    new WeaponId = GetWeapId(ItemId);
    if (!IsCustomWeapon(WeaponId)) {
        return HC_CONTINUE;
    }

    new UserId = get_member(ItemId, m_pPlayer);

    if (
        get_member(ItemId, m_Weapon_iClip) >= iClipSize
        || !CallWeaponEvent(WeaponId, CWAPI_WE_Reload, ItemId)
    ) {
        SetWeaponIdleAnim(UserId, ItemId);
        SetHookChainReturn(ATYPE_INTEGER, false);

        return HC_BREAK;
    }
    
    new Data[CWAPI_WeaponData];
    ArrayGetArray(CustomWeapons, WeaponId, Data);

    if (Data[CWAPI_WD_ReloadTime] >= 0.0) {
        SetHookChainArg(4, ATYPE_FLOAT, Data[CWAPI_WD_ReloadTime]);
    }

    return HC_CONTINUE;
}

public Hook_DefaultDeploy_Post(const ItemId, szViewModel[], szWeaponModel[], iAnim, szAnimExt[], skiplocal) {
    new WeaponId = GetWeapId(ItemId);
    if (!IsCustomWeapon(WeaponId)) {
        return;
    }
    
    new Data[CWAPI_WeaponData];
    ArrayGetArray(CustomWeapons, WeaponId, Data);

    if (Data[CWAPI_WD_Accuracy] >= 0.0) {
        set_member(ItemId, m_Weapon_flAccuracy, Data[CWAPI_WD_Accuracy]);
    }

    CallWeaponEvent(WeaponId, CWAPI_WE_Deploy, ItemId);
}

public Hook_DefaultReload_Post(const ItemId, iClipSize, iAnim, Float:fDelay) {
    new WeaponId = GetWeapId(ItemId);
    if (!IsCustomWeapon(WeaponId)) {
        return HC_CONTINUE;
    }

    new Data[CWAPI_WeaponData];
    ArrayGetArray(CustomWeapons, WeaponId, Data);

    if (Data[CWAPI_WD_Accuracy] >= 0.0) {
        set_member(ItemId, m_Weapon_flAccuracy, Data[CWAPI_WD_Accuracy]);
    }

    return HC_CONTINUE;
}

public Hook_DefaultShotgunReload(const ItemId, iAnim, iStartAnim, Float:fDelay, Float:fStartDelay, const pszReloadSound1[], const pszReloadSound2[]) {
    new WeaponId = GetWeapId(ItemId);
    if (!IsCustomWeapon(WeaponId)) {
        return HC_CONTINUE;
    }

    new UserId = get_member(ItemId, m_pPlayer);

    if (
        get_member(ItemId, m_Weapon_iClip) >= rg_get_iteminfo(ItemId, ItemInfo_iMaxClip)
        || !CallWeaponEvent(WeaponId, CWAPI_WE_Reload, ItemId)
    ) {
        SetWeaponIdleAnim(UserId, ItemId);
        SetHookChainReturn(ATYPE_BOOL, false);

        return HC_BREAK;
    }
    
    new Data[CWAPI_WeaponData];
    ArrayGetArray(CustomWeapons, WeaponId, Data);

    if (Data[CWAPI_WD_ReloadTime] >= 0.0) {
        SetHookChainArg(4, ATYPE_FLOAT, Data[CWAPI_WD_ReloadTime]);
        SetHookChainArg(5, ATYPE_FLOAT, Data[CWAPI_WD_ReloadTime]);
    }
    
    return HC_CONTINUE;
}

#else

public Hook_PlayerItemDeploy(const ItemId) {
    new WeaponId = GetWeapId(ItemId);
    if (!IsCustomWeapon(WeaponId)) {
        return;
    }
    
    new Id = get_member(ItemId, m_pPlayer);

    if (!is_user_connected(Id)) {
        return;
    }
    
    new Data[CWAPI_WeaponData];
    ArrayGetArray(CustomWeapons, WeaponId, Data);

    if (Data[CWAPI_WD_Models][CWAPI_WM_V][0]) {
        set_entvar(Id, var_viewmodel, Data[CWAPI_WD_Models][CWAPI_WM_V]);
    }
    
    if (Data[CWAPI_WD_Models][CWAPI_WM_P][0]) {
        set_entvar(Id, var_weaponmodel, Data[CWAPI_WD_Models][CWAPI_WM_P]);
    }
    
    if (Data[CWAPI_WD_DeployTime] >= 0.0) {
        SetWeaponNextAttack(ItemId, Data[CWAPI_WD_DeployTime]);
    }

    if (Data[CWAPI_WD_Accuracy] >= 0.0) {
        set_member(ItemId, m_Weapon_flAccuracy, Data[CWAPI_WD_Accuracy]);
    }

    CallWeaponEvent(WeaponId, CWAPI_WE_Deploy, ItemId);
    return;
}

public Hook_PlayerItemReloaded(const ItemId) {
    new Id = get_member(ItemId, m_pPlayer);

    if (
        !is_user_connected(Id)
        || !IsCustomWeapon(GetWeapId(ItemId))
        || (
            get_member(ItemId, m_Weapon_iClip) < rg_get_iteminfo(ItemId, ItemInfo_iMaxClip)
            && CallWeaponEvent(GetWeapId(ItemId), CWAPI_WE_Reload, ItemId)
        )
    ) {
        return HAM_IGNORED;
    }

    SetWeaponIdleAnim(Id, ItemId);
    return HAM_SUPERCEDE;
}

public Hook_PlayerItemReloaded_Post(const ItemId) {
    if (!IsCustomWeapon(GetWeapId(ItemId))) {
        return HAM_IGNORED;
    }

    new Data[CWAPI_WeaponData];
    ArrayGetArray(CustomWeapons, GetWeapId(ItemId), Data);
    
    if (Data[CWAPI_WD_ReloadTime] >= 0.0) {
        if (get_member(ItemId, m_Weapon_fInSpecialReload)) {
            set_member(ItemId, m_Weapon_flNextReload, Data[CWAPI_WD_ReloadTime]);
        } else {
            SetWeaponNextAttack(ItemId, Data[CWAPI_WD_ReloadTime]);
        }
    }

    return HAM_IGNORED;
}
#endif

public Hook_PlayerItemHolster(const ItemId) {
    if (!IsCustomWeapon(GetWeapId(ItemId))) {
        return;
    }

    CallWeaponEvent(GetWeapId(ItemId), CWAPI_WE_Holster, ItemId);
    return;
}

public Hook_PlayerGetMaxSpeed(const ItemId) {
    if (!IsCustomWeapon(GetWeapId(ItemId))) {
        return HAM_IGNORED;
    }
    
    new Data[CWAPI_WeaponData];
    ArrayGetArray(CustomWeapons, GetWeapId(ItemId), Data);

    if (Data[CWAPI_WD_MaxWalkSpeed] >= 0) {
        SetHamReturnFloat(Data[CWAPI_WD_MaxWalkSpeed]);
        return HAM_SUPERCEDE;
    }

    return HAM_IGNORED;
}

public Hook_PrimaryAttack_Pre(ItemId) {
    new WeaponId = GetWeapId(ItemId);
    if (!IsCustomWeapon(WeaponId)) {
        return;
    }

    if (get_member(ItemId, m_Weapon_iClip) < 1) {
        return;
    }

    if (
        IsPistol(ItemId)
        && get_member(ItemId, m_Weapon_iShotsFired)+1 > 1
    ) {
        return;
    }

    new Data[CWAPI_WeaponData];
    ArrayGetArray(CustomWeapons, WeaponId, Data);
    
    CallWeaponEvent(WeaponId, CWAPI_WE_PrimaryAttack, ItemId);
    return;
}

public Hook_PrimaryAttack_Post(ItemId) {
    new WeaponId = GetWeapId(ItemId);

    if (!IsCustomWeapon(WeaponId)) {
        return HAM_IGNORED;
    }

    if (get_member(ItemId, m_Weapon_fFireOnEmpty)) {
        return HAM_IGNORED;
    }

    if (IsPistol(ItemId) && get_member(ItemId, m_Weapon_iShotsFired) > 1) {
        return HAM_IGNORED;
    }

    new Data[CWAPI_WeaponData];
    ArrayGetArray(CustomWeapons, WeaponId, Data);

    // CallWeaponEvent(WeaponId, CWAPI_WE_PrimaryAttack, ItemId);

    if (Data[CWAPI_WD_PrimaryAttackRate] > 0.0) {
        SetWeaponNextAttack(ItemId, Data[CWAPI_WD_PrimaryAttackRate]);
    }

    new UserId = get_member(ItemId, m_pPlayer);

    if (IsWeaponSilenced(ItemId)) if (Data[CWAPI_WD_Sounds][CWAPI_WS_ShotSilent][0]) {
        rh_emit_sound2(UserId, 0, CHAN_WEAPON, Data[CWAPI_WD_Sounds][CWAPI_WS_ShotSilent]);
    } else if (Data[CWAPI_WD_Sounds][CWAPI_WS_Shot][0]) {
        rh_emit_sound2(UserId, 0, CHAN_WEAPON, Data[CWAPI_WD_Sounds][CWAPI_WS_Shot]);
    }

    return HAM_IGNORED;
}

public Hook_SecondaryAttack(ItemId) {
    new WeaponId = GetWeapId(ItemId);
    if (!IsCustomWeapon(WeaponId)) {
        return HAM_IGNORED;
    }

    if (!CallWeaponEvent(WeaponId, CWAPI_WE_SecondaryAttack, ItemId)) {
        return HAM_IGNORED;
    }

    new Data[CWAPI_WeaponData];
    ArrayGetArray(CustomWeapons, WeaponId, Data);

    if (Data[CWAPI_WD_SecondaryAttackRate] > 0.0) {
        SetWeaponNextAttack(ItemId, Data[CWAPI_WD_SecondaryAttackRate]);
    }
    
    return HAM_IGNORED;
}

public Hook_AddItemToPlayer_Post(const ItemId, const UserId) {
    new WeaponId = GetWeapId(ItemId);
    if (!IsCustomWeapon(WeaponId)) {
        return HAM_IGNORED;
    }

    new Data[CWAPI_WeaponData];
    ArrayGetArray(CustomWeapons, WeaponId, Data);

    if (Data[CWAPI_WD_HasCustomHud]) {
        ShowWeaponListHud(UserId, ItemId, Data[CWAPI_WD_Name]);
    }

    return HAM_IGNORED;
}

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

// UTILS

GetAttackerWeapon(const AttackerId, const InflictorId) {
    if (InflictorId != AttackerId) {
        return 0;
    }
    // return FClassnameIs(InflictorId, "grenade") ? get_entvar(InflictorId, var_impulse) : 0;
    // TODO: Сделать поддержку гранат

    if (is_user_connected(AttackerId)) {
        return get_member(AttackerId, m_pActiveItem);
    }

    return 0;
}

// Получение ID итема из WeaponBox'а
GetItemFromWeaponBox(const WeaponBox) {
    for (new i = 0, ItemId; i < MAX_ITEM_TYPES; i++) {
        ItemId = get_member(WeaponBox, m_WeaponBox_rgpPlayerItems, i);
        if (!is_nullent(ItemId)) {
            return ItemId;
        }
    }
    return NULLENT;
}

#if CHECK_BUYZONE
// В зоне закупки ли игрок
bool:IsUserInBuyZone(const UserId) {
    new Signal[UnifiedSignals];
    get_member(UserId, m_signals, Signal);
    return ((SignalState:Signal[US_State] & SIGNAL_BUY) == SIGNAL_BUY);
}
#endif

// Установка времени до следующего выстрела
SetWeaponNextAttack(ItemId, Float:Rate) {
    set_member(ItemId, m_Weapon_flNextPrimaryAttack, Rate);
    set_member(ItemId, m_Weapon_flNextSecondaryAttack, Rate);
}

SetWeaponIdleAnim(const UserId, const ItemId) {
    new Anim = 0;
    if (!IsWeaponSilenced(ItemId)) {
        new WeaponIdType:WeaponId = WeaponIdType:rg_get_iteminfo(ItemId, ItemInfo_iId);
        if (WeaponId == WEAPON_M4A1) {
            Anim = 7;
        } else if (WeaponId == WEAPON_USP) {
            Anim = 8;
        }
    }

    set_entvar(UserId, var_weaponanim, Anim);

    message_begin(MSG_ONE_UNRELIABLE, SVC_WEAPONANIM, .player = UserId);
    write_byte(Anim);
    write_byte(get_entvar(UserId, var_body));
    message_end();
}

ShowWeaponListHud(const UserId, const ItemId, const WeaponName[]) {
    new WeaponId = rg_get_iteminfo(ItemId, ItemInfo_iId);
    message_begin(MSG_ONE, UserMsgs[UM_WeaponList], .player = UserId);
    write_string(GetWeapFullName(WeaponName));					// WeaponName
    write_byte(rg_get_weapon_info(WeaponId, WI_AMMO_TYPE));	    // PrimaryAmmoID
    write_byte(rg_get_iteminfo(ItemId, ItemInfo_iMaxAmmo1));	// PrimaryAmmoMaxAmount
    write_byte(-1);												// SecondaryAmmoID
    write_byte(-1);												// SecondaryAmmoMaxAmount
    write_byte(rg_get_iteminfo(ItemId, ItemInfo_iSlot));		// SlotID (0...N)
    write_byte(rg_get_iteminfo(ItemId, ItemInfo_iPosition));	// NumberInSlot (1...N)
    write_byte(WeaponId);                                       // WeaponID
    write_byte(0);												// Flags
    message_end();
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

    ExecuteForward(Fwds[F_LoadWeaponsPost]);
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

InitForwards() {
    Fwds[F_LoadWeaponsPost] = CreateMultiForward("CWAPI_LoadWeaponsPost", ET_IGNORE);
}

// EVENTS

CallWeaponEvent(const WeaponId, const CWAPI_WeaponEvents:Event, const ItemId, const any:...) {
    static WData[CWAPI_WeaponData];
    ArrayGetArray(CustomWeapons, WeaponId, WData);
    if (WData[CWAPI_WD_CustomHandlers][Event] == Invalid_Array) {
        return true;
    }
    
    static FwdId, Return, Status;
    for (new i = 0, iMax = ArraySizeSafe(WData[CWAPI_WD_CustomHandlers][Event]); i < iMax; i++) {
        FwdId = ArrayGetCell(WData[CWAPI_WD_CustomHandlers][Event], i);

        Return = CWAPI_RET_CONTINUE;
        switch (Event) {
            case CWAPI_WE_PrimaryAttack: Status = ExecuteForward(FwdId, Return, ItemId);
            case CWAPI_WE_SecondaryAttack: Status = ExecuteForward(FwdId, Return, ItemId);
            case CWAPI_WE_Reload: Status = ExecuteForward(FwdId, Return, ItemId);
            case CWAPI_WE_Deploy: Status = ExecuteForward(FwdId, Return, ItemId);
            case CWAPI_WE_Holster: Status = ExecuteForward(FwdId, Return, ItemId);
            case CWAPI_WE_Damage: Status = ExecuteForward(FwdId, Return, ItemId, _:getarg(3), Float:getarg(4), _:getarg(5));
            case CWAPI_WE_Droped: Status = ExecuteForward(FwdId, Return, ItemId, getarg(3));
            case CWAPI_WE_AddItem: Status = ExecuteForward(FwdId, Return, ItemId, getarg(3));
            case CWAPI_WE_Take: Status = ExecuteForward(FwdId, Return, ItemId, getarg(3));
            case CWAPI_WE_Kill: Status = ExecuteForward(FwdId, Return, ItemId, _:getarg(3));
            default: {
                log_error(CWAPI_ERR_UNDEFINED_EVENT, "Undefined weapon event '%d'.", _:Event);
                Status = 0;
            }
        }

        if (!Status) {
            log_error(CWAPI_ERR_CANT_EXECUTE_FWD, "Can't execute event forward.");
        }

        if (Return == CWAPI_RET_HANDLED) {
            break;
        }
    }

    if (Return == CWAPI_RET_HANDLED) {
        return false;
    }

    return true;
}

_CreateOneForward(const PluginId, const FuncName[], const CWAPI_WeaponEvents:Event) {
    switch (Event) {
        case CWAPI_WE_PrimaryAttack: return CreateOneForward(PluginId, FuncName, FP_CELL);
        case CWAPI_WE_SecondaryAttack: return CreateOneForward(PluginId, FuncName, FP_CELL);
        case CWAPI_WE_Reload: return CreateOneForward(PluginId, FuncName, FP_CELL);
        case CWAPI_WE_Deploy: return CreateOneForward(PluginId, FuncName, FP_CELL);
        case CWAPI_WE_Holster: return CreateOneForward(PluginId, FuncName, FP_CELL);
        case CWAPI_WE_Damage: return CreateOneForward(PluginId, FuncName, FP_CELL, FP_CELL, FP_FLOAT, FP_CELL);
        case CWAPI_WE_Droped: return CreateOneForward(PluginId, FuncName, FP_CELL, FP_CELL);
        case CWAPI_WE_AddItem: return CreateOneForward(PluginId, FuncName, FP_CELL, FP_CELL);
        case CWAPI_WE_Take: return CreateOneForward(PluginId, FuncName, FP_CELL, FP_CELL);
        case CWAPI_WE_Kill: return CreateOneForward(PluginId, FuncName, FP_CELL, FP_CELL);
    }

    return log_error(CWAPI_ERR_UNDEFINED_EVENT, "Undefined weapon event '%d'", _:Event);
}

ArraySizeSafe(const Array:a) {
    if (a == Invalid_Array) {
        return 0;
    }

    return ArraySize(a);
}

// NATIVES

#define NATIVE_CHECK_WEAPONS_LOADED(%1) \
    if (CustomWeapons == Invalid_Array) { \
        log_error(0, "[ERROR] Custom weapons not loaded yet."); \
        return %1; \
    }

public plugin_natives() {
    register_library(CWAPI_LIBRARY);
    
    register_native("CWAPI_RegisterHook", "Native_RegisterHook");
    register_native("CWAPI_AddCustomWeapon", "Native_AddCustomWeapon");
    
    register_native("CWAPI_GiveWeapon", "Native_GiveWeapon");
    register_native("CWAPI_GiveWeaponById", "Native_GiveWeaponById");

    register_native("CWAPI_IsCustomWeapon", "Native_IsCustomWeapon");
    register_native("CWAPI_GetWeaponId", "Native_GetWeaponId");
    register_native("CWAPI_GetWeaponData", "Native_GetWeaponData");
    register_native("CWAPI_GetWeaponsList", "Native_GetWeaponsList");
    register_native("CWAPI_FindWeapon", "Native_FindWeapon");
    register_native("CWAPI_GetAbilityWeaponsList", "Native_GetAbilityWeaponsList");

    register_native("CWAPI_GetWeaponIdFromEnt", "Native_GetWeaponIdFromEnt");
}

public Native_GiveWeapon() {
    NATIVE_CHECK_WEAPONS_LOADED(-1)

    enum {Arg_UserId = 1, Arg_WeaponName, Arg_GiveType};
    new UserId = get_param(Arg_UserId);

    new WeaponName[32];
    get_string(Arg_WeaponName, WeaponName, charsmax(WeaponName));

    new CWAPI_GiveType:Type = CWAPI_GiveType:get_param(Arg_GiveType);

    if (!TrieKeyExists(WeaponsNames, WeaponName)) {
        log_error(CWAPI_ERR_WEAPON_NOT_FOUND, "Weapon '%s' not found", WeaponName);
        return -1;
    }

    new WeaponId;
    TrieGetCell(WeaponsNames, WeaponName, WeaponId);

    return GiveCustomWeapon(UserId, WeaponId, Type);
}

public Native_GiveWeaponById() {
    NATIVE_CHECK_WEAPONS_LOADED(-1)

    enum {Arg_UserId = 1, Arg_WeaponId, Arg_GiveType};
    new UserId = get_param(Arg_UserId);
    new WeaponId = get_param(Arg_WeaponId);
    new CWAPI_GiveType:Type = CWAPI_GiveType:get_param(Arg_GiveType);

    if (!IsCustomWeapon(WeaponId)) {
        log_error(CWAPI_ERR_WEAPON_NOT_FOUND, "Weapon #%d not found", WeaponId);
        return -1;
    }

    return GiveCustomWeapon(UserId, WeaponId, Type);
}

public Native_RegisterHook(const PluginId, const Params) {
    NATIVE_CHECK_WEAPONS_LOADED(-1)

    enum {Arg_WeaponName = 1, Arg_Event, Arg_FuncName};
    new WeaponName[32];
    get_string(Arg_WeaponName, WeaponName, charsmax(WeaponName));

    new CWAPI_WeaponEvents:Event = CWAPI_WeaponEvents:get_param(Arg_Event);

    new FuncName[64];
    get_string(Arg_FuncName, FuncName, charsmax(FuncName));

    if (!TrieKeyExists(WeaponsNames, WeaponName)) {
        return -1;
    }

    new WeaponId; TrieGetCell(WeaponsNames, WeaponName, WeaponId);
    new Data[CWAPI_WeaponData]; ArrayGetArray(CustomWeapons, WeaponId, Data);

    if (Data[CWAPI_WD_CustomHandlers][Event] == Invalid_Array) {
        Data[CWAPI_WD_CustomHandlers][Event] = ArrayCreate();
        ArraySetArray(CustomWeapons, WeaponId, Data);
    }
    
    new FwdId = _CreateOneForward(PluginId, FuncName, Event);
    new HandlerId = ArrayPushCell(Data[CWAPI_WD_CustomHandlers][Event], _:FwdId);

    return HandlerId;
}

public Array:Native_GetWeaponsList() {
    NATIVE_CHECK_WEAPONS_LOADED(Invalid_Array)

    return ArrayClone(CustomWeapons);
}

public Native_GetWeaponData() {
    NATIVE_CHECK_WEAPONS_LOADED(-1)

    enum {Arg_WeaponId = 1, Arg_WeaponData};

    new WeaponId = get_param(Arg_WeaponId);

    if (!IsCustomWeapon(WeaponId)) {
        log_error(CWAPI_ERR_WEAPON_NOT_FOUND, "Weapon #%d not found", WeaponId);
        return -1;
    }

    new WeaponData[CWAPI_WeaponData];
    ArrayGetArray(CustomWeapons, WeaponId, WeaponData);

    return set_array(Arg_WeaponData, WeaponData, CWAPI_WeaponData);
}

public Native_GetWeaponId() {
    NATIVE_CHECK_WEAPONS_LOADED(-1)

    enum {Arg_WeaponName = 1};
    new WeaponName[32];
    get_string(Arg_WeaponName, WeaponName, charsmax(WeaponName));

    new WeaponId;
    if (!TrieGetCell(WeaponsNames, WeaponName, WeaponId)) {
        return -1;
    }

    return WeaponId;
}

public Native_AddCustomWeapon() {
    NATIVE_CHECK_WEAPONS_LOADED(-1)

    enum {Arg_WeaponData = 1}
    new Data[CWAPI_WeaponData]; get_array(Arg_WeaponData, Data, CWAPI_WeaponData);

    if (TrieKeyExists(WeaponsNames, Data[CWAPI_WD_Name])) {
        log_error(CWAPI_ERR_DUPLICATE_WEAPON_NAME, "Weapon '%s' already exists.", Data[CWAPI_WD_Name]);
        return -1;
    }

    for (new i = 1; i < _:CWAPI_WeaponEvents; i++) {
        if (Data[CWAPI_WD_CustomHandlers][CWAPI_WeaponEvents:i] != Invalid_Array) {
            ArrayDestroy(Data[CWAPI_WD_CustomHandlers][CWAPI_WeaponEvents:i]);
        }
    }

    new WeaponId = ArrayPushArray(CustomWeapons, Data);
    TrieSetCell(WeaponsNames, Data[CWAPI_WD_Name], WeaponId);
    return WeaponId;
}

public Native_FindWeapon() {
    NATIVE_CHECK_WEAPONS_LOADED(-1)

    enum {Arg_StartWeaponId = 1, Arg_Field, Arg_Value};
    new StartWeaponId = get_param(Arg_StartWeaponId)+1;
    new Field = get_param(Arg_Field);

    new Data[CWAPI_WeaponData];
    switch (Field) {
        case CWAPI_WD_DefaultName: {
            new Value[32];
            get_string(Arg_Value, Value, charsmax(Value));

            for (new WeaponId = StartWeaponId, iMax = ArraySizeSafe(CustomWeapons); WeaponId < iMax; WeaponId++) {
                ArrayGetArray(CustomWeapons, WeaponId, Data);
                if (equal(Data[Field], Value)) {
                    return WeaponId;
                }
            }
        }
        case CWAPI_WD_Price,
        CWAPI_WD_Weight,
        CWAPI_WD_MaxAmmo, 
        CWAPI_WD_ClipSize: {
            new Value = get_param_byref(Arg_Value);

            for (new WeaponId = StartWeaponId, iMax = ArraySizeSafe(CustomWeapons); WeaponId < iMax; WeaponId++) {
                ArrayGetArray(CustomWeapons, WeaponId, Data);
                if (Data[Field] == Value) {
                    return WeaponId;
                }
            }
        }
        case CWAPI_WD_MaxWalkSpeed,
        CWAPI_WD_DamageMult,
        CWAPI_WD_Accuracy,
        CWAPI_WD_DeployTime,
        CWAPI_WD_ReloadTime,
        CWAPI_WD_PrimaryAttackRate,
        CWAPI_WD_SecondaryAttackRate,
        CWAPI_WD_Damage: {
            new Float:Value = get_float_byref(Arg_Value);

            for (new WeaponId = StartWeaponId, iMax = ArraySizeSafe(CustomWeapons); WeaponId < iMax; WeaponId++) {
                ArrayGetArray(CustomWeapons, WeaponId, Data);
                if (Data[Field] == Value) {
                    return WeaponId;
                }
            }
        }
        case CWAPI_WD_HasSecondaryAttack: {
            new bool:Value = bool:get_param_byref(Arg_Value);
            for (new WeaponId = StartWeaponId, iMax = ArraySizeSafe(CustomWeapons); WeaponId < iMax; WeaponId++) {
                ArrayGetArray(CustomWeapons, WeaponId, Data);
                if (bool:Data[Field] == Value) {
                    return WeaponId;
                }
            }
        }
        default: {
            log_error(CWAPI_ERR_UNDEFINED_WEAPON_FIELD, "Undefined weapon field or not allowed for search.");
        }
    }
    return -1;
}

public Array:Native_GetAbilityWeaponsList() {
    NATIVE_CHECK_WEAPONS_LOADED(Invalid_Array)

    enum {Arg_AbilityName = 1};
    new AbilityName[32]; get_string(Arg_AbilityName, AbilityName, charsmax(AbilityName));
    new Array:AbilityWeaponsList; 
    if (!TrieGetCell(WeaponAbilities, AbilityName, AbilityWeaponsList)) {
        AbilityWeaponsList = ArrayCreate(CWAPI_WeaponAbilityData, 2);
        TrieSetCell(WeaponAbilities, AbilityName, AbilityWeaponsList);
    }
    return AbilityWeaponsList;
}

public Native_GetWeaponIdFromEnt() {
    enum {Arg_ItemId = 1}
    new ItemId = get_param(Arg_ItemId);
    return GetWeapId(ItemId);
}

public bool:Native_IsCustomWeapon() {
    NATIVE_CHECK_WEAPONS_LOADED(false)

    enum {Arg_WeaponId = 1}
    new WeaponId = get_param(Arg_WeaponId);
    return IsCustomWeapon(WeaponId);
}