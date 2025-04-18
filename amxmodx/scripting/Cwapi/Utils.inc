#if defined __Cwapi_utils_included
    #endinput
#endif
#define __Cwapi_utils_included

#include <amxmodx>
#include <reapi>
#include <json>
#include "Cwapi/DebugMode"

// Thx: https://dev-cs.ru/threads/222/page-12#post-103174 & Garey
new __CompositeMacros__always_false = false;
#define CompositeMacros(%1) \
    do { %1 } while(__CompositeMacros__always_false)

const WEAPON_PISTOLS = (
    BIT(_:WEAPON_P228)
    | BIT(_:WEAPON_GLOCK)
    | BIT(_:WEAPON_ELITE)
    | BIT(_:WEAPON_FIVESEVEN)
    | BIT(_:WEAPON_USP)
    | BIT(_:WEAPON_GLOCK18)
    | BIT(_:WEAPON_DEAGLE)
);

const WEAPON_GRENADE = (
    BIT(_:WEAPON_HEGRENADE)
    | BIT(_:WEAPON_SMOKEGRENADE)
    | BIT(_:WEAPON_FLASHBANG)
);

#define CallOnce() CompositeMacros( \
    static bool:__CallOnce_bCalled; \
    if (__CallOnce_bCalled) \
        return; \
    __CallOnce_bCalled = true; \
)

stock IntToStr(const iNum) {
    return fmt("%d", iNum);
}

stock bool:IsWeaponSilenced(const ItemId) {
    return bool:((WPNSTATE_M4A1_SILENCED | WPNSTATE_USP_SILENCED) & get_member(ItemId, m_Weapon_iWeaponState));
}

stock bool:IsWeaponBurst(const ItemId) {
    return bool:((WPNSTATE_GLOCK18_BURST_MODE | WPNSTATE_FAMAS_BURST_MODE) & get_member(ItemId, m_Weapon_iWeaponState));
}

stock bool:IsWeaponGrenade(const WeaponIdType:weaponIndex) {
    return (WEAPON_GRENADE & BIT(_:weaponIndex)) > 0;
}

// Получение ID итема из WeaponBox'а
stock GetItemFromWeaponBox(const WeaponBox) {
    for (new i = 0, ItemId; i < MAX_ITEM_TYPES; i++) {
        ItemId = get_member(WeaponBox, m_WeaponBox_rgpPlayerItems, i);
        if (!is_nullent(ItemId)) {
            return ItemId;
        }
    }
    return NULLENT;
}

// Установка времени до следующего выстрела
stock SetWeaponNextAttack(const ItemId, const Float:fTime) {
    set_member(get_member(ItemId, m_pPlayer), m_flNextAttack, fTime);
    set_member(ItemId, m_Weapon_flNextPrimaryAttack, fTime);
    set_member(ItemId, m_Weapon_flNextSecondaryAttack, fTime);
}

stock SetWeaponIdleAnim(const UserId, const ItemId) {
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

stock ShowWeaponListHud(const UserId, const ItemId) {
    new sWeaponName[32];
    rg_get_iteminfo(ItemId, ItemInfo_pszName, sWeaponName, charsmax(sWeaponName));

    message_begin(MSG_ONE, UserMsgs[UM_WeaponList], .player = UserId);
    write_string(sWeaponName);
    write_byte(get_member(ItemId, m_Weapon_iPrimaryAmmoType));
    write_byte(rg_get_iteminfo(ItemId, ItemInfo_iMaxAmmo1));
    write_byte(get_member(ItemId, m_Weapon_iSecondaryAmmoType));
    write_byte(rg_get_iteminfo(ItemId, ItemInfo_iMaxAmmo2));
    write_byte(rg_get_iteminfo(ItemId, ItemInfo_iSlot));
    write_byte(rg_get_iteminfo(ItemId, ItemInfo_iPosition));
    write_byte(rg_get_iteminfo(ItemId, ItemInfo_iId));
    write_byte(rg_get_iteminfo(ItemId, ItemInfo_iFlags));
    message_end();
}

stock CreateConstCvar(const sCvarName[], const sCvarValue[]) {
    set_pcvar_string(create_cvar(sCvarName, sCvarValue, FCVAR_SERVER), sCvarValue);
}

stock bool:mult_member_f(const EntId, const any:iMember, const Float:fMult) {
    return bool:set_member(EntId, iMember, (Float:get_member(EntId, iMember)) * fMult);
}

stock bool:mult_member(const EntId, const any:iMember, const iMult) {
    return bool:set_member(EntId, iMember, get_member(EntId, iMember) * iMult);
}

stock bool:set_member_if_specified(const EntId, const any:iMember, const any:iValue, const any:iNotSpecVal) {
    if (iValue != iNotSpecVal) {
        set_member(EntId, iMember, iValue);
    }
}

stock bool:rg_set_iteminfo_if_specified(const ItemId, const ItemInfo:iItemInfo, const any:iValue, const any:iNotSpecVal) {
    if (iValue != iNotSpecVal) {
        rg_set_iteminfo(ItemId, iItemInfo, iValue);
    }
}

// Не использовать для вепонбоксов! (thx fl0werD)
stock RemoveEntity(const EntId) {
    set_entvar(EntId, var_flags, FL_KILLME);
    set_entvar(EntId, var_nextthink, -1.0);
}

stock RemoveWeaponBox(const EntId) {
    set_entvar(EntId, var_nextthink, get_gametime() + 0.0001);
}

stock MultItemDamage(const ItemId, const Float:fMultiplier) {
    new WeaponIdType:WeaponId = WeaponIdType:rg_get_iteminfo(ItemId, ItemInfo_iId);

    mult_member_f(ItemId, m_Weapon_flBaseDamage, fMultiplier);
    if (WeaponId == WEAPON_KNIFE) {
        mult_member_f(ItemId, m_Knife_flStabBaseDamage, fMultiplier);
        mult_member_f(ItemId, m_Knife_flSwingBaseDamage, fMultiplier);
        mult_member_f(ItemId, m_Knife_flSwingBaseDamage_Fast, fMultiplier);
    } else if (WeaponId == WEAPON_M4A1) {
        mult_member_f(ItemId, m_M4A1_flBaseDamageSil, fMultiplier);
    } else if (WeaponId == WEAPON_USP) {
        mult_member_f(ItemId, m_USP_flBaseDamageSil, fMultiplier);
    } else if (WeaponId == WEAPON_FAMAS) {
        mult_member_f(ItemId, m_Famas_flBaseDamageBurst, fMultiplier);
    }
}

stock WeaponIdType:GetWeaponIdByName(const sWeaponName[]) {
    return WeaponIdType:rg_get_weapon_info(sWeaponName, WI_ID);
}

stock InstantReload(const ItemId) {
    set_member(ItemId, m_Weapon_iClip, rg_get_iteminfo(ItemId, ItemInfo_iMaxClip));
}

stock bool:IsUserIdValid(const UserId) {
    return (
        UserId > 0
        && UserId <= MAX_PLAYERS
    );
}

stock FillBpAmmoByItem(const UserId, const ItemId, const amountOverride = -1) {
    new amount = rg_get_iteminfo(ItemId, ItemInfo_iMaxAmmo1);
    if (amount <= 0) {
        return;
    }

    if (amountOverride > 0) {
        amount = amountOverride;
    }

    new WeaponIdType:WeaponId = WeaponIdType:rg_get_iteminfo(ItemId, ItemInfo_iId);

    new currentAmount = rg_get_user_bpammo(UserId, WeaponId);
    if (currentAmount >= amount) {
        return;
    }

    rg_set_user_bpammo(UserId, WeaponId, amount);
}

stock bool:IsPistol(const ItemId) {
    return (WEAPON_PISTOLS & BIT(rg_get_iteminfo(ItemId, ItemInfo_iId))) > 0;
}

stock bool:IsKnife(const ItemId) {
    if (is_nullent(ItemId)) {
        return false;
    }

    return WeaponIdType:rg_get_iteminfo(ItemId, ItemInfo_iId) == WEAPON_KNIFE;
}

stock bool:WeaponCanAttack(const ItemId) {
    return IsKnife(ItemId) || (
        get_member(ItemId, m_Weapon_iClip) > 0
        && (
            !IsPistol(ItemId)
            || get_member(ItemId, m_Weapon_iShotsFired) < 1
        )
    );
}

stock bool:WeaponWasAttack(const ItemId) {
    return (
        !get_member(ItemId, m_Weapon_fFireOnEmpty)
        && (
            !IsPistol(ItemId)
            || get_member(ItemId, m_Weapon_iShotsFired) <= 1
        )
    );
}
