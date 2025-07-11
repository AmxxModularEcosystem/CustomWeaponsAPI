#include <amxmodx>
#include <reapi>
#include <hamsandwich>
#include <fakemeta>
#include <cwapi>
#include "Cwapi/Utils"

#include "Cwapi/Core/CWeapons/NotClientWeapons"

enum _E_Ham_WeaponHook{
    Ham:Ham_WH_Hook,
    Ham_WH_Func[64],
    bool:Ham_WH_Post,
}

static const _HAM_WEAPON_HOOKS[][_E_Ham_WeaponHook] = {
    {Ham_Spawn, "Hook_Spawn_Post", true},
    {Ham_Item_Holster, "Hook_ItemHolster_Pre", false},
    {Ham_Weapon_PrimaryAttack, "Hook_PrimaryAttack_Pre", false},
    {Ham_Weapon_PrimaryAttack, "Hook_PrimaryAttack_Post", true},
    {Ham_CS_Weapon_SendWeaponAnim, "Hook_SendWeaponAnim_Post", true},
    {Ham_CS_Item_GetMaxSpeed, "@Ham_CS_Item_GetMaxSpeed_Pre", false},
    // {Ham_Weapon_SecondaryAttack, "Hook_SecondaryAttack", true},
    // {Ham_Item_AddToPlayer, "Hook_AddItemToPlayer_Post", true},
};

static Trie:gRegisteredRefWeapons = Invalid_Trie;

static Trie:ItemOwners = Invalid_Trie;

CWeapons_Hooks_Init() {
    CWeapons_Hooks__RegReapi();
    RegisterHam(Ham_Touch, "weaponbox", "Hook_WeaponBoxTouch", false);
    register_forward(FM_UpdateClientData, "FMHook_UpdateClientData_Post", true);

    gRegisteredRefWeapons = TrieCreate();
    ItemOwners = TrieCreate();
}

CWeapons_Hooks_RegForWeapon(const sReference[], const sWeaponName[]) {
    // Больше обёрток богу обёрток!!!
    CWeapons_Hooks__RegHamOnce(sReference);
    register_clcmd(sWeaponName, "Hook_WeaponCommand");
}

static CWeapons_Hooks__RegReapi() {
    RegisterHookChain(RG_CSGameRules_PlayerKilled, "Hook_PlayerKilled_Post", true);

    RegisterHookChain(RG_CWeaponBox_SetModel, "Hook_WeaponBoxSetModel_Pre", false);
    RegisterHookChain(RG_CBasePlayer_AddPlayerItem, "Hook_AddPlayerItem_Pre", false);
    RegisterHookChain(RG_CBasePlayer_RemovePlayerItem, "Hook_RemovePlayerItem_Post", true);
    RegisterHookChain(RG_CBasePlayerWeapon_DefaultDeploy, "Hook_DefaultDeploy_Pre", false);
    RegisterHookChain(RG_CBasePlayerWeapon_DefaultDeploy, "Hook_DefaultDeploy_Post", true);
    RegisterHookChain(RG_CBasePlayer_TakeDamage, "Hook_PlayerTakeDamage_Post", false);
    RegisterHookChain(RG_CBasePlayerWeapon_DefaultReload, "Hook_DefaultReload_Pre", false);
    RegisterHookChain(RG_CSGameRules_CanHavePlayerItem, "Hook_CanHavePlayerItem_Pre", false);
    RegisterHookChain(RG_CBasePlayer_ThrowGrenade, "Hook_PlayerThrowGrenade_Pre", .post = false);
    RegisterHookChain(RG_CBasePlayer_ThrowGrenade, "Hook_PlayerThrowGrenade_Post", .post = true);
    RegisterHookChain(RG_CBasePlayer_Radio, "Hook_PlayerRadio_Pre", .post = false);
    // RegisterHookChain(RG_CBasePlayerWeapon_DefaultReload, "Hook_DefaultReload_Post", true);
    // RegisterHookChain(RG_CBasePlayerWeapon_DefaultShotgunReload, "Hook_DefaultShotgunReload", false);
    RegisterHookChain(RG_CBasePlayer_RemoveAllItems, "Hook_RemoveAllItems_Pre", false);
}

static CWeapons_Hooks__RegHamOnce(const sWeaponName[]) {
    if (!TrieKeyExists(gRegisteredRefWeapons, sWeaponName)) {
        CWeapons_Hooks__RegHam(sWeaponName);

        TrieSetCell(gRegisteredRefWeapons, sWeaponName, true);
    }
}

static CWeapons_Hooks__RegHam(const sWeaponName[]) {
    for (new i = 0; i < sizeof _HAM_WEAPON_HOOKS; i++) {
        RegisterHam(
            _HAM_WEAPON_HOOKS[i][Ham_WH_Hook],
            sWeaponName,
            _HAM_WEAPON_HOOKS[i][Ham_WH_Func],
            _HAM_WEAPON_HOOKS[i][Ham_WH_Post]
        );
    }
}

// Callbacks

public Hook_RemoveAllItems_Pre(const playerIndex) {
    new Array:forRemove = ArrayCreate(11, 1);

    new TrieIter:iter = TrieIterCreate(ItemOwners);
    while (!TrieIterEnded(iter)) {
        new owner;
        TrieIterGetCell(iter, owner);
        if (owner == playerIndex) {
            new key[11];
            TrieIterGetKey(iter, key, charsmax(key));
            ArrayPushString(forRemove, key);
        }
        TrieIterNext(iter);
    }
    TrieIterDestroy(iter);

    for (new i = 0, ii = ArraySize(forRemove); i < ii; i++) {
        new key[11];
        ArrayGetString(forRemove, i, key, charsmax(key));
        TrieDeleteKey(ItemOwners, key);
    }
    ArrayDestroy(forRemove);

    new item = get_member(playerIndex, m_rgpPlayerItems);
    while (!is_nullent(item)) {
        Hook_RemovePlayerItem_Post(playerIndex, item);
        item = get_member(item, m_pNext);
    }

    return HC_CONTINUE;
}

public Hook_PlayerRadio_Pre(const playerIndex, msgId[], msgVerbose[]) {
    if(msgVerbose[0] == EOS || !equal(msgVerbose, "#Fire_in_the_hole")) {
        return HC_CONTINUE;
    }

    new itemIndex = get_member(playerIndex, m_pActiveItem);
    if(is_nullent(itemIndex)) {
        return HC_CONTINUE;
    }

    new weaponObject[S_CustomWeapon];
    if (!CWeapons_GetByItem(itemIndex, weaponObject)) {
        return HC_CONTINUE;
    }
    
    if (weaponObject[CWeapon_Grenade_PreventFITH]) {
        return HC_BREAK;
    }
    
    return HC_CONTINUE;
}

public Hook_PlayerThrowGrenade_Pre(const playerIndex, const grenadeItemIndex, Float:vecSrc[3], Float:vecThrow[3], Float:time, const usEvent) {
    new Weapon[S_CustomWeapon], T_CustomWeapon:iWeapon;
    if (!CWeapons_GetByItem(grenadeItemIndex, Weapon, iWeapon)) {
        return HC_CONTINUE;
    }
    
    Dbg_Log("Hook_PlayerThrowGrenade_Pre(%n, %d)", playerIndex, grenadeItemIndex);

    CWeapons_Call(Weapon, CWeapon_OnPlayerThrowGrenade, [iWeapon, grenadeItemIndex, playerIndex, PrepareArray(_:vecSrc, 3, true), PrepareArray(_:vecThrow, 3, true), time, usEvent]);

    if (Events_IsRet(CWAPI_STOP)) {
        return HC_CONTINUE;
    }

    if (Events_IsRet(CWAPI_STOP_MAIN)) {
        SetHookChainReturn(ATYPE_INTEGER, 0);
        return HC_BREAK;
    }

    // SetHookChainArg(3, ATYPE_VECTOR, vecSrc);
    // SetHookChainArg(4, ATYPE_VECTOR, vecThrow);
    SetHookChainArg(5, ATYPE_FLOAT, time);
    
    return HC_CONTINUE;
}

public Hook_PlayerThrowGrenade_Post(const playerIndex, const grenadeItemIndex, Float:vecSrc[3], Float:vecThrow[3], Float:time, const usEvent) {
    new weaponObject[S_CustomWeapon], T_CustomWeapon:weaponIndex;
    if (!CWeapons_GetByItem(grenadeItemIndex, weaponObject, weaponIndex)) {
        return HC_CONTINUE;
    }

    new grenadeEntityIndex = GetHookChainReturn(ATYPE_INTEGER);
    set_entvar(grenadeEntityIndex, var_impulse, CWeapons_GetImpulse(weaponIndex));
    set_entvar(grenadeEntityIndex, var_model, weaponObject[CWeapon_Grenade_Model]);
    set_entvar(grenadeEntityIndex, var_modelindex, weaponObject[CWeapon_Grenade_ModelIndex]);

    Dbg_Log("Hook_PlayerThrowGrenade_Post(%n, %d): grenadeEntityIndex = %d.", playerIndex, grenadeItemIndex, grenadeEntityIndex);
    Dbg_Log("Hook_PlayerThrowGrenade_Post(%n, %d): Set impulse = %d (%d).", playerIndex, grenadeItemIndex, CWeapons_GetImpulse(weaponIndex), weaponIndex);
    
    return HC_CONTINUE;
}

public Hook_PlayerKilled_Post(const VictimId, const KillerId, const InflictorId) {
    if (KillerId != InflictorId) {
        return;
    }

    if (!IsUserIdValid(KillerId)) {
        return;
    }

    new ItemId = get_member(KillerId, m_pActiveItem);

    new Weapon[S_CustomWeapon], T_CustomWeapon:iWeapon;
    if (!CWeapons_GetByItem(ItemId, Weapon, iWeapon)) {
        return;
    }
    
    CWeapons_Call(Weapon, CWeapon_OnPlayerKilled, [iWeapon, ItemId, VictimId, KillerId]);
}

public Hook_WeaponCommand(const UserId) {
    if (!is_user_alive(UserId)) {
        return PLUGIN_HANDLED;
    }

    new sWeaponName[CWAPI_WEAPON_NAME_MAX_LEN];
    read_argv(0, sWeaponName, charsmax(sWeaponName));
    
    Dbg_Log("Hook_WeaponCommand(%n): Exec cmd.", UserId);
    
    new Weapon[S_CustomWeapon];
    if (!CWeapons_GetByName(sWeaponName, Weapon)) {
        return PLUGIN_CONTINUE;
    }
    
    Dbg_Log("Hook_WeaponCommand(%n): For weapon '%s'.", UserId, Weapon[CWeapon_Name]);

    engclient_cmd(UserId, Weapon[CWeapon_Reference]);
    return PLUGIN_HANDLED_MAIN;
}

public Hook_Spawn_Post(const ItemId) {
    new Weapon[S_CustomWeapon], T_CustomWeapon:iWeapon;
    if (!CWeapons_GetByItem(ItemId, Weapon, iWeapon)) {
        return;
    }

    Dbg_Log("Hook_Spawn_Post(%d): For weapon '%s'.", ItemId, Weapon[CWeapon_Name]);

    CWeapons_Call(Weapon, CWeapon_OnSpawn, [iWeapon, ItemId]);
    
    Dbg_Log("Hook_Spawn_Post(%d): Event return - %d", ItemId, Events_GetReturnedValue());

    if (Events_IsRet(CWAPI_STOP_MAIN)) {
        RemoveEntity(ItemId);
        return;
    }

    if (Events_IsRet(CWAPI_STOP)) {
        return;
    }

    // set_member_if_specified(ItemId, m_Weapon_bHasSecondaryAttack, Weapon[CWeapon_HasSecondaryAttack], false);
    rg_set_iteminfo_if_specified(ItemId, ItemInfo_iWeight, Weapon[CWeapon_Weight], -1);
    
    new WeaponIdType:WeaponId = WeaponIdType:rg_get_iteminfo(ItemId, ItemInfo_iId);
    if (WeaponId != WEAPON_KNIFE) {
        rg_set_iteminfo_if_specified(ItemId, ItemInfo_iMaxClip, Weapon[CWeapon_MaxClip], -1);
        rg_set_iteminfo_if_specified(ItemId, ItemInfo_iMaxAmmo1, Weapon[CWeapon_MaxAmmo], -1);

        InstantReload(ItemId);
    }

    MultItemDamage(ItemId, Weapon[CWeapon_DamageMult]);
    
    set_entvar(ItemId, var_CWAPI_ItemOwner, 0);

    CWeapons_Call(Weapon, CWeapon_OnSpawnPost, [iWeapon, ItemId]);
}

public Hook_AddPlayerItem_Pre(const UserId, const ItemId) {
    if (!is_user_connected(UserId)) {
        return HC_CONTINUE;
    }

    new Weapon[S_CustomWeapon], T_CustomWeapon:iWeapon;
    if (!CWeapons_GetByItem(ItemId, Weapon, iWeapon)) {
        return HC_CONTINUE;
    }

    if (get_entvar(ItemId, var_CWAPI_ItemOwner) == 0) {
        set_entvar(ItemId, var_CWAPI_ItemOwner, UserId);
    }

    CWeapons_Call(Weapon, CWeapon_OnAddPlayerItem, [iWeapon, ItemId, UserId]);

    new itemIdStr[11];
    num_to_str(ItemId, itemIdStr, charsmax(itemIdStr));
    TrieSetCell(ItemOwners, itemIdStr, UserId);

    return HC_CONTINUE;
}

public Hook_RemovePlayerItem_Post(const UserId, const ItemId) {
    if (!is_user_connected(UserId)) {
        return HC_CONTINUE;

    }
    new Weapon[S_CustomWeapon], T_CustomWeapon:iWeapon;
    if (!CWeapons_GetByItem(ItemId, Weapon, iWeapon)) {
        return HC_CONTINUE;
    }

    new itemIdStr[11];
    num_to_str(ItemId, itemIdStr, charsmax(itemIdStr));
    new owner = 0;
    TrieGetCell(ItemOwners, itemIdStr, owner);
    TrieDeleteKey(ItemOwners, itemIdStr);
    if (owner != UserId) {
        return HC_CONTINUE;
    }

    CWeapons_Call(Weapon, CWeapon_OnRemovePlayerItem, [iWeapon, ItemId, UserId]);

    return HC_CONTINUE;
}

public Hook_WeaponBoxSetModel_Pre(const iWeaponBox, const szModelName[]) {
    new ItemId = GetItemFromWeaponBox(iWeaponBox);

    new Weapon[S_CustomWeapon], T_CustomWeapon:iWeapon;
    if (!CWeapons_GetByItem(ItemId, Weapon, iWeapon)) {
        return HC_CONTINUE;
    }

    CWeapons_Call(Weapon, CWeapon_OnSetWeaponBoxModel, [iWeapon, iWeaponBox, ItemId]);

    if (Events_IsRet(CWAPI_STOP_MAIN)) {
        RemoveWeaponBox(iWeaponBox);
        return HC_BREAK;
    }

    if (
        Events_IsRet(CWAPI_CONTINUE)
        && Weapon[CWeapon_Models][CWeapon_Model_World][0]
    ) {
        SetHookChainArg(2, ATYPE_STRING, Weapon[CWeapon_Models][CWeapon_Model_World]);
    }

    return HC_CONTINUE;
}

public Hook_DefaultDeploy_Pre(const ItemId, szViewModel[], szWeaponModel[]) {
    new Weapon[S_CustomWeapon], T_CustomWeapon:iWeapon;
    if (!CWeapons_GetByItem(ItemId, Weapon, iWeapon)) {
        return HC_CONTINUE;
    }

    Dbg_Log("Hook_DefaultDeploy_Pre(%d): For weapon '%s'.", ItemId, Weapon[CWeapon_Name]);

    if (Weapon[CWeapon_Models][CWeapon_Model_View][0]) {
        SetHookChainArg(2, ATYPE_STRING, Weapon[CWeapon_Models][CWeapon_Model_View]);
        Dbg_Log("Hook_DefaultDeploy_Pre(%d): Set '%s' as view model.", ItemId, Weapon[CWeapon_Models][CWeapon_Model_View]);
    }
    
    if (Weapon[CWeapon_Models][CWeapon_Model_Player][0]) {
        SetHookChainArg(3, ATYPE_STRING, Weapon[CWeapon_Models][CWeapon_Model_Player]);
        Dbg_Log("Hook_DefaultDeploy_Pre(%d): Set '%s' as player model.", ItemId, Weapon[CWeapon_Models][CWeapon_Model_Player]);
    }
    
    return HC_CONTINUE;
}

public Hook_DefaultDeploy_Post(const ItemId) {
    new Weapon[S_CustomWeapon], T_CustomWeapon:iWeapon;
    if (!CWeapons_GetByItem(ItemId, Weapon, iWeapon)) {
        return HC_CONTINUE;
    }

    Dbg_Log("Hook_DefaultDeploy_Post(%d): For weapon '%s'.", ItemId, Weapon[CWeapon_Name]);

    new Float:fDeployTime = get_member(get_member(ItemId, m_pPlayer), m_flNextAttack);
    if (Weapon[CWeapon_DeployTime] >= 0.0) {
        fDeployTime = Weapon[CWeapon_DeployTime];
    }

    CWeapons_Call(Weapon, CWeapon_OnDeploy, [iWeapon, ItemId, fDeployTime]);
    
    SetWeaponNextAttack(ItemId, fDeployTime);
    Dbg_Log("Hook_DefaultDeploy_Post(%d): Set next attack after %.2f seconds.", ItemId, fDeployTime);
    
    return HC_CONTINUE;
}

public Hook_ItemHolster_Pre(const ItemId) {
    new Weapon[S_CustomWeapon], T_CustomWeapon:iWeapon;
    if (!CWeapons_GetByItem(ItemId, Weapon, iWeapon)) {
        return HAM_IGNORED;
    }
    
    Dbg_Log("Hook_ItemHolster_Pre(%d) - Weapon[CWeapon_Name] = %s", ItemId, Weapon[CWeapon_Name]);

    CWeapons_Call(Weapon, CWeapon_OnHolster, [iWeapon, ItemId]);
    
    return HAM_IGNORED;
}

public Hook_PlayerTakeDamage_Post(const VictimId, InflictorId, AttackerId, Float:fDamage, iDamageBits) {
    if (
        !is_user_connected(VictimId)
        || !is_user_connected(AttackerId)
    ) {
        return HC_CONTINUE;
    }

    Dbg_Log("Hook_PlayerTakeDamage_Post(%d, %d, %d, %.2f, %d)", VictimId, InflictorId, AttackerId, fDamage, iDamageBits);

    new itemIndex = 0;
    if (FClassnameIs(InflictorId, "player")) {
        itemIndex = get_member(InflictorId, m_pActiveItem);
    }
    Dbg_Log(" itemIndex = %d", itemIndex);

    /**
     * Для ножа почему-то вызывается хук дамага для тиммейтов,
     * хотя по факту урон не наносится.
     */
    if (
        (is_nullent(itemIndex) || IsKnife(itemIndex))
        && !rg_is_player_can_takedamage(VictimId, AttackerId)
    ) {
        Dbg_Log(" knife, cant dmg");
        return HC_CONTINUE;
    }

    new weaponObject[S_CustomWeapon], T_CustomWeapon:weapon;
    if (!CWeapons_GetByInflictor(InflictorId, weaponObject, weapon)) {
        Dbg_Log(" inflictor weapon not found");
        return HC_CONTINUE;
    }
    Dbg_Log(" weaponObject = %d", weaponObject);

    if (InflictorId != AttackerId) {
        fDamage *= weaponObject[CWeapon_DamageMult];
    }

    CWeapons_Call(weaponObject, CWeapon_OnDamage, [weapon, itemIndex, VictimId, InflictorId, AttackerId, fDamage, iDamageBits]);
    if (Events_IsRet(CWAPI_STOP_MAIN)) {
        SetHookChainReturn(ATYPE_INTEGER, 0);
        return HC_BREAK;
    }

    SetHookChainArg(4, ATYPE_FLOAT, fDamage);
    SetHookChainArg(5, ATYPE_INTEGER, iDamageBits);

    return HC_CONTINUE;
}

public Hook_DefaultReload_Pre(const ItemId, iClipSize, iAnim, Float:fDelay) {
    new Weapon[S_CustomWeapon], T_CustomWeapon:iWeapon;
    if (!CWeapons_GetByItem(ItemId, Weapon, iWeapon)) {
        return HC_CONTINUE;
    }

    if (Weapon[CWeapon_ReloadTime] >= 0.0) {
        fDelay = Weapon[CWeapon_ReloadTime];
    }

    CWeapons_Call(Weapon, CWeapon_OnReload, [iWeapon, ItemId, iClipSize, iAnim, fDelay]);
    SetHookChainArg(2, ATYPE_INTEGER, iClipSize);
    SetHookChainArg(3, ATYPE_INTEGER, iAnim);
    SetHookChainArg(4, ATYPE_FLOAT, fDelay);

    if (
        Events_IsRet(CWAPI_STOP_MAIN)
        || get_member(ItemId, m_Weapon_iClip) >= iClipSize
    ) {
        SetWeaponIdleAnim(get_member(ItemId, m_pPlayer), ItemId);

        SetHookChainReturn(ATYPE_INTEGER, false);
        return HC_BREAK;
    }

    return HC_CONTINUE;
}

public Hook_CanHavePlayerItem_Pre(const UserId, const ItemId) {
    new Weapon[S_CustomWeapon], T_CustomWeapon:iWeapon;
    if (!CWeapons_GetByItem(ItemId, Weapon, iWeapon)) {
        return HC_CONTINUE;
    }

    CWeapons_Call(Weapon, CWeapon_OnPlayerCanHaveWeapon, [iWeapon, ItemId, UserId]);

    if (Events_IsRet(CWAPI_STOP_MAIN)) {
        SetHookChainReturn(ATYPE_INTEGER, 0);
        return HC_BREAK;
    }
    
    return HC_CONTINUE;
}

public Hook_PrimaryAttack_Pre(const ItemId) {
    new Weapon[S_CustomWeapon], T_CustomWeapon:iWeapon;
    if (!CWeapons_GetByItem(ItemId, Weapon, iWeapon)) {
        return HAM_IGNORED;
    }

    if (Weapon[CWeapon_HasCustomSounds]) {
        NotClientWeapons_ManageAttackHooks(true);
    }

    if (!WeaponCanAttack(ItemId)) {
        return HAM_IGNORED;
    }
    
    // TODO: Сделать что-то с гранатами, чтоб оно не вызывалось постоянно в замахе

    CWeapons_Call(Weapon, CWeapon_OnPrimaryAttackPre, [iWeapon, ItemId]);

    if (Events_IsRet(CWAPI_STOP_MAIN)) {
        return HAM_SUPERCEDE;
    }

    new UserId = get_member(ItemId, m_pPlayer);
    if (Weapon[CWeapon_HasCustomSounds]) {
        NotClientWeapons_SendDefaultWeaponAnim(UserId, ItemId, ANIMSTYPE_ATTACK);
    }

    // TODO: Сначала нужен список деф звуков
    // if (Events_IsRet(CWAPI_STOP)) {
    //     return HAM_IGNORED;
    // }
    
    if (Weapon[CWeapon_HasCustomSounds]) {
        new sShotSound[PLATFORM_MAX_PATH];
        if (IsWeaponSilenced(ItemId)) {
            copy(sShotSound, charsmax(sShotSound), Weapon[CWeapon_Sounds][CWeapon_Sound_ShotSilenced]);
        } else if (IsWeaponBurst(ItemId)) {
            copy(sShotSound, charsmax(sShotSound), Weapon[CWeapon_Sounds][CWeapon_Sound_ShotBurst]);
        } else {
            copy(sShotSound, charsmax(sShotSound), Weapon[CWeapon_Sounds][CWeapon_Sound_Shot]);
        }

        if (sShotSound[0] != EOS) {
            rh_emit_sound2(UserId, 0, CHAN_WEAPON, sShotSound);
        }
    }

    return HAM_IGNORED;
}

public Hook_PrimaryAttack_Post(const ItemId) {
    new Weapon[S_CustomWeapon], T_CustomWeapon:iWeapon;
    if (!CWeapons_GetByItem(ItemId, Weapon, iWeapon)) {
        return HAM_IGNORED;
    }

    NotClientWeapons_ManageAttackHooks(false);

    if (!WeaponWasAttack(ItemId)) {
        return HAM_IGNORED;
    }

    CWeapons_Call(Weapon, CWeapon_OnPrimaryAttackPost, [iWeapon, ItemId]);

    // if (Data[CWAPI_WD_PrimaryAttackRate] > 0.0) {
    //     SetWeaponNextAttack(ItemId, Data[CWAPI_WD_PrimaryAttackRate]);
    // }

    return HAM_IGNORED;
}

public Hook_WeaponBoxTouch(const iWeaponBox, const iToucher) {
    if (!IsUserIdValid(iToucher)) {
        return HAM_IGNORED;
    }

    new ItemId = GetItemFromWeaponBox(iWeaponBox);

    new Weapon[S_CustomWeapon], T_CustomWeapon:iWeapon;
    if (!CWeapons_GetByItem(ItemId, Weapon, iWeapon)) {
        return HAM_IGNORED;
    }

    CWeapons_Call(Weapon, CWeapon_OnPlayerTouchWeaponBox, [iWeapon, iWeaponBox, ItemId, iToucher]);
    if (Events_IsRet(CWAPI_STOP_MAIN)) {
        return HAM_SUPERCEDE;
    }

    CWeapons_Call(Weapon, CWeapon_OnPlayerCanHaveWeapon, [iWeapon, ItemId, iToucher]);
    if (Events_IsRet(CWAPI_STOP_MAIN)) {
        return HAM_SUPERCEDE;
    }

    return HAM_IGNORED;
}

public Hook_SendWeaponAnim_Post(ItemId, anim, skiplocal) {
    new Weapon[S_CustomWeapon], T_CustomWeapon:iWeapon;
    if (!CWeapons_GetByItem(ItemId, Weapon, iWeapon)) {
        return HAM_IGNORED;
    }

    if (Weapon[CWeapon_HasCustomSounds]) {
        set_entvar(ItemId, var_CWAPI_DefaultDeployAnimIndex, anim);
        NotClientWeapons_SendWeaponAnim(get_member(ItemId, m_pPlayer), anim, 0);
    }

    return HAM_IGNORED;
}

public FMHook_UpdateClientData_Post(iPlayer, SendWeapons, CD_Handle) {
    enum {
        SPEC_MODE,
        SPEC_TARGET,

        SPEC_END,
    };
    static aSpecInfo[MAX_PLAYERS + 1][SPEC_END];

    static Float:flGameTime;
    static Float:flLastEventCheck;

    new iSpecMode = get_entvar(iPlayer, var_iuser1);
    new iTarget = iSpecMode ? get_entvar(iPlayer, var_iuser2) : iPlayer;

    if (!is_user_alive(iTarget)) {
        return FMRES_IGNORED;
    }

    new iActiveItem = get_member(iTarget, m_pActiveItem);
    if (iActiveItem == NULLENT) {
        return FMRES_IGNORED;
    }
    
    new Weapon[S_CustomWeapon], T_CustomWeapon:iWeapon;
    if (!CWeapons_GetByItem(iActiveItem, Weapon, iWeapon)) {
        return FMRES_IGNORED;
    }

    if (!Weapon[CWeapon_HasCustomSounds]) {
        return FMRES_IGNORED;
    }

    flGameTime = get_gametime();
    flLastEventCheck = get_member(iActiveItem, m_flLastEventCheck);

    if (iSpecMode) {
        if (aSpecInfo[iPlayer][SPEC_MODE] != iSpecMode) {
            aSpecInfo[iPlayer][SPEC_MODE] = iSpecMode;
            aSpecInfo[iPlayer][SPEC_TARGET] = 0;
        }

        if (iSpecMode == OBS_IN_EYE && aSpecInfo[iPlayer][SPEC_TARGET] != iTarget) {
            aSpecInfo[iPlayer][SPEC_TARGET] = iTarget;
            NotClientWeapons_SendDefaultWeaponAnim(iTarget, iActiveItem, ANIMSTYPE_IDLE);
        }
    }

    if (!flLastEventCheck) {
        set_cd(CD_Handle, CD_flNextAttack, flGameTime + 0.001);
    } else if (flLastEventCheck <= flGameTime) {
        set_member(iActiveItem, m_flLastEventCheck, 0.0);
    }

    return FMRES_IGNORED;
}

@Ham_CS_Item_GetMaxSpeed_Pre(const item) {
    new weaponObject[S_CustomWeapon], T_CustomWeapon:weapon;
    if (!CWeapons_GetByItem(item, weaponObject, weapon)) {
        return HAM_IGNORED;
    }

    if (weaponObject[CWeapon_MaxWalkSpeed] < 0) {
        return HAM_IGNORED;
    }

    SetHamReturnFloat(weaponObject[CWeapon_MaxWalkSpeed]);
    return HAM_SUPERCEDE;
}

// public Hook_DefaultShotgunReload(const ItemId, iAnim, iStartAnim, Float:fDelay, Float:fStartDelay, const pszReloadSound1[], const pszReloadSound2[]) {
//     new WeaponId = GetWeapId(ItemId);
//     if (!IsCustomWeapon(WeaponId)) {
//         return HC_CONTINUE;
//     }

//     new UserId = get_member(ItemId, m_pPlayer);

//     if (
//         get_member(ItemId, m_Weapon_iClip) >= rg_get_iteminfo(ItemId, ItemInfo_iMaxClip)
//         || !CallWeaponEvent(WeaponId, CWAPI_WE_Reload, ItemId)
//     ) {
//         SetWeaponIdleAnim(UserId, ItemId);
//         SetHookChainReturn(ATYPE_BOOL, false);

//         return HC_BREAK;
//     }
    
//     new Data[CWAPI_WeaponData];
//     ArrayGetArray(CustomWeapons, WeaponId, Data);

//     if (Data[CWAPI_WD_ReloadTime] >= 0.0) {
//         SetHookChainArg(4, ATYPE_FLOAT, Data[CWAPI_WD_ReloadTime]);
//         SetHookChainArg(5, ATYPE_FLOAT, Data[CWAPI_WD_ReloadTime]);
//     }
    
//     return HC_CONTINUE;
// }

// public Hook_AddItemToPlayer_Post(const ItemId, const UserId) {
//     new WeaponId = GetWeapId(ItemId);
//     if (!IsCustomWeapon(WeaponId)) {
//         return HAM_IGNORED;
//     }

//     new Data[CWAPI_WeaponData];
//     ArrayGetArray(CustomWeapons, WeaponId, Data);

//     if (Data[CWAPI_WD_HasCustomHud]) {
//         ShowWeaponListHud(UserId, ItemId);
//     }

//     return HAM_IGNORED;
// }
