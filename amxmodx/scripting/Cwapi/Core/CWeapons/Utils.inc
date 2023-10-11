#if defined __cwapi_CustomWeapons_Utils_included
    #endinput
#endif
#define __cwapi_CustomWeapons_Utils_included

#include <amxmodx>
#include "Cwapi/Core/CustomWeapons"

CWeaponUtils_MakeEventCallback(const E_CWeapon_Event:iEvent, const iPlugin, const sCallback[]) {
    new iListener = -1;
    switch (iEvent) {
        case CWeapon_OnSpawn, CWeapon_OnDeploy, CWeapon_OnHolster:
            // (const T_CustomWeapon:iWeapon, const ItemId, const Trie:tAbilityParams = Invalid_Trie)
            iListener = CreateOneForward(iPlugin, sCallback, FP_CELL, FP_CELL, FP_CELL);
        case CWeapon_OnSetWeaponBoxModel:
            // (const T_CustomWeapon:iWeapon, const iWeaponBox, const ItemId, const Trie:tAbilityParams = Invalid_Trie)
            iListener = CreateOneForward(iPlugin, sCallback, FP_CELL, FP_CELL, FP_CELL, FP_CELL);
        case CWeapon_OnAddPlayerItem, CWeapon_OnRemovePlayerItem:
            // (const T_CustomWeapon:iWeapon, const ItemId, const UserId, const Trie:tAbilityParams = Invalid_Trie)
            iListener = CreateOneForward(iPlugin, sCallback, FP_CELL, FP_CELL, FP_CELL, FP_CELL);
    }

    if (iListener < 0) {
        ThrowError("Can`t create forward for function '%s' in plugin #%d.", sCallback, iPlugin);
    }
    
    return iListener;
}