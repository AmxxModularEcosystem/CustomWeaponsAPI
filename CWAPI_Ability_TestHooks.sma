#include <amxmodx>
#include <reapi>
#include <cwapi>

new const PLUG_NAME[] = "[CWAPI][Ability] Test Hooks";
new const PLUG_VER[] = "1.0";

public CWAPI_LoadWeaponsPost(){
    register_plugin(PLUG_NAME, PLUG_VER, "ArKaNeMaN");

    new Array:TestWeapons = CWAPI_GetAbilityWeaponsList("Test");
    new WeaponAbilityData[CWAPI_WeaponAbilityData];
    for(new i = 0; i < ArraySize(TestWeapons); i++){
        ArrayGetArray(TestWeapons, i, WeaponAbilityData);

        CWAPI_RegisterHook(WeaponAbilityData[CWAPI_WAD_WeaponName], CWAPI_WE_PrimaryAttack, "Hook_CWAPI_PrimaryAttack");
        CWAPI_RegisterHook(WeaponAbilityData[CWAPI_WAD_WeaponName], CWAPI_WE_SecondaryAttack, "Hook_CWAPI_SecondaryAttack");
        CWAPI_RegisterHook(WeaponAbilityData[CWAPI_WAD_WeaponName], CWAPI_WE_Reload, "Hook_CWAPI_Reload");
        CWAPI_RegisterHook(WeaponAbilityData[CWAPI_WAD_WeaponName], CWAPI_WE_Deploy, "Hook_CWAPI_Deploy");
        CWAPI_RegisterHook(WeaponAbilityData[CWAPI_WAD_WeaponName], CWAPI_WE_Holster, "Hook_CWAPI_Holster");
        CWAPI_RegisterHook(WeaponAbilityData[CWAPI_WAD_WeaponName], CWAPI_WE_Damage, "Hook_CWAPI_Damage");
        CWAPI_RegisterHook(WeaponAbilityData[CWAPI_WAD_WeaponName], CWAPI_WE_Droped, "Hook_CWAPI_Droped");
        CWAPI_RegisterHook(WeaponAbilityData[CWAPI_WAD_WeaponName], CWAPI_WE_AddItem, "Hook_CWAPI_AddItem");
        CWAPI_RegisterHook(WeaponAbilityData[CWAPI_WAD_WeaponName], CWAPI_WE_Take, "Hook_CWAPI_Take");
    }
    ArrayDestroy(TestWeapons);

    server_print("[%s v%s] loaded.", PLUG_NAME, PLUG_VER);
}

public Hook_CWAPI_PrimaryAttack(const ItemId){
    static UserId; UserId = get_member(ItemId, m_pPlayer);
    client_print(UserId, print_center, "HookTest: PrimaryAttack");
    client_print(UserId, print_console, "HookTest: PrimaryAttack");
}

public Hook_CWAPI_SecondaryAttack(const ItemId){
    static UserId; UserId = get_member(ItemId, m_pPlayer);
    client_print(UserId, print_center, "HookTest: SecondaryAttack");
    client_print(UserId, print_console, "HookTest: SecondaryAttack");
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
    if(!is_user_connected(Victim)) return CWAPI_RET_CONTINUE;
    static UserId; UserId = get_member(ItemId, m_pPlayer);
    client_print(UserId, print_center, "HookTest: Damage [Damage = %.1f | Victim = %n | DamageBits = %d]", Damage, Victim, DamageBits);
    client_print(UserId, print_console, "HookTest: Damage [Damage = %.1f | Victim = %n | DamageBits = %d]", Damage, Victim, DamageBits);
    return CWAPI_RET_CONTINUE;
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