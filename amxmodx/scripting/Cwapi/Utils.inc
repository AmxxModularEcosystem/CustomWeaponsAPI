#if defined __Cwapi_utils_included
    #endinput
#endif
#define __Cwapi_utils_included

#include <amxmodx>
#include <reapi>
#include <json>

stock const __INT_TEMPLATE_STR[] = "%d";

#define CompositeMacros(%1) \
    do { %1 } while(is_linux_server() == 0xDEADBEEF)

#define IntToStr(%1) \
    fmt(__INT_TEMPLATE_STR, %1)

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

#define IsWeaponSilenced(%0) \
    bool:((WPNSTATE_M4A1_SILENCED | WPNSTATE_USP_SILENCED) & get_member(%0, m_Weapon_iWeaponState))

#define CallOnce() CompositeMacros( \
    static bool:__CallOnce_bCalled; \
    if (__CallOnce_bCalled) return; \
    __CallOnce_bCalled = true; \
)

stock GetAttackerWeapon(const AttackerId, const InflictorId) {
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
stock SetWeaponNextAttack(ItemId, Float:Rate) {
    set_member(ItemId, m_Weapon_flNextPrimaryAttack, Rate);
    set_member(ItemId, m_Weapon_flNextSecondaryAttack, Rate);
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

// https://github.com/Nord1cWarr1or/Universal-AFK-Manager/blob/6272afbb8c27f8b7ad770e3036b5960042001e6b/scripting/UAFKManager.sma#L298-L321
stock GetAmxxVersionNum() {
    static iRes;
    if (iRes) {
        return iRes;
    }

    new sAmxxVer[16];
    get_amxx_verstring(sAmxxVer, charsmax(sAmxxVer));

    if (strfind(sAmxxVer, "1.10.0") != -1) {
        iRes = 1100;
    } else if (strfind(sAmxxVer, "1.9.0") != -1) {
        iRes = 190;
    } else if (strfind(sAmxxVer, "1.8.3") != -1) {
        iRes = 183;
    } else if (strfind(sAmxxVer, "1.8.2") != -1) {
        iRes = 182;
    } else {
        iRes = 1;
    }

    return iRes;
}

#define RegisterPluginByVars() CompositeMacros( \
    if (GetAmxxVersionNum() < 1100) { \
        register_plugin(PluginName, PluginVersion, PluginAuthor); \
    } \
)

stock CreateConstCvar(const sCvarName[], const sCvarValue[]) {
    set_pcvar_string(create_cvar(sCvarName, sCvarValue), sCvarValue);
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

#define ThrowError(%1) CompositeMacros( \
    log_error(0, %1); \
    log_amx(%1); \
)

stock InstantReload(const ItemId) {
    set_member(ItemId, m_Weapon_iClip, rg_get_iteminfo(ItemId, ItemInfo_iMaxClip));
}
