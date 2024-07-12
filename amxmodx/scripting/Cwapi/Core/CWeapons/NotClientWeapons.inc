#include <amxmodx>
#include <reapi>
#include <fakemeta>
#include <hamsandwich>

/**
 * Автор кода, отвечающего за кастомные звуки: BalbuR/DeMNiX.
 * Огромное ему спасибо за помощь с этим всем.
 */

enum {
    eWeaponsDataAnimsFlags_Unsilenced = 1, // 2
    eWeaponsDataAnimsFlags_LastBullet, // 4
    eWeaponsDataAnimsFlags_Silenced, // 8
    eWeaponsDataAnimsFlags_Burst, // 16
    eWeaponsDataAnimsFlags_AttackLeft,
    eWeaponsDataAnimsFlags_AttackRight,
    eWeaponsDataAnimsFlags_EmptyLeft,
    eWeaponsDataAnimsFlags_EmptyLeftAndRight,
}

enum {
	ANIMSTYPE_IDLE,
	ANIMSTYPE_ATTACK,
	ANIMSTYPE_RELOAD,
	ANIMSTYPE_DEPLOY,
	ANIMSTYPE_RELOAD_START,
	ANIMSTYPE_RELOAD_INSERT,
	ANIMSTYPE_RELOAD_END,
	ANIMSTYPE_ATTACH_SILENCER,
	ANIMSTYPE_DETACH_SILENCER,
	ANIMSTYPE_ATTACK_BURST,

	ANIMSTYPE_OTHERS,
	ANIMSTYPE_MAX,
}

NotClientWeapons_ManageAttackHooks(const bool:bState) {
    static fm_hooktrace, fm_playbackevent;

    if (bState) {
        if (!fm_hooktrace) {
            fm_hooktrace = register_forward(FM_TraceLine, "FMHook_NotClientWeapons_TraceLine", true);
        }

        if (!fm_playbackevent) {
            fm_playbackevent = register_forward(FM_PlaybackEvent, "FMHook_NotClientWeapons_Supercede", false);
        }
    } else {
        if (fm_hooktrace) {
            unregister_forward(FM_TraceLine, fm_hooktrace, true);
        }

        if (fm_playbackevent) {
            unregister_forward(FM_PlaybackEvent, fm_playbackevent, false);
        }

        fm_playbackevent = fm_hooktrace = 0;
    }
}

public FMHook_NotClientWeapons_Supercede() {
    return FMRES_SUPERCEDE;
}

public FMHook_NotClientWeapons_TraceLine(Float:tr_start[3], Float:tr_end[3], tr_flag, tr_ignore, tr) {
    if (tr_flag & IGNORE_MONSTERS) {
        return FMRES_IGNORED;
    }


    new iDecal = random_num(41, 45);
    new iHit = get_tr2(tr, TR_pHit);
    if (!is_nullent(iHit)) {
        if (get_entvar(iHit, var_solid) != SOLID_BSP) {
            return FMRES_IGNORED;
        }

        if (get_entvar(iHit, var_rendermode) != 0) {
            static glassdecal;
            if (!glassdecal) {
                glassdecal = engfunc(EngFunc_DecalIndex, "{bproof1");
            }
            
            iDecal = glassdecal;
        }
    }

    static Float:vecEnd[3];
    get_tr2(tr, TR_vecEndPos, vecEnd);
    if (vector_distance(tr_end, vecEnd) < 10.0) {
        return FMRES_IGNORED;
    }

    engfunc(EngFunc_MessageBegin, MSG_PAS, SVC_TEMPENTITY, vecEnd, 0);
    write_byte(TE_GUNSHOTDECAL);
    engfunc(EngFunc_WriteCoord, vecEnd[0]);
    engfunc(EngFunc_WriteCoord, vecEnd[1]);
    engfunc(EngFunc_WriteCoord, vecEnd[2]);
    write_short(iHit > 0 ? iHit : 0);
    write_byte(iDecal);
    message_end();

    return FMRES_IGNORED;
}

NotClientWeapons_SendDefaultWeaponAnim(const UserId, const ItemId, const iAnimType) {
    new iAnimIndex = NotClientWeapons__GetDefaultAnimationData(ItemId, false, iAnimType);
    if (iAnimIndex != -1) {
        NotClientWeapons_SendWeaponAnim(UserId, iAnimIndex, 0);
    }
}

NotClientWeapons_SendWeaponAnim(const iPlayer, const iAnim, const iBody) {
    set_entvar(iPlayer, var_weaponanim, iAnim);

    message_begin(MSG_ONE, SVC_WEAPONANIM, _, iPlayer);
    write_byte(iAnim);
    write_byte(139);
    message_end();

    if (get_entvar(iPlayer, var_iuser1)) {
        return;
    }

    static iCount, iSpectator, iszSpectators[32];
    get_players(iszSpectators, iCount, "bch");
    for (new i = 0; i < iCount; i++) {
        iSpectator = iszSpectators[i];

        if (
            get_entvar(iSpectator, var_iuser1) != 4 /* OBS_IN_EYE */
            || get_entvar(iSpectator, var_iuser2) != iPlayer
        ) {
            continue;
        }

        set_entvar(iSpectator, var_weaponanim, iAnim);

        message_begin(MSG_ONE, SVC_WEAPONANIM, _, iSpectator);
        write_byte(iAnim);
        write_byte(iBody);
        message_end();
    }
}

static NotClientWeapons__GetDefaultAnimationData(
    const iWeapon,
    const bool:BySequenceIndex,
    const iSeqOrAnimTypeIndex,
    const bool:bReturnDefData = false,
    szFullName[32] = "None",
    &iDefAnimFps = 0,
    &iDefFramesTotal = 0,
    &iDefAnimsFlags = 0
) {
    iDefAnimsFlags = 0;
    iDefAnimFps = 0;
    iDefFramesTotal = 0;

    switch (get_member(iWeapon, m_iId)) {
        case CSW_P228: {
            if (BySequenceIndex) {
                switch (iSeqOrAnimTypeIndex) {
                    case 0: { 
                        if (bReturnDefData) { 
                            copy(szFullName, charsmax(szFullName), "idle1");
                            iDefFramesTotal = 11;
                            iDefAnimFps = 16;  
                            iDefAnimsFlags = BIT(eWeaponsDataAnimsFlags_Unsilenced);
                        }
                        return ANIMSTYPE_IDLE; 
                    }
                    case 1, 2, 3, 4: { 
                        if (bReturnDefData) { 
                            formatex(szFullName, charsmax(szFullName), "shoot1");
                            iDefFramesTotal = 26;
                            iDefAnimFps = 34; 
                            iDefAnimsFlags = BIT(eWeaponsDataAnimsFlags_Unsilenced);

                            if (iSeqOrAnimTypeIndex == 4) {
                                iDefAnimsFlags |= BIT(eWeaponsDataAnimsFlags_LastBullet);
                            }
                        }
                        return ANIMSTYPE_ATTACK; 
                    }
                    case 5: { 
                        if (bReturnDefData) { 
                            formatex(szFullName, charsmax(szFullName), "reload");
                            iDefFramesTotal = 96;
                            iDefAnimFps = 35;  
                            iDefAnimsFlags = BIT(eWeaponsDataAnimsFlags_Unsilenced);
                        }
                        return ANIMSTYPE_RELOAD; 
                    }
                    case 6: { 
                        if (bReturnDefData) { 
                            formatex(szFullName, charsmax(szFullName), "draw");
                            iDefFramesTotal = 31;
                            iDefAnimFps = 30;  
                            iDefAnimsFlags = BIT(eWeaponsDataAnimsFlags_Unsilenced);
                        }
                        return ANIMSTYPE_DEPLOY; 
                    } 
                }
            } else {
                switch (iSeqOrAnimTypeIndex) {
                    case ANIMSTYPE_IDLE: {
                        if (bReturnDefData) { 
                            copy(szFullName, charsmax(szFullName), "idle1");
                            iDefFramesTotal = 11;
                            iDefAnimFps = 16;  
                            iDefAnimsFlags = BIT(eWeaponsDataAnimsFlags_Unsilenced);
                        }
                        return 0;
                    }
                    case ANIMSTYPE_ATTACK: {
                        new iAmmo = get_member(iWeapon, m_Weapon_iClip);
                        new iAnim = iAmmo ? random_num(1, 3) : 4;
                        if (bReturnDefData) {
                            iDefFramesTotal = 26;
                            iDefAnimFps = 34;
                            iDefAnimsFlags = BIT(eWeaponsDataAnimsFlags_Unsilenced);

                            switch (iAnim) {
                                case 1: {
                                    formatex(szFullName, charsmax(szFullName), "shoot1");
                                }
                                case 2: {
                                    formatex(szFullName, charsmax(szFullName), "shoot2");
                                }
                                case 3: {
                                    formatex(szFullName, charsmax(szFullName), "shoot3");
                                }
                                case 4: {
                                    formatex(szFullName, charsmax(szFullName), "shoot_empty");
                                    iDefAnimsFlags |= BIT(eWeaponsDataAnimsFlags_LastBullet);
                                }
                            }
                        }

                        return iAnim;
                    }
                    case ANIMSTYPE_RELOAD: {
                        if (bReturnDefData) { 
                            formatex(szFullName, charsmax(szFullName), "reload");
                            iDefFramesTotal = 96;
                            iDefAnimFps = 35;  
                            iDefAnimsFlags = BIT(eWeaponsDataAnimsFlags_Unsilenced);
                        }
                        return 5;
                    }
                    case ANIMSTYPE_DEPLOY: {
                        if (bReturnDefData) { 
                            formatex(szFullName, charsmax(szFullName), "draw");
                            iDefFramesTotal = 31;
                            iDefAnimFps = 30;  
                            iDefAnimsFlags = BIT(eWeaponsDataAnimsFlags_Unsilenced);
                        }

                        return 6;
                    }
                }
            }
        }
        case CSW_SCOUT: {
            if (BySequenceIndex) {
                switch (iSeqOrAnimTypeIndex) {
                    case 0: { 
                        if (bReturnDefData) { 
                            formatex(szFullName, charsmax(szFullName), "idle1");
                            iDefFramesTotal = 11;
                            iDefAnimFps = 16;  
                            iDefAnimsFlags = BIT(eWeaponsDataAnimsFlags_Unsilenced);
                        }
                        return ANIMSTYPE_IDLE; 
                    }
                    case 1, 2: { 
                        if (bReturnDefData) { 
                            formatex(szFullName, charsmax(szFullName), "shoot");
                            iDefFramesTotal = 46;
                            iDefAnimFps = 35;  
                            iDefAnimsFlags = BIT(eWeaponsDataAnimsFlags_Unsilenced);
                        }
                        return ANIMSTYPE_ATTACK; 
                    }
                    case 3: { 
                        if (bReturnDefData) { 
                            formatex(szFullName, charsmax(szFullName), "reload");
                            iDefFramesTotal = 46;
                            iDefAnimFps = 35;  
                            iDefAnimsFlags = BIT(eWeaponsDataAnimsFlags_Unsilenced);
                        }
                        return ANIMSTYPE_RELOAD; 
                    }
                    case 4: { 
                        if (bReturnDefData) { 
                            formatex(szFullName, charsmax(szFullName), "draw");
                            iDefFramesTotal = 31;
                            iDefAnimFps = 30;  
                            iDefAnimsFlags = BIT(eWeaponsDataAnimsFlags_Unsilenced);
                        }
                        return ANIMSTYPE_DEPLOY; 
                    } 
                }
            } else {
                switch (iSeqOrAnimTypeIndex) {
                    case ANIMSTYPE_IDLE: {
                        if (bReturnDefData) { 
                            formatex(szFullName, charsmax(szFullName), "idle1");
                            iDefFramesTotal = 11;
                            iDefAnimFps = 16;  
                            iDefAnimsFlags = BIT(eWeaponsDataAnimsFlags_Unsilenced);
                        }

                        return 0;
                    }
                    case ANIMSTYPE_DEPLOY: {
                        if (bReturnDefData) { 
                            formatex(szFullName, charsmax(szFullName), "draw");
                            iDefFramesTotal = 31;
                            iDefAnimFps = 30;  
                            iDefAnimsFlags = BIT(eWeaponsDataAnimsFlags_Unsilenced);
                        }
                        return 4;
                    }
                    case ANIMSTYPE_RELOAD: {
                        if (bReturnDefData) { 
                            formatex(szFullName, charsmax(szFullName), "reload");
                            iDefFramesTotal = 46;
                            iDefAnimFps = 35;  
                            iDefAnimsFlags = BIT(eWeaponsDataAnimsFlags_Unsilenced);
                        }

                        return 3;
                    }
                    case ANIMSTYPE_ATTACK: {
                        new iAnim = random_num(1, 2);
                        if (bReturnDefData) { 
                            if (iAnim == 1) {
                                formatex(szFullName, charsmax(szFullName), "shoot1");
                            } else {
                                formatex(szFullName, charsmax(szFullName), "shoot2");
                            }

                            iDefFramesTotal = 46;
                            iDefAnimFps = 35;  
                            iDefAnimsFlags = BIT(eWeaponsDataAnimsFlags_Unsilenced);
                        }

                        return iAnim;
                    }
                }
            }
        }
        case CSW_XM1014: {
            if (BySequenceIndex) {
                switch (iSeqOrAnimTypeIndex) {
                    case 0: { 
                        if (bReturnDefData) { 
                            formatex(szFullName, charsmax(szFullName), "idle");
                            iDefFramesTotal = 9;
                            iDefAnimFps = 12;  
                            iDefAnimsFlags = BIT(eWeaponsDataAnimsFlags_Unsilenced);
                        }
                        return ANIMSTYPE_IDLE; 
                    }
                    case 1, 2: { 
                        if (bReturnDefData) {
                            if (iSeqOrAnimTypeIndex == 1) {
                                formatex(szFullName, charsmax(szFullName), "shoot1");
                            } else {
                                formatex(szFullName, charsmax(szFullName), "shoot2");
                            }

                            iDefFramesTotal = 21;
                            iDefAnimFps = 40;  
                            iDefAnimsFlags = BIT(eWeaponsDataAnimsFlags_Unsilenced);
                        }
                        return ANIMSTYPE_ATTACK; 
                    }
                    case 5: { 
                        if (bReturnDefData) { 
                            formatex(szFullName, charsmax(szFullName), "start_reload");
                            iDefFramesTotal = 21;
                            iDefAnimFps = 30;  
                            iDefAnimsFlags = BIT(eWeaponsDataAnimsFlags_Unsilenced);
                        }
                        return ANIMSTYPE_RELOAD_START; 
                    }
                    case 3: {
                        if (bReturnDefData) { 
                            formatex(szFullName, charsmax(szFullName), "insert");
                            iDefFramesTotal = 36;
                            iDefAnimFps = 90;  
                            iDefAnimsFlags = BIT(eWeaponsDataAnimsFlags_Unsilenced);
                        } 
                        return ANIMSTYPE_RELOAD_INSERT; 
                    }
                    case 4: { 
                        if (bReturnDefData) { 
                            formatex(szFullName, charsmax(szFullName), "after_reload");
                            iDefFramesTotal = 13;
                            iDefAnimFps = 30;  
                            iDefAnimsFlags = BIT(eWeaponsDataAnimsFlags_Unsilenced);
                        }
                        return ANIMSTYPE_RELOAD_END; 
                    }
                    case 6: { 
                        if (bReturnDefData) { 
                            formatex(szFullName, charsmax(szFullName), "draw");
                            iDefFramesTotal = 31;
                            iDefAnimFps = 30;  
                            iDefAnimsFlags = BIT(eWeaponsDataAnimsFlags_Unsilenced);
                        }
                        return ANIMSTYPE_DEPLOY; 
                    }
                }
            }
            else {
                switch (iSeqOrAnimTypeIndex) {
                    case ANIMSTYPE_IDLE: {
                        if (bReturnDefData) { 
                            formatex(szFullName, charsmax(szFullName), "idle");
                            iDefFramesTotal = 9;
                            iDefAnimFps = 12;  
                            iDefAnimsFlags = BIT(eWeaponsDataAnimsFlags_Unsilenced);
                        }

                        return 0;
                    }
                    case ANIMSTYPE_ATTACK: {
                        new iAnim = random_num(1,2);
                        if (bReturnDefData) { 
                            if (iAnim == 1) {
                                formatex(szFullName, charsmax(szFullName), "shoot1");
                            } else {
                                formatex(szFullName, charsmax(szFullName), "shoot2");
                            }

                            iDefFramesTotal = 21;
                            iDefAnimFps = 40;  
                            iDefAnimsFlags = BIT(eWeaponsDataAnimsFlags_Unsilenced);
                        }

                        return iAnim; 
                    }
                    case ANIMSTYPE_RELOAD_START: {
                        if (bReturnDefData) { 
                            formatex(szFullName, charsmax(szFullName), "start_reload");
                            iDefFramesTotal = 21;
                            iDefAnimFps = 30;  
                            iDefAnimsFlags = BIT(eWeaponsDataAnimsFlags_Unsilenced);
                        }

                        return 5;
                    }
                    case ANIMSTYPE_RELOAD_INSERT: {
                        if (bReturnDefData) { 
                            formatex(szFullName, charsmax(szFullName), "insert");
                            iDefFramesTotal = 36;
                            iDefAnimFps = 90;  
                            iDefAnimsFlags = BIT(eWeaponsDataAnimsFlags_Unsilenced);
                        } 
                        return 3;
                    }
                    case ANIMSTYPE_RELOAD_END: {
                        if (bReturnDefData) { 
                            formatex(szFullName, charsmax(szFullName), "after_reload");
                            iDefFramesTotal = 13;
                            iDefAnimFps = 30;  
                            iDefAnimsFlags = BIT(eWeaponsDataAnimsFlags_Unsilenced);
                        }
                        return 4;
                    }
                    case ANIMSTYPE_DEPLOY: {
                        if (bReturnDefData) { 
                            formatex(szFullName, charsmax(szFullName), "draw");
                            iDefFramesTotal = 31;
                            iDefAnimFps = 30;  
                            iDefAnimsFlags = BIT(eWeaponsDataAnimsFlags_Unsilenced);
                        }

                        return 6;
                    }
                }
            }
        }
        case CSW_MAC10: {
            if (BySequenceIndex) {
                switch (iSeqOrAnimTypeIndex) {
                    case 0: { return ANIMSTYPE_IDLE; }
                    case 1: { return ANIMSTYPE_RELOAD; }
                    case 2: { return ANIMSTYPE_DEPLOY; }
                    case 3, 4, 5: { return ANIMSTYPE_ATTACK; }
                }
            } else {
                switch (iSeqOrAnimTypeIndex) {
                    case ANIMSTYPE_IDLE: {
                        return 0;
                    }
                    case ANIMSTYPE_RELOAD: {
                        return 1;
                    }
                    case ANIMSTYPE_DEPLOY: {
                        return 2;
                    }
                    case ANIMSTYPE_ATTACK: {
                        return random_num(3, 5);
                    }
                }
            }
        }
        case CSW_AUG: {
            if (BySequenceIndex) {
                switch (iSeqOrAnimTypeIndex) {
                    case 0: { return ANIMSTYPE_IDLE; }
                    case 1: { return ANIMSTYPE_RELOAD; }
                    case 3, 4, 5: { return ANIMSTYPE_ATTACK; }
                    case 2: { return ANIMSTYPE_DEPLOY; }
                }
            } else {
                switch (iSeqOrAnimTypeIndex) {
                    case ANIMSTYPE_IDLE: {
                        return 0;
                    }
                    case ANIMSTYPE_RELOAD: {
                        return 1;
                    }
                    case ANIMSTYPE_ATTACK: {
                        return random_num(3, 5);
                    }
                    case ANIMSTYPE_DEPLOY: {
                        return 2;
                    }
                }
            }
        }
        case CSW_ELITE: {

            if (BySequenceIndex) {
                switch (iSeqOrAnimTypeIndex) {
                    case 0, 1: { 
                        if (bReturnDefData) {
                            if (iSeqOrAnimTypeIndex == 1) {
                                iDefAnimsFlags |= BIT(eWeaponsDataAnimsFlags_EmptyLeft);
                                iDefFramesTotal = 9;
                                iDefAnimFps = 16;
                                copy(szFullName, charsmax(szFullName), "idle_leftempty");  
                            } else {
                                iDefFramesTotal = 11;
                                iDefAnimFps = 16;
                                copy(szFullName, charsmax(szFullName), "idle");  
                            }
                        }

                        return ANIMSTYPE_IDLE; 
                    }
                    case 2..7: {
                        if (bReturnDefData) {
                            switch (iSeqOrAnimTypeIndex) {
                                
                                case 2: {
                                    copy(szFullName, charsmax(szFullName), "shoot_left1"); 
                                }
                                case 3: {
                                    copy(szFullName, charsmax(szFullName), "shoot_left2"); 
                                }
                                case 4: {
                                    copy(szFullName, charsmax(szFullName), "shoot_left3"); 
                                }
                                case 5: {
                                    copy(szFullName, charsmax(szFullName), "shoot_left4"); 
                                }
                                case 6: {
                                    copy(szFullName, charsmax(szFullName), "shoot_left5"); 
                                }
                                case 7: {
                                    iDefAnimsFlags |= BIT(eWeaponsDataAnimsFlags_EmptyLeft);
                                    copy(szFullName, charsmax(szFullName), "shoot_leftlast"); 
                                }
                            }

                            iDefFramesTotal = 9;
                            iDefAnimFps = 16;
                        }

                        return ANIMSTYPE_ATTACK;
                    }
                    case 8..13: {
                        if (bReturnDefData) {
                            switch (iSeqOrAnimTypeIndex) {
                                case 8: {
                                    copy(szFullName, charsmax(szFullName), "shoot_right1"); 
                                }
                                case 9: {
                                    copy(szFullName, charsmax(szFullName), "shoot_right2"); 
                                }
                                case 10: {
                                    copy(szFullName, charsmax(szFullName), "shoot_right3"); 
                                }
                                case 11: {
                                    copy(szFullName, charsmax(szFullName), "shoot_right4"); 
                                }
                                case 12: {
                                    copy(szFullName, charsmax(szFullName), "shoot_right5"); 
                                }
                                case 13: {
                                    iDefAnimsFlags |= BIT(eWeaponsDataAnimsFlags_EmptyLeft);
                                    copy(szFullName, charsmax(szFullName), "shoot_rightlast"); 
                                }
                            }

                            iDefFramesTotal = 9;
                            iDefAnimFps = 16;
                        }

                        if (iSeqOrAnimTypeIndex == 13) {
                            iDefAnimsFlags |= BIT(eWeaponsDataAnimsFlags_EmptyLeftAndRight);
                        }

                        return ANIMSTYPE_ATTACK;
                    }
                    case 14: {
                        if (bReturnDefData) {
                            copy(szFullName, charsmax(szFullName), "reload"); 
                            iDefFramesTotal = 138;
                            iDefAnimFps = 30;
                        }
                        return ANIMSTYPE_RELOAD;
                    }
                    case 15: {
                        if (bReturnDefData) {
                            copy(szFullName, charsmax(szFullName), "draw"); 
                            iDefFramesTotal = 33;
                            iDefAnimFps = 30;
                        }
                        return ANIMSTYPE_DEPLOY;
                    }
                }
            } else {
                switch (iSeqOrAnimTypeIndex) {
                    case ANIMSTYPE_IDLE: {
                        switch (get_member(iWeapon, m_Weapon_iClip)) {
                            case -1: { return -1; }
                            case 0: {
                                // no ammo - no anims (@last shoot@ anim stop)
                                return -1;
                            }
                            case 1: {
                                // left no ammo
                                if (bReturnDefData) {
                                    iDefAnimsFlags |= BIT(eWeaponsDataAnimsFlags_EmptyLeft);
                                    iDefFramesTotal = 9;
                                    iDefAnimFps = 16;
                                    copy(szFullName, charsmax(szFullName), "idle_leftempty");  
                                }

                                return 1; // sequence #1
                            }
                            default: { // default
                                if (bReturnDefData) {
                                    iDefFramesTotal = 11;
                                    iDefAnimFps = 16;
                                    copy(szFullName, charsmax(szFullName), "idle");  
                                }
                                return 0;
                            }
                        }
                    }
                    case ANIMSTYPE_ATTACK: {
                        new iAmmo = get_member(iWeapon, m_Weapon_iClip);
                        new bool:IsLeftAnimation = (get_member(iWeapon, m_Weapon_iWeaponState) & WPNSTATE_ELITE_LEFT) == WPNSTATE_ELITE_LEFT ? true : false;
                        new iAnimSeqNumber;
                        switch (iAmmo) {
                            case 0: {
                                iAnimSeqNumber = -1;
                            }
                            case 1: {
                                if (IsLeftAnimation) {
                                    iAnimSeqNumber = 7;
                                } else {
                                    iAnimSeqNumber = 13;
                                }
                            }
                            default: {
                                if (IsLeftAnimation) {
                                    iAnimSeqNumber = random_num(2, 6);
                                } else {
                                    iAnimSeqNumber = random_num(8, 12);
                                }
                            }
                        }

                        if (bReturnDefData) {
                            switch (iAnimSeqNumber) {
                                case 8: {
                                    copy(szFullName, charsmax(szFullName), "shoot_right1"); 
                                }
                                case 9: {
                                    copy(szFullName, charsmax(szFullName), "shoot_right2"); 
                                }
                                case 10: {
                                    copy(szFullName, charsmax(szFullName), "shoot_right3"); 
                                }
                                case 11: {
                                    copy(szFullName, charsmax(szFullName), "shoot_right4"); 
                                }
                                case 12: {
                                    copy(szFullName, charsmax(szFullName), "shoot_right5"); 
                                }
                                case 13: {
                                    iDefAnimsFlags |= BIT(eWeaponsDataAnimsFlags_EmptyLeft);
                                    copy(szFullName, charsmax(szFullName), "shoot_rightlast"); 
                                }
                            }

                            iDefFramesTotal = 9;
                            iDefAnimFps = 16;
                        }

                        return iAnimSeqNumber;
                    }

                    case ANIMSTYPE_RELOAD: {
                        if (bReturnDefData) {
                            copy(szFullName, charsmax(szFullName), "reload"); 
                            iDefFramesTotal = 138;
                            iDefAnimFps = 30;
                        }

                        return 14;
                    }

                    case ANIMSTYPE_DEPLOY: {
                        if (bReturnDefData) {
                            copy(szFullName, charsmax(szFullName), "draw"); 
                            iDefFramesTotal = 33;
                            iDefAnimFps = 30;
                        }

                        return 15;
                    }
                }
            }
        }
        case CSW_FIVESEVEN: {
            if (BySequenceIndex) {
                switch (iSeqOrAnimTypeIndex) {
                    case 0: { return ANIMSTYPE_IDLE; }
                    case 1, 2: { return ANIMSTYPE_ATTACK; }
                    case 4: { return ANIMSTYPE_RELOAD; }
                    case 5: { return ANIMSTYPE_DEPLOY; }
                }
            } else {
                switch (iSeqOrAnimTypeIndex) {
                    case ANIMSTYPE_IDLE: {
                        return 0;
                    }
                    case ANIMSTYPE_ATTACK: {
                        return random_num(1,2);
                    }
                    case ANIMSTYPE_RELOAD: {
                        return 4;
                    }
                    case ANIMSTYPE_DEPLOY: {
                        return 5;
                    }
                }
            }
        }
        case CSW_UMP45: {
            if (BySequenceIndex) {
                switch (iSeqOrAnimTypeIndex) {
                    case 0: { return ANIMSTYPE_IDLE; }
                    case 3, 4, 5: { return ANIMSTYPE_ATTACK; }
                    case 1: { return ANIMSTYPE_RELOAD; }
                    case 2: { return ANIMSTYPE_DEPLOY; }
                }
            } else {
                switch (iSeqOrAnimTypeIndex) { 
                    case ANIMSTYPE_IDLE: {
                        return 0;
                    }
                    case ANIMSTYPE_ATTACK: {
                        return random_num(3, 5);
                    }
                    case ANIMSTYPE_RELOAD: {
                        return 1;
                    }
                    case ANIMSTYPE_DEPLOY: {
                        return 2;
                    }
                }
            }
        }
        case CSW_SG550: {
            if (BySequenceIndex) {
                switch (iSeqOrAnimTypeIndex) {
                    case 0: { return ANIMSTYPE_IDLE; }
                    case 1, 2: { return ANIMSTYPE_ATTACK; }
                    case 3: { return ANIMSTYPE_RELOAD; }
                    case 4: { return ANIMSTYPE_DEPLOY; }
                }
            } else {
                switch (iSeqOrAnimTypeIndex) {
                    case ANIMSTYPE_IDLE: {
                        return 0;
                    }
                    case ANIMSTYPE_ATTACK: {
                        return random_num(1, 2);
                    }
                    case ANIMSTYPE_RELOAD: {
                        return 3;
                    }
                    case ANIMSTYPE_DEPLOY: {
                        return 4;
                    }
                }
            }
        }
        case CSW_GALIL: {
            if (BySequenceIndex) {
                switch (iSeqOrAnimTypeIndex) {
                    case 0: { return ANIMSTYPE_IDLE; }
                    case 3, 4, 5: { return ANIMSTYPE_ATTACK; }
                    case 1: { return ANIMSTYPE_RELOAD; }
                    case 2: { return ANIMSTYPE_DEPLOY; }
                }
            } else {
                switch (iSeqOrAnimTypeIndex) {
                    case ANIMSTYPE_IDLE: {
                        return 0;
                    }
                    case ANIMSTYPE_ATTACK: {
                        return random_num(3, 5);
                    }
                    case ANIMSTYPE_RELOAD: {
                        return 1;
                    }
                    case ANIMSTYPE_DEPLOY: {
                        return 2;
                    }
                }
            }
        }
        case CSW_FAMAS: {
            if (BySequenceIndex) {
                switch (iSeqOrAnimTypeIndex) {
                    case 0: { return ANIMSTYPE_IDLE; }
                    case 3, 4, 5: { return ANIMSTYPE_ATTACK; }
                    case 1: { return ANIMSTYPE_RELOAD; }
                    case 2: { return ANIMSTYPE_DEPLOY; }
                }
            } else {
                switch (iSeqOrAnimTypeIndex) {
                    case ANIMSTYPE_IDLE: {
                        return 0;
                    }
                    case ANIMSTYPE_ATTACK: {
                        return random_num(3, 5);
                    }
                    case ANIMSTYPE_RELOAD: {
                        return 1;
                    }
                    case ANIMSTYPE_DEPLOY: {
                        return 2;
                    }
                }
            }
        }
        case CSW_USP: {
            if (BySequenceIndex) {
                switch (iSeqOrAnimTypeIndex) {
                    case 0, 8: { return ANIMSTYPE_IDLE; }
                    case 1, 2, 3, 9, 10, 11: { return ANIMSTYPE_ATTACK; }
                    case 5, 13: { return ANIMSTYPE_RELOAD; }
                    case 6, 14: { return ANIMSTYPE_DEPLOY; }
                    case 7: { return ANIMSTYPE_ATTACH_SILENCER; }
                    case 15: { return ANIMSTYPE_DETACH_SILENCER; }
                }
            } else {
                switch (iSeqOrAnimTypeIndex) { 
                    case ANIMSTYPE_IDLE: {
                        new iAnims;
                        if (IsWeaponSilenced(iWeapon)) {
                            iDefAnimsFlags |= BIT(eWeaponsDataAnimsFlags_Silenced);
                            iAnims = 8;
                        } else {
                            iDefAnimsFlags |= BIT(eWeaponsDataAnimsFlags_Unsilenced);
                            iAnims = 0;
                        }

                        return iAnims;
                    }
                    case ANIMSTYPE_ATTACK: {
                        new iAnims;
                        if (IsWeaponSilenced(iWeapon)) {
                            iDefAnimsFlags |= BIT(eWeaponsDataAnimsFlags_Silenced);
                            iAnims = random_num(1, 3);
                        } else {
                            iDefAnimsFlags |= BIT(eWeaponsDataAnimsFlags_Unsilenced);
                            iAnims = random_num(9, 11);
                        }

                        return iAnims;
                    }
                    case ANIMSTYPE_RELOAD: {
                        new iAnims;
                        if (IsWeaponSilenced(iWeapon)) {
                            iDefAnimsFlags |= BIT(eWeaponsDataAnimsFlags_Silenced);
                            iAnims = 13;
                        } else {
                            iDefAnimsFlags |= BIT(eWeaponsDataAnimsFlags_Unsilenced);
                            iAnims = 5;
                        }

                        return iAnims;
                    }
                    case ANIMSTYPE_DEPLOY: {
                        new iAnims;
                        if (IsWeaponSilenced(iWeapon)) {
                            iDefAnimsFlags |= BIT(eWeaponsDataAnimsFlags_Silenced);
                            iAnims = 14;
                        } else {
                            iDefAnimsFlags |= BIT(eWeaponsDataAnimsFlags_Unsilenced);
                            iAnims = 6;
                        }

                        return iAnims;
                    }
                    case ANIMSTYPE_ATTACH_SILENCER: {
                        return 7;
                    }
                    case ANIMSTYPE_DETACH_SILENCER: {
                        return 15;
                    }
                }
            }
        }
        case CSW_GLOCK18: {
            if (BySequenceIndex) {
                switch (iSeqOrAnimTypeIndex) {
                    case 0, 1, 2: { return ANIMSTYPE_IDLE; }
                    case 3, 4, 5: { return ANIMSTYPE_ATTACK; }
                    case 7, 12: { return ANIMSTYPE_RELOAD; }
                    case 8, 11: { return ANIMSTYPE_DEPLOY; }
                    case 9: { return ANIMSTYPE_ATTACK_BURST; }
                }
            } else {
                switch (iSeqOrAnimTypeIndex) {
                    case ANIMSTYPE_IDLE: {
                        return random_num(0, 2);
                    }
                    case ANIMSTYPE_ATTACK: {
                        return random_num(3, 5);
                    }
                    case ANIMSTYPE_RELOAD: {
                        return random_num(0, 1) == 1 ? 7 : 12;
                    }
                    case ANIMSTYPE_DEPLOY: {
                        return random_num(0, 1) == 1 ? 8 : 11;
                    }
                    case ANIMSTYPE_ATTACK_BURST: {
                        return 9;
                    }
                }
            }
        }
        case CSW_AWP: {
            if (BySequenceIndex) {
                switch (iSeqOrAnimTypeIndex) {
                    case 0: { return ANIMSTYPE_IDLE; }
                    case 1, 2, 3: { return  ANIMSTYPE_ATTACK; }
                    case 4: { return  ANIMSTYPE_RELOAD; }
                    case 5: { return  ANIMSTYPE_DEPLOY; }
                }
            } else {
                switch (iSeqOrAnimTypeIndex) {
                    case ANIMSTYPE_IDLE: {
                        return 0;
                    }
                    case ANIMSTYPE_ATTACK : {
                        return random_num(1,3);
                    }
                    case ANIMSTYPE_RELOAD: {
                        return 4;
                    }
                    case ANIMSTYPE_DEPLOY: {
                        return 5;
                    }
                }
            }
        }
        case CSW_MP5NAVY: {
            if (BySequenceIndex) {
                switch (iSeqOrAnimTypeIndex) {
                    case 0: { return ANIMSTYPE_IDLE; }
                    case 3, 4, 5: { return ANIMSTYPE_ATTACK; }
                    case 1: { return ANIMSTYPE_RELOAD; }
                    case 2: { return ANIMSTYPE_DEPLOY; }
                }
            } else {
                switch (iSeqOrAnimTypeIndex) {
                    case ANIMSTYPE_IDLE: {
                        return 0;
                    }
                    case ANIMSTYPE_ATTACK: {
                        return random_num(3, 5);
                    }
                    case ANIMSTYPE_RELOAD: {
                        return 1;
                    } 
                    case ANIMSTYPE_DEPLOY: {
                        return 2;
                    }
                }
            }
        }
        case CSW_M249: {
            if (BySequenceIndex) {
                switch (iSeqOrAnimTypeIndex) {
                    case 0: { return ANIMSTYPE_IDLE; }
                    case 1, 2: { return ANIMSTYPE_ATTACK; }
                    case 3: { return ANIMSTYPE_RELOAD;}
                    case 4: { return ANIMSTYPE_DEPLOY; }
                }
            } else {
                switch (iSeqOrAnimTypeIndex) {
                    case ANIMSTYPE_IDLE: {
                        return 0;
                    }
                    case ANIMSTYPE_ATTACK: {
                        return random_num(1,2);
                    }
                    case ANIMSTYPE_RELOAD: {
                        return 3;
                    }
                    case ANIMSTYPE_DEPLOY: {
                        return 4;
                    }
                }
            }
        }
        case CSW_M3: {
            if (BySequenceIndex) {
                switch (iSeqOrAnimTypeIndex) {
                    case 0: { return ANIMSTYPE_IDLE; }
                    case 1, 2: { return ANIMSTYPE_ATTACK; }
                    case 5: { return ANIMSTYPE_RELOAD_START; }
                    case 3: { return ANIMSTYPE_RELOAD_INSERT; }
                    case 4: { return ANIMSTYPE_RELOAD_END; }
                    case 6: { return ANIMSTYPE_DEPLOY; }
                }
            } else {
                switch (iSeqOrAnimTypeIndex) {
                    case ANIMSTYPE_IDLE: {
                        return 0;
                    }
                    case ANIMSTYPE_ATTACK: {
                        return 1, 2;
                    }
                    case ANIMSTYPE_RELOAD_START: {
                        return 5;
                    }
                    case ANIMSTYPE_RELOAD_INSERT: {
                        return 3;
                    }
                    case ANIMSTYPE_RELOAD_END: {
                        return 4;
                    }
                    case ANIMSTYPE_DEPLOY: {
                        return 6;
                    }
                }
            }
        }
        case CSW_M4A1: {
            if (BySequenceIndex) {
                switch (iSeqOrAnimTypeIndex) {
                    case 0, 7: { return ANIMSTYPE_IDLE; }
                    case 1, 2, 3, 8, 9, 10: { return ANIMSTYPE_ATTACK; }
                    case 4, 11: { return ANIMSTYPE_RELOAD; }
                    case 5, 12: { return ANIMSTYPE_DEPLOY; }
                    case 6: { return ANIMSTYPE_ATTACH_SILENCER; }
                    case 13: { return ANIMSTYPE_DETACH_SILENCER; }
                }
            } else {
                switch (iSeqOrAnimTypeIndex) {
                    case ANIMSTYPE_IDLE: {
                        return IsWeaponSilenced(iWeapon) ? 0 : 7;
                    }
                    case ANIMSTYPE_ATTACK: {
                        return IsWeaponSilenced(iWeapon) ? (random_num(1,3)) : random_num(8,10);
                    }
                    case ANIMSTYPE_RELOAD: {
                        return IsWeaponSilenced(iWeapon) ? 4 : 11;
                    }
                    case ANIMSTYPE_DEPLOY: {
                        return IsWeaponSilenced(iWeapon) ? 5 : 12;
                    }
                    case ANIMSTYPE_ATTACH_SILENCER: {
                        return 6;
                    }
                    case ANIMSTYPE_DETACH_SILENCER: {
                        return 13;
                    }
                }
            }
        }
        case CSW_TMP: {
            if (BySequenceIndex) {
                switch (iSeqOrAnimTypeIndex) {
                    case 0: { return ANIMSTYPE_IDLE; }
                    case 3, 4, 5: { return ANIMSTYPE_ATTACK; }
                    case 1: { return ANIMSTYPE_RELOAD; }
                    case 2: { return ANIMSTYPE_DEPLOY; }
                }
            } else {
                switch (iSeqOrAnimTypeIndex) {
                    case ANIMSTYPE_IDLE: {
                        return 0;
                    }
                    case ANIMSTYPE_ATTACK: {
                        return random_num(3, 5);
                    }
                    case ANIMSTYPE_RELOAD: {
                        return 1;
                    }
                    case ANIMSTYPE_DEPLOY: {
                        return 2;
                    }
                }
            }
        }
        case CSW_G3SG1: {
            if (BySequenceIndex) {
                switch (iSeqOrAnimTypeIndex) {
                    case 0: { return ANIMSTYPE_IDLE; }
                    case 1, 2: { return ANIMSTYPE_ATTACK; }
                    case 3: { return ANIMSTYPE_RELOAD; }
                    case 4: { return ANIMSTYPE_DEPLOY; }
                }
            } else {
                switch (iSeqOrAnimTypeIndex) {
                    case ANIMSTYPE_IDLE: {
                        return 0;
                    }
                    case ANIMSTYPE_ATTACK: {
                        return random_num(1,2);
                    }
                    case ANIMSTYPE_RELOAD: {
                        return 3;
                    }
                    case ANIMSTYPE_DEPLOY: {
                        return 4;
                    }
                }
            }
        }
        case CSW_DEAGLE: {
            if (BySequenceIndex) {
                switch (iSeqOrAnimTypeIndex) {
                    case 0: { return ANIMSTYPE_IDLE; }
                    case 1, 2: { return ANIMSTYPE_ATTACK; }
                    case 4: { return ANIMSTYPE_RELOAD; }
                    case 5: { return ANIMSTYPE_DEPLOY; }
                }
            } else {
                switch (iSeqOrAnimTypeIndex) {
                    case ANIMSTYPE_IDLE: {
                        return 0;
                    }
                    case ANIMSTYPE_ATTACK: {
                        return random_num(1, 2);
                    }
                    case ANIMSTYPE_RELOAD: {
                        return 4;
                    }
                    case ANIMSTYPE_DEPLOY: {
                        return 5;
                    }
                }
            }
        }
        case CSW_SG552: {
            if (BySequenceIndex) {
                switch (iSeqOrAnimTypeIndex) {
                    case 0: { return ANIMSTYPE_IDLE; }
                    case 3, 4, 5: { return ANIMSTYPE_ATTACK; }
                    case 1: { return ANIMSTYPE_RELOAD; }
                    case 2: { return ANIMSTYPE_DEPLOY; }
                }
            } else {
                switch (iSeqOrAnimTypeIndex) {
                    case ANIMSTYPE_IDLE: {
                        return 0;
                    }
                    case ANIMSTYPE_ATTACK: {
                        return random_num(3, 5);
                    }
                    case ANIMSTYPE_RELOAD: {
                        return 1;
                    }
                    case ANIMSTYPE_DEPLOY: {
                        return 2;
                    }
                }
            }
        }
        case CSW_AK47: {
            if (BySequenceIndex) {
                switch (iSeqOrAnimTypeIndex) {
                    case 0: { return ANIMSTYPE_IDLE; }
                    case 3, 4, 5: { return ANIMSTYPE_ATTACK; }
                    case 1: { return ANIMSTYPE_RELOAD; }
                    case 2: { return ANIMSTYPE_DEPLOY; }
                }
            } else {
                switch (iSeqOrAnimTypeIndex) {
                    case ANIMSTYPE_IDLE: {
                        return 0;
                    }
                    case ANIMSTYPE_ATTACK: {
                        return random_num(3, 5);
                    }
                    case ANIMSTYPE_RELOAD: {
                        return 1;
                    }
                    case ANIMSTYPE_DEPLOY: {
                        return 2;
                    }
                }
            }
        }
        case CSW_P90: {
            if (BySequenceIndex) {
                switch (iSeqOrAnimTypeIndex) {
                    case 0: { return ANIMSTYPE_IDLE; }
                    case 3, 4: { return ANIMSTYPE_ATTACK; }
                    case 1: { return ANIMSTYPE_RELOAD; }
                    case 2: { return ANIMSTYPE_DEPLOY; }
                }
            } else {
                switch (iSeqOrAnimTypeIndex) {
                    case ANIMSTYPE_IDLE: {
                        return 0;
                    }
                    case ANIMSTYPE_ATTACK: {
                        return random_num(3,4);
                    }
                    case ANIMSTYPE_RELOAD: {
                        return 1;
                    }
                    case ANIMSTYPE_DEPLOY: {
                        return 2;
                    }
                }
            }
        }
    }

    return ANIMSTYPE_OTHERS;
}
