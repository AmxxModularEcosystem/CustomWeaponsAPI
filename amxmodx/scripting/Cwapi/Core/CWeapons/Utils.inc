#if defined __cwapi_CustomWeapons_Utils_included
    #endinput
#endif
#define __cwapi_CustomWeapons_Utils_included

#include <amxmodx>
#include "Cwapi/Core/CustomWeapons"

CWeaponUtils_MakeEventCallback(const E_CWeapon_Event:iEvent, const iPlugin, const sCallback[]) {
    new iListener = -1;
    switch (iEvent) {
        case CWeapon_OnSpawn,
             CWeapon_OnSpawnPost,
             CWeapon_OnHolster,
             CWeapon_OnPrimaryAttackPre,
             CWeapon_OnPrimaryAttackPost:
            // (const T_CustomWeapon:iWeapon, const ItemId, const Trie:tAbilityParams = Invalid_Trie)
            iListener = CreateOneForward(iPlugin, sCallback, FP_CELL, FP_CELL, FP_CELL);
        case CWeapon_OnDeploy:
            // (const T_CustomWeapon:iWeapon, const ItemId, &Float:fDeployTime, const Trie:tAbilityParams = Invalid_Trie)
            iListener = CreateOneForward(iPlugin, sCallback, FP_CELL, FP_CELL, FP_VAL_BYREF, FP_CELL);
        case CWeapon_OnSetWeaponBoxModel:
            // (const T_CustomWeapon:iWeapon, const iWeaponBox, const ItemId, const Trie:tAbilityParams = Invalid_Trie)
            iListener = CreateOneForward(iPlugin, sCallback, FP_CELL, FP_CELL, FP_CELL, FP_CELL);
        case CWeapon_OnPlayerTouchWeaponBox:
            // (const T_CustomWeapon:iWeapon, const iWeaponBox, const ItemId, const UserId, const Trie:tAbilityParams = Invalid_Trie)
            iListener = CreateOneForward(iPlugin, sCallback, FP_CELL, FP_CELL, FP_CELL, FP_CELL, FP_CELL);
        case CWeapon_OnAddPlayerItem,
             CWeapon_OnRemovePlayerItem,
             CWeapon_OnPlayerCanHaveWeapon:
            // (const T_CustomWeapon:iWeapon, const ItemId, const UserId, const Trie:tAbilityParams = Invalid_Trie)
            iListener = CreateOneForward(iPlugin, sCallback, FP_CELL, FP_CELL, FP_CELL, FP_CELL);
        case CWeapon_OnPlayerKilled:
            // (const T_CustomWeapon:iWeapon, const ItemId, const VictimId, const KillerId, const Trie:tAbilityParams = Invalid_Trie)
            iListener = CreateOneForward(iPlugin, sCallback, FP_CELL, FP_CELL, FP_CELL, FP_CELL, FP_CELL);
        case CWeapon_OnDamage:
            // (const T_CustomWeapon:iWeapon, const ItemId, const VictimId, const InflictorId, const AttackerId, &Float:fDamage, &iDamageBits, const Trie:tAbilityParams = Invalid_Trie)
            iListener = CreateOneForward(iPlugin, sCallback, FP_CELL, FP_CELL, FP_CELL, FP_CELL, FP_CELL, FP_VAL_BYREF, FP_VAL_BYREF, FP_CELL);
        case CWeapon_OnReload:
            // (const T_CustomWeapon:iWeapon, const ItemId, &iClipSize, &iAnim, &Float:fDelay, const Trie:tAbilityParams = Invalid_Trie)
            iListener = CreateOneForward(iPlugin, sCallback, FP_CELL, FP_CELL, FP_VAL_BYREF, FP_VAL_BYREF, FP_VAL_BYREF, FP_CELL);
    }

    if (iListener < 0) {
        ThrowError("Can`t create forward for function '%s' in plugin #%d.", sCallback, iPlugin);
    }
    
    return iListener;
}

CWeapon_GiveType:CWeaponUtils_StrToGiveType(const sType[], &bool:bIsValid = false) {
    static Trie:tTypesMap = Invalid_Trie;
    if (tTypesMap == Invalid_Trie) {
        tTypesMap = TrieCreate();
        TrieSetCell(tTypesMap, "Smart", CWAPI_GT_SMART);
        TrieSetCell(tTypesMap, "CWAPI_GT_SMART", CWAPI_GT_SMART);

        TrieSetCell(tTypesMap, "Append", CWAPI_GT_APPEND);
        TrieSetCell(tTypesMap, "Add", CWAPI_GT_APPEND);
        TrieSetCell(tTypesMap, "CWAPI_GT_APPEND", CWAPI_GT_APPEND);
        TrieSetCell(tTypesMap, "GT_APPEND", CWAPI_GT_APPEND);

        TrieSetCell(tTypesMap, "Replace", CWAPI_GT_REPLACE);
        TrieSetCell(tTypesMap, "CWAPI_GT_REPLACE", CWAPI_GT_REPLACE);
        TrieSetCell(tTypesMap, "GT_REPLACE", CWAPI_GT_REPLACE);

        TrieSetCell(tTypesMap, "Drop", CWAPI_GT_DROP);
        TrieSetCell(tTypesMap, "CWAPI_GT_DROP", CWAPI_GT_DROP);
        TrieSetCell(tTypesMap, "GT_DROP_AND_REPLACE", CWAPI_GT_DROP);
    }

    new CWeapon_GiveType:iType = CWAPI_GT_SMART;
    bIsValid = TrieGetCell(tTypesMap, sType, iType);

    return iType;
}
