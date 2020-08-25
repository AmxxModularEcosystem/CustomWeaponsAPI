#include <amxmodx>
#include <reapi>
#include <hamsandwich>
#include <cwapi>

#pragma semicolon 1

#define var_FireEnt var_iuser2
#define var_FireSwitch var_iuser3
#define var_FireEndTime var_fuser1
#define GetUserFire(%0) get_entvar(%0,var_FireEnt)
#define IsFired(%1) (!is_nullent(GetUserFire(%1)))
#define Cvar(%1) Cvars[Cvar_%1]
#define Lang(%1) fmt("%l",%1)

/* НАСТРОЙКИ */

// Спрайт огня
new const FIRE_SPRITE[] = "sprites/FireWeapons/fire.spr";

/* НАСТРОЙКИ */

enum E_Cvars{
    Cvar_DamageInterval[24],
    Cvar_Duration[24],
    Cvar_Damage[24],
    Cvar_Glow,
    Cvar_FireRange,
}

new Cvars[E_Cvars];

new const ABILITY_NAME[] = "Fire";
new const PLUG_NAME[] = "[CWAPI][Ability] Fire";
new const PLUG_VER[] = "2.0";

public CWAPI_LoadWeaponsPost(){
    register_plugin(PLUG_NAME, PLUG_VER, "ArKaNeMaN");
    register_dictionary("CWAPI-Fire.ini");

    new Array:WeaponsList = CWAPI_GetAbilityWeaponsList(ABILITY_NAME);
    new AbilityData[CWAPI_WeaponAbilityData];
    for(new i = 0; i < ArraySize(WeaponsList); i++){
        ArrayGetArray(WeaponsList, i, AbilityData);
        CWAPI_RegisterHook(AbilityData[CWAPI_WAD_WeaponName], CWAPI_WE_Damage, "@Hook_CWAPI_Damage");
        CWAPI_RegisterHook(AbilityData[CWAPI_WAD_WeaponName], CWAPI_WE_SecondaryAttack, "@Hook_CWAPI_SecondaryAttack");
    }
    ArrayDestroy(WeaponsList);
    
    RegisterHookChain(RG_CSGameRules_PlayerKilled , "@Hook_PlayerKilled", false);
    RegisterHookChain(RG_RoundEnd , "@Hook_RoundEnd", false);

    IntiCvars();

    server_print("[%s v%s] loaded.", PLUG_NAME, PLUG_VER);
}

public plugin_precache(){
    precache_model(FIRE_SPRITE);
}

@Hook_CWAPI_SecondaryAttack(ItemId){
    static UserId; UserId = get_member(ItemId, m_pPlayer);
    static FireSwitch; FireSwitch = get_entvar(ItemId, var_FireSwitch);
    set_entvar(ItemId, var_FireSwitch, _:!bool:FireSwitch);
    client_print(UserId, print_center, "Огонь в%sключен", FireSwitch ? "ы" : "");
}

@Hook_CWAPI_Damage(const ItemId, const Victim, const Float:Damage, const DamageBits){
    if(!get_entvar(ItemId, var_FireSwitch))
        return CWAPI_RET_CONTINUE;

    static Attacker; Attacker = get_member(ItemId, m_pPlayer);

    if(IsFired(Victim))
        RemoveFire(Victim);

    StartFire(Victim, Attacker, GetFloat(Cvar(Duration)));

    return CWAPI_RET_CONTINUE;
}

@Hook_RoundEnd(){
    for(new i = 1; i <= MAX_PLAYERS; i++)
        RemoveFire(i);
}

@Hook_PlayerKilled(const Id){
    RemoveFire(Id);
}

public client_disconnected(Id){
    RemoveFire(Id);
}

RemoveFire(const UserId){
    if(!is_user_connected(UserId))
        return;

    static FireEnt; FireEnt = GetUserFire(UserId);
    if(!is_nullent(FireEnt))
        set_entvar(FireEnt, var_flags, FL_KILLME);

    rg_set_user_rendering(UserId);

    set_entvar(UserId, var_FireEnt, 0);
    return;
}

StartFire(const UserId, const Attacker, const Float:Duration){
    if(!is_user_alive(UserId) || !is_user_connected(Attacker) || !rg_is_player_can_takedamage(UserId, Attacker))
        return;

    new Ent = rg_create_entity("env_sprite", true);
    if(is_nullent(Ent))
        return;

    set_entvar(Ent, var_model, FIRE_SPRITE);
    set_entvar(Ent, var_rendermode, kRenderTransAdd);
    set_entvar(Ent, var_renderamt, 200.0);
    set_entvar(Ent, var_framerate, 20.0);
    set_entvar(Ent, var_spawnflags, SF_SPRITE_STARTON);
    
    ExecuteHam(Ham_Spawn, Ent);

    set_entvar(Ent, var_owner, Attacker);
    set_entvar(Ent, var_aiment, UserId);
    set_entvar(UserId, var_FireEnt, Ent);
    set_entvar(Ent, var_movetype, MOVETYPE_FOLLOW);
    set_entvar(Ent, var_FireEndTime, get_gametime() + Duration);

    set_entvar(Ent, var_nextthink, get_gametime() + GetFloat(Cvar(DamageInterval)));
    SetThink(Ent, "@Think_Fire");

    if(Cvar(Glow))
        rg_set_user_rendering(UserId, kRenderFxGlowShell, 240, 127, 19, kRenderNormal, 25);
}

@Think_Fire(const EntId){
    new UserId = get_entvar(EntId, var_aiment);
    new Attacker = get_entvar(EntId, var_owner);

    if(!is_user_alive(UserId) || !is_user_connected(Attacker)){
        RemoveFire(UserId);
        set_entvar(EntId, var_flags, FL_KILLME);
        return;
    }

    if(get_gametime() >= get_entvar(EntId, var_FireEndTime) || get_entvar(UserId, var_waterlevel) >= 2){
        RemoveFire(UserId);
        return;
    }

    new Float:Origin[3]; get_entvar(EntId, var_origin, Origin);
    new Float:Damage = GetFloat(Cvar(Damage));

    ExecuteHamB(Ham_TakeDamage, UserId, EntId, Attacker, Damage, DMG_GENERIC);
        
    message_begin(MSG_ONE, get_user_msgid("Damage"), _, UserId);
    write_byte(floatround(Damage));
    write_byte(floatround(Damage));
    write_long(DMG_GENERIC);
    write_coord_f(Origin[0]);
    write_coord_f(Origin[1]);
    write_coord_f(Origin[2]);
    message_end();

    if(Cvar(FireRange) > 0)
        for(new i = 1; i <= MAX_PLAYERS; i++){
            if(i == UserId)
                continue;

            if(!is_user_alive(i))
                continue;

            if(IsFired(i))
                continue;
            
            new Float:Target[3]; get_entvar(i, var_origin, Target);
            if(get_distance_f(Origin, Target) <= Cvar(FireRange))
                StartFire(i, Attacker, GetFloat(Cvar(Duration)));
        }

    set_entvar(EntId, var_nextthink, get_gametime() + GetFloat(Cvar(DamageInterval)));
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

    bind_pcvar_string(create_cvar(
        "CWAPI_Fire_DamageInterval", "0.8 1.2",
        FCVAR_NONE, Lang("CVAR_DAMAGE_INTERVAL")
    ), Cvar(DamageInterval), charsmax(Cvar(DamageInterval)));

    bind_pcvar_string(create_cvar(
        "CWAPI_Fire_Duration", "4.0 8.0",
        FCVAR_NONE, Lang("CVAR_DURATION")
    ), Cvar(Duration), charsmax(Cvar(Duration)));

    bind_pcvar_string(create_cvar(
        "CWAPI_Fire_Damage", "4.0 8.0",
        FCVAR_NONE, Lang("CVAR_DAMAGE")
    ), Cvar(Damage), charsmax(Cvar(Damage)));

    bind_pcvar_num(create_cvar(
        "CWAPI_Fire_Glow", "1",
        FCVAR_NONE, Lang("CVAR_GLOW"),
        true, 0.0, true, 1.0
    ), Cvar(Glow));

    bind_pcvar_num(create_cvar(
        "CWAPI_Fire_FireRange", "70",
        FCVAR_NONE, Lang("CVAR_FIRE_RANGE"),
        true, 0.0
    ), Cvar(FireRange));

    AutoExecConfig(true, "Fire", "CustomWeaponsAPI/Modules");
}

Float:GetFloat(const Str[]){
    new strNums[2][11];
    new Count = parse(Str, strNums[0], charsmax(strNums[]), strNums[1], charsmax(strNums[]));
    if(Count == 1)
        return str_to_float(strNums[0]);
    else if(Count < 1)
        return 0.0;
    else return random_float(str_to_float(strNums[0]), str_to_float(strNums[1]));
}