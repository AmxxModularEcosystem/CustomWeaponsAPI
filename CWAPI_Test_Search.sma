#include <amxmodx>
#include <cwapi>

new const PLUG_NAME[] = "[CWAPI][Test] Search";
new const PLUG_VER[] = "1.0.0";

public CWAPI_LoadWeaponsPost(){
    register_plugin(PLUG_NAME, PLUG_VER, "ArKaNeMaN");
    new WeaponId = -1;
    new WeaponData[CWAPI_WeaponData];
    log_amx("[Test] Found weapons list:");
    while((WeaponId = CWAPI_FindWeapon(WeaponId, CWAPI_WD_HasSecondaryAttack, true)) != -1){
        CWAPI_GetWeaponData(WeaponId, WeaponData);
        log_amx("[Test]     %d: %s", WeaponId, WeaponData[CWAPI_WD_Name]);
    }
    log_amx("[Test] =====================");
}