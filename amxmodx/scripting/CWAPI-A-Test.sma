#include <amxmodx>
#include <reapi>
#include <cwapi>

new const ABILITY_NAME[] = "TestAbility";

public CWAPI_OnLoad() {
    register_plugin("[CWAPI-A] TestAbility", CWAPI_VERSION, "ArKaNeMaN");

    new T_WeaponAbility:iAbility = CWAPI_Abilities_Register(ABILITY_NAME);
    CWAPI_Abilities_AddParams(iAbility,
        "TestInteger", "Integer", false
    );

    CWAPI_Abilities_AddEventListener(iAbility, CWeapon_OnSpawn, "@OnSpawn");
    CWAPI_Abilities_AddEventListener(iAbility, CWeapon_OnSetWeaponBoxModel, "@OnSetWeaponBoxModel");
    CWAPI_Abilities_AddEventListener(iAbility, CWeapon_OnAddPlayerItem, "@OnAddPlayerItem");
    CWAPI_Abilities_AddEventListener(iAbility, CWeapon_OnDeploy, "@OnDeploy");
    CWAPI_Abilities_AddEventListener(iAbility, CWeapon_OnHolster, "@OnHolster");
    CWAPI_Abilities_AddEventListener(iAbility, CWeapon_OnRemovePlayerItem, "@OnRemovePlayerItem");
    CWAPI_Abilities_AddEventListener(iAbility, CWeapon_OnPlayerKilled, "@OnPlayerKilled");
    CWAPI_Abilities_AddEventListener(iAbility, CWeapon_OnSpawnPost, "@OnSpawnPost");
    CWAPI_Abilities_AddEventListener(iAbility, CWeapon_OnDamage, "@OnDamage");
    CWAPI_Abilities_AddEventListener(iAbility, CWeapon_OnReload, "@OnReload");
    CWAPI_Abilities_AddEventListener(iAbility, CWeapon_OnPlayerCanHaveWeapon, "@OnPlayerCanHaveWeapon");
    CWAPI_Abilities_AddEventListener(iAbility, CWeapon_OnPrimaryAttackPre, "@OnPrimaryAttackPre");
    CWAPI_Abilities_AddEventListener(iAbility, CWeapon_OnPrimaryAttackPost, "@OnPrimaryAttackPost");
    CWAPI_Abilities_AddEventListener(iAbility, CWeapon_OnPlayerTouchWeaponBox, "@OnPlayerTouchWeaponBox");
    CWAPI_Abilities_AddEventListener(iAbility, CWeapon_OnPlayerThrowGrenade, "@OnPlayerThrowGrenade");
}

@OnSpawn(const T_CustomWeapon:iWeapon, const ItemId, const Trie:tAbilityParams) {
    new iTestInteger = 0;
    TrieGetCell(tAbilityParams, "TestInteger", iTestInteger);

    PrintMessage(iWeapon, ItemId, "@OnSpawn(%d, %d, %d): %d", iWeapon, ItemId, tAbilityParams, iTestInteger);
}

@OnPlayerThrowGrenade(const T_CustomWeapon:iWeapon, const ItemId, const UserId, Float:vecSrc[3], Float:vecThrow[3], &Float:time, const usEvent, const Trie:tAbilityParams) {
    new iTestInteger = 0;
    TrieGetCell(tAbilityParams, "TestInteger", iTestInteger);

    PrintMessage(iWeapon, ItemId, "@OnPlayerThrowGrenade(%d, %d, %n, [...], [...], %.2f, %d, %d): %d", iWeapon, ItemId, UserId, time, usEvent, tAbilityParams, iTestInteger);
}

@OnSetWeaponBoxModel(const T_CustomWeapon:iWeapon, const iWeaponBox, const ItemId, const Trie:tAbilityParams) {
    new iTestInteger = 0;
    TrieGetCell(tAbilityParams, "TestInteger", iTestInteger);

    PrintMessage(iWeapon, ItemId, "@OnSetWeaponBoxModel(%d, %d, %d, %d): %d", iWeapon, iWeaponBox, ItemId, tAbilityParams, iTestInteger);
}

@OnAddPlayerItem(const T_CustomWeapon:iWeapon, const ItemId, const UserId, const Trie:tAbilityParams) {
    new iTestInteger = 0;
    TrieGetCell(tAbilityParams, "TestInteger", iTestInteger);

    PrintMessage(iWeapon, ItemId, "@OnAddPlayerItem(%d, %d, %n, %d): %d", iWeapon, ItemId, UserId, tAbilityParams, iTestInteger);
}

@OnDeploy(const T_CustomWeapon:iWeapon, const ItemId, &Float:fDeployTime, const Trie:tAbilityParams) {
    new iTestInteger = 0;
    TrieGetCell(tAbilityParams, "TestInteger", iTestInteger);

    PrintMessage(iWeapon, ItemId, "@OnDeploy(%d, %d, %.2f, %d): %d", iWeapon, ItemId, fDeployTime, tAbilityParams, iTestInteger);
}

@OnHolster(const T_CustomWeapon:iWeapon, const ItemId, const Trie:tAbilityParams) {
    new iTestInteger = 0;
    TrieGetCell(tAbilityParams, "TestInteger", iTestInteger);

    PrintMessage(iWeapon, ItemId, "@OnHolster(%d, %d, %d): %d", iWeapon, ItemId, tAbilityParams, iTestInteger);
}

@OnRemovePlayerItem(const T_CustomWeapon:iWeapon, const ItemId, const UserId, const Trie:tAbilityParams) {
    new iTestInteger = 0;
    TrieGetCell(tAbilityParams, "TestInteger", iTestInteger);

    PrintMessage(iWeapon, ItemId, "@OnRemovePlayerItem(%d, %d, %n, %d): %d", iWeapon, ItemId, UserId, tAbilityParams, iTestInteger);
}

@OnPlayerKilled(const T_CustomWeapon:iWeapon, const ItemId, const VictimId, const KillerId, const Trie:tAbilityParams) {
    new iTestInteger = 0;
    TrieGetCell(tAbilityParams, "TestInteger", iTestInteger);
    
    PrintMessage(iWeapon, ItemId, "@OnPlayerKilled(%d, %d, %n, %n, %d): %d", iWeapon, ItemId, VictimId, KillerId, tAbilityParams, iTestInteger);
}

@OnSpawnPost(const T_CustomWeapon:iWeapon, const ItemId, const Trie:tAbilityParams) {
    new iTestInteger = 0;
    TrieGetCell(tAbilityParams, "TestInteger", iTestInteger);

    PrintMessage(iWeapon, ItemId, "@OnSpawnPost(%d, %d, %d): %d", iWeapon, ItemId, tAbilityParams, iTestInteger);
}

@OnDamage(const T_CustomWeapon:iWeapon, const ItemId, const VictimId, const InflictorId, const AttackerId, &Float:fDamage, &iDamageBits, const Trie:tAbilityParams) {
    new iTestInteger = 0;
    TrieGetCell(tAbilityParams, "TestInteger", iTestInteger);

    PrintMessage(iWeapon, ItemId, "@OnDamage(%d, %d, %d, %d, %d, %.2f, %d, %d): %d", iWeapon, ItemId, VictimId, InflictorId, AttackerId, fDamage, iDamageBits, tAbilityParams, iTestInteger);
}

@OnReload(const T_CustomWeapon:iWeapon, const ItemId, &iClipSize, &iAnim, &Float:fDelay, const Trie:tAbilityParams) {
    new iTestInteger = 0;
    TrieGetCell(tAbilityParams, "TestInteger", iTestInteger);

    PrintMessage(iWeapon, ItemId, "@OnReload(%d, %d, %d, %d, %.2f, %d): %d", iWeapon, ItemId, iClipSize, iAnim, fDelay, tAbilityParams, iTestInteger);
}

@OnPlayerCanHaveWeapon(const T_CustomWeapon:iWeapon, const ItemId, const UserId, const Trie:tAbilityParams) {
    new iTestInteger = 0;
    TrieGetCell(tAbilityParams, "TestInteger", iTestInteger);

    PrintMessage(iWeapon, ItemId, "@OnPlayerCanHaveWeapon(%d, %d, %n, %d): %d", iWeapon, ItemId, UserId, tAbilityParams, iTestInteger);
}

@OnPrimaryAttackPre(const T_CustomWeapon:iWeapon, const ItemId, const Trie:tAbilityParams) {
    new iTestInteger = 0;
    TrieGetCell(tAbilityParams, "TestInteger", iTestInteger);

    PrintMessage(iWeapon, ItemId, "@OnPrimaryAttackPre(%d, %d, %d): %d", iWeapon, ItemId, tAbilityParams, iTestInteger);
}

@OnPrimaryAttackPost(const T_CustomWeapon:iWeapon, const ItemId, const Trie:tAbilityParams) {
    new iTestInteger = 0;
    TrieGetCell(tAbilityParams, "TestInteger", iTestInteger);

    PrintMessage(iWeapon, ItemId, "@OnPrimaryAttackPost(%d, %d, %d): %d", iWeapon, ItemId, tAbilityParams, iTestInteger);
}

@OnPlayerTouchWeaponBox(const T_CustomWeapon:iWeapon, const iWeaponBox, const ItemId, const UserId, const Trie:tAbilityParams) {
    new iTestInteger = 0;
    TrieGetCell(tAbilityParams, "TestInteger", iTestInteger);
    
    PrintMessage(iWeapon, ItemId, "@OnPlayerTouchWeaponBox(%d, %d, %d, %n, %d): %d", iWeapon, iWeaponBox, ItemId, UserId, tAbilityParams, iTestInteger);
}


PrintMessage(const T_CustomWeapon:iWeapon, const ItemId, const sMsg[], const any:...) {
    new UserId = get_member(ItemId, m_pPlayer);

    new sFmtMsg[256];
    vformat(sFmtMsg, charsmax(sFmtMsg), sMsg, 4);

    new sWeaponName[CWAPI_WEAPON_NAME_MAX_LEN];
    CWAPI_Weapons_GetAttribute(iWeapon, CWeaponAttr_Name, sWeaponName, charsmax(sWeaponName));

    if (is_user_connected(UserId)) {
        client_print(UserId, print_chat, "[TEST] [%s] %s", sWeaponName, sFmtMsg);
        client_print(UserId, print_console, "[TEST] [%s] %s", sWeaponName, sFmtMsg);
        server_print("[TEST] [%n, %s] %s", UserId, sWeaponName, sFmtMsg);
    } else {
        server_print("[TEST] [*undefined*, %s] %s", sWeaponName, sFmtMsg);
    }
}
