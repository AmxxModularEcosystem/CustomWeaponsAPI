#include <amxmodx>
#include <reapi>
#include <hamsandwich>
#include <cwapi>

#pragma semicolon 1

#define FIRE_DEAGLE_WEAPON_FULL_NAME fmt("weapon_%s",FIRE_DEAGLE_WEAPON_NAME)
#define var_FireEnt var_iuser2
#define var_FireSwitch var_iuser3
#define GetUserFire(%0) get_entvar(%0,var_FireEnt)

/* НАСТРОЙКИ */

// Спрайт огня
new const FIRE_SPRITE[] = "sprites/FireWeapons/fire.spr";

/* НАСТРОЙКИ */

enum E_Cvars{
    bool:C_SupportFfaMode,
    bool:C_GlowFiredPlayer,
    C_IgniteDuration_Min,
    C_IgniteDuration_Max,
    C_IgniteDamage_Min,
    C_IgniteDamage_Max,
    C_FireMoveRange,
    Float:C_FireDamageInterval,
}

new Cvars[E_Cvars];

new HudDamager;


new const PLUG_NAME[] = "[CWAPI][Ability] Fire";
new const PLUG_VER[] = "1.1";

public CWAPI_LoawWeaponsPost(){
    register_plugin(PLUG_NAME, PLUG_VER, "ArKaNeMaN");

    new Array:FireWeapons = CWAPI_GetAbilityWeaponsList("Fire");
    new WeaponAbilityData[CWAPI_WeaponAbilityData];
    for(new i = 0; i < ArraySize(FireWeapons); i++){
        ArrayGetArray(FireWeapons, i, WeaponAbilityData);
        CWAPI_RegisterHook(WeaponAbilityData[CWAPI_WAD_WeaponName], CWAPI_WE_Damage, "Hook_CWAPI_Damage");
        CWAPI_RegisterHook(WeaponAbilityData[CWAPI_WAD_WeaponName], CWAPI_WE_SecondaryAttack, "Hook_CWAPI_SecondaryAttack");
    }
    ArrayDestroy(FireWeapons);

    RegisterHookChain(RG_CSGameRules_PlayerKilled , "Hook_PlayerKilled", false);
    //RegisterHookChain(RG_RoundEnd , "Hook_RoundEnd", false);

    HudDamager = CreateHudSyncObj();

    IntiCvars();

    server_print("[%s v%s] loaded.", PLUG_NAME, PLUG_VER);
}

public Hook_CWAPI_SecondaryAttack(ItemId){
    static UserId; UserId = get_member(ItemId, m_pPlayer);
    static FireSwitch; FireSwitch = get_entvar(ItemId, var_FireSwitch);
    set_entvar(ItemId, var_FireSwitch, _:!bool:FireSwitch);
    client_print(UserId, print_center, "Огонь в%sключен", FireSwitch ? "ы" : "");
}

public Hook_CWAPI_Damage(const ItemId, const Victim, const Float:Damage, const DamageBits){
    if(!get_entvar(ItemId, var_FireSwitch)) return CWAPI_RET_CONTINUE;

    static Attacker; Attacker = get_member(ItemId, m_pPlayer);

    if(!is_user_connected(Victim) || !is_user_connected(Attacker) || Victim == Attacker) return CWAPI_RET_CONTINUE;
    if(get_user_team(Attacker) == get_user_team(Victim) && !Cvars[C_SupportFfaMode]) return CWAPI_RET_CONTINUE;

    if(GetUserFire(Victim)) PlayerStopFire(Victim);
    PlayerStartFire(Victim, Attacker, random_num(Cvars[C_IgniteDuration_Min], Cvars[C_IgniteDuration_Max]));

    return CWAPI_RET_CONTINUE;
}

public plugin_precache(){
    precache_model(FIRE_SPRITE);
}

//public Hook_RoundEnd(){
//    for(new i = 1; i <= MAX_PLAYERS; i++) if(is_user_connected(i) && GetUserFire(i)) PlayerStopFire(i);
//}

public Hook_PlayerKilled(const Id){
    if(is_user_connected(Id) && GetUserFire(Id)) PlayerStopFire(Id);
}

public client_disconnected(Id){
    if(is_user_connected(Id) && GetUserFire(Id)) PlayerStopFire(Id);
}

public PlayerStopFire(const Id){
    if(!is_user_connected(Id)) return;
    rg_set_user_rendering(Id);
    if(task_exists(Id)) remove_task(Id);
    if(task_exists(Id+200)) remove_task(Id+200);
    static FireEnt; FireEnt = GetUserFire(Id);
    if(!is_nullent(FireEnt)) set_entvar(FireEnt, var_flags, FL_KILLME);
    set_entvar(Id, var_FireEnt, 0);
    return;
}

PlayerStartFire(const Id, const Igniter, const Dur){
    new Ent = rg_create_entity("env_sprite", true);
    if(is_entity(Ent)){
        set_entvar(Ent, var_model, FIRE_SPRITE);
        
        set_entvar(Ent, var_rendermode, kRenderTransAdd);
        set_entvar(Ent, var_renderamt, 200.0);
        
        set_entvar(Ent, var_framerate, 20.0);
        set_entvar(Ent, var_spawnflags, SF_SPRITE_STARTON);
        
        ExecuteHam(Ham_Spawn, Ent);

        set_entvar(Ent, var_owner, Igniter);
        set_entvar(Ent, var_aiment, Id);
        set_entvar(Id, var_FireEnt, Ent);
        set_entvar(Ent, var_movetype, MOVETYPE_FOLLOW);

        static data[1]; data[0] = Id;
        set_task(float(Dur), "PlayerStopFire", Id);
        set_task(Cvars[C_FireDamageInterval], "IgnitePlayer", Id+200, data, 1, "b");

        if(Cvars[C_GlowFiredPlayer]) rg_set_user_rendering(Id, kRenderFxGlowShell, 240, 127, 19, kRenderNormal, 25);
    }
}

public IgnitePlayer(const data[1]){
    static Id; Id = data[0];
    if(!is_user_connected(Id)){
        PlayerStopFire(Id);
        return PLUGIN_CONTINUE;
    }
    static FireEnt; FireEnt = GetUserFire(Id);
    if(
        is_user_alive(Id) && 
        is_entity(FireEnt) && 
        get_entvar(Id, var_waterlevel) < 2
    ){
        static korigin[3]; get_user_origin(Id, korigin);
        static Igniter; Igniter = get_entvar(FireEnt, var_owner);
        if(!is_user_connected(Igniter)) Igniter = Id;
        
        static dmg; dmg = random_num(Cvars[C_IgniteDamage_Min], Cvars[C_IgniteDamage_Max]);
        ExecuteHam(Ham_TakeDamage, Id, FireEnt, Igniter, float(dmg*(is_user_bot(Id) ? 1 : 4)), DMG_BURN);
        set_hudmessage(240, 127, 30, random_float(0.44, 0.46), random_float(0.59, 0.61));
        ShowSyncHudMsg(Igniter, HudDamager, "%d", dmg);
        
        message_begin(MSG_ONE, get_user_msgid("Damage"), {0, 0, 0}, Id);
        write_byte(30);
        write_byte(30);
        write_long(1<<21);
        write_coord(korigin[0]);
        write_coord(korigin[1]);
        write_coord(korigin[2]);
        message_end();
        
        new players[32], inum = 0, pOrigin[3];
        get_players(players, inum, "a");
        for(new i = 0 ; i < inum; ++i){
            get_user_origin(players[i], pOrigin);
            if(get_distance(korigin, pOrigin) < Cvars[C_FireMoveRange] && players[i] != Id && !GetUserFire(players[i])){
                PlayerStartFire(players[i], Igniter, random_num(Cvars[C_IgniteDuration_Min], Cvars[C_IgniteDuration_Max]));
            }
        }
    }
    else PlayerStopFire(Id);
    return PLUGIN_CONTINUE;
}

rg_set_user_rendering(const Ent, const Fx = kRenderFxNone, const r = 0, const g = 0, const b = 0, const Render = kRenderNormal, const Amount = 0){
    set_entvar(Ent, var_rendermode, Render);
    set_entvar(Ent, var_renderamt, float(Amount));
    static Float:Color[3];
    Color[0] = float(r);
    Color[1] = float(g);
    Color[2] = float(b);
    set_entvar(Ent, var_rendercolor, Color);
    set_entvar(Ent, var_renderfx, Fx);
}

IntiCvars(){
    bind_pcvar_num(create_cvar("FireDeagle_SupportFfaMode", "0", FCVAR_NONE, "Вкл/выкл поддержку FFA режима", true, 0.0, true, 1.0), Cvars[C_SupportFfaMode]);
    bind_pcvar_num(create_cvar("FireDeagle_GlowFiredPlayer", "1", FCVAR_NONE, "Псевдосвечение горящих игроков", true, 0.0, true, 1.0), Cvars[C_GlowFiredPlayer]);
    bind_pcvar_num(create_cvar("FireDeagle_IgniteDuration_Min", "4", FCVAR_NONE, "Минимальная длительность горения", true, 1.0), Cvars[C_IgniteDuration_Min]);
    bind_pcvar_num(create_cvar("FireDeagle_IgniteDuration_Max", "8", FCVAR_NONE, "Максимальная длительность горения", true, 1.0), Cvars[C_IgniteDuration_Max]);
    bind_pcvar_num(create_cvar("FireDeagle_IgniteDamage_Min", "4", FCVAR_NONE, "Минимальный урон от горения", true, 1.0), Cvars[C_IgniteDamage_Min]);
    bind_pcvar_num(create_cvar("FireDeagle_IgniteDamage_Max", "8", FCVAR_NONE, "Максимальный урон от горения", true, 1.0), Cvars[C_IgniteDamage_Max]);
    bind_pcvar_num(create_cvar("FireDeagle_FireMoveRange", "70", FCVAR_NONE, "Радиус распостранения огня", true, 1.0), Cvars[C_FireMoveRange]);
    bind_pcvar_float(create_cvar("FireDeagle_FireDamageInterval", "1.0", FCVAR_NONE, "Время между нанесениями урона огнём", true, 1.0), Cvars[C_FireDamageInterval]);

    AutoExecConfig(true, "FireDeagle", "CWAPI");
}
