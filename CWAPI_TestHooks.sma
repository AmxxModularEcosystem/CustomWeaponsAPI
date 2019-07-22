#include <amxmodx>
#include <reapi>
#include <cwapi>

new const WEAPON_NAME[] = "FireDeagle";

new const PLUG_NAME[] = "[CWAPI] Test Hooks";
new const PLUG_VER[] = "1.0";

public plugin_init(){
    register_plugin(PLUG_NAME, PLUG_VER, "ArKaNeMaN");

    server_print("[%s v%s] loaded.", PLUG_NAME, PLUG_VER);
}

public CWAPI_LoawWeaponsPost(){
    //log_amx("CWAPI_LoawWeaponsPost");
    CWAPI_RegisterHook(WEAPON_NAME, CWAPI_WE_Shot, "Hook_CWAPI_Shot");
    CWAPI_RegisterHook(WEAPON_NAME, CWAPI_WE_Reload, "Hook_CWAPI_Reload");
    CWAPI_RegisterHook(WEAPON_NAME, CWAPI_WE_Deploy, "Hook_CWAPI_Deploy");
    CWAPI_RegisterHook(WEAPON_NAME, CWAPI_WE_Holster, "Hook_CWAPI_Holster");
    CWAPI_RegisterHook(WEAPON_NAME, CWAPI_WE_Damage, "Hook_CWAPI_Damage");
    CWAPI_RegisterHook(WEAPON_NAME, CWAPI_WE_Droped, "Hook_CWAPI_Droped");
    CWAPI_RegisterHook(WEAPON_NAME, CWAPI_WE_AddItem, "Hook_CWAPI_AddItem");
    CWAPI_RegisterHook(WEAPON_NAME, CWAPI_WE_Take, "Hook_CWAPI_Take");
}

public Hook_CWAPI_Shot(const ItemId){
    static UserId; UserId = get_member(ItemId, m_pPlayer);
    client_print(UserId, print_center, "HookTest: Shot");
    client_print(UserId, print_console, "HookTest: Shot");
}

public Hook_CWAPI_Reload(const ItemId){
    static UserId; UserId = get_member(ItemId, m_pPlayer);
    client_print(UserId, print_center, "HookTest: Reload");
    client_print(UserId, print_console, "HookTest: Reload");
}

public Hook_CWAPI_Deploy(const ItemId){
    static UserId; UserId = get_member(ItemId, m_pPlayer);
    client_print(UserId, print_center, "HookTest: Deploy");
    client_print(UserId, print_console, "HookTest: Deploy");
}

public Hook_CWAPI_Holster(const ItemId){
    static UserId; UserId = get_member(ItemId, m_pPlayer);
    client_print(UserId, print_center, "HookTest: Holster");
    client_print(UserId, print_console, "HookTest: Holster");
}

public Hook_CWAPI_Damage(const ItemId, const Victim, const FLoat:Damage, const DamageBits){
    static UserId; UserId = get_member(ItemId, m_pPlayer);
    client_print(UserId, print_center, "HookTest: Damage [Damage = %.1f | Victim = %n | DamageBits = %d]", Damage, Victim, DamageBits);
    client_print(UserId, print_console, "HookTest: Damage [Damage = %.1f | Victim = %n | DamageBits = %d]", Damage, Victim, DamageBits);
    return CWAPI_RET_HANDLED;
}

public Hook_CWAPI_Droped(const ItemId, const WeaponBox){
    client_print(0, print_center, "HookTest: Droped");
    client_print(0, print_console, "HookTest: Droped");
}

public Hook_CWAPI_AddItem(const ItemId, const UserId){
    client_print(UserId, print_center, "HookTest: AddItem");
    client_print(UserId, print_console, "HookTest: AddItem");
}

public Hook_CWAPI_Take(const ItemId, const UserId){
    client_print(UserId, print_center, "HookTest: Take");
    client_print(UserId, print_console, "HookTest: Take");
}