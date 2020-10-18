#include <amxmodx>
#include <cwapi>

new const ABIL_NAME[] = "Test_AbilParams";
/*
{
    "Abilities": {
        "Test_AbilParams": {
            "TestReal": "10.5",
            "TestInt": "123",
            "TestString": "Test123"
        }
    }
}
*/
new const PLUG_NAME[] = "[CWAPI][Test] Abil Params";
new const PLUG_VER[] = "1.0.0";

public CWAPI_LoadWeaponsPost(){
    register_plugin(PLUG_NAME, PLUG_VER, "ArKaNeMaN");

    new Array:TestWeapons = CWAPI_GetAbilityWeaponsList(ABIL_NAME);
    new WeaponAbilityData[CWAPI_WeaponAbilityData], ParamStr[16];
    for(new i = 0; i < ArraySize(TestWeapons); i++){
        ArrayGetArray(TestWeapons, i, WeaponAbilityData);
        
        log_amx("[Test] ==========[ Weapon: %s ]==========", WeaponAbilityData[CWAPI_WAD_WeaponName]);
        log_amx("[Test]     ParamReal = %.1f", CWAPI_GetAbilParamFloat(WeaponAbilityData[CWAPI_WAD_CustomData], "TestReal"));
        log_amx("[Test]     ParamInt = %d", CWAPI_GetAbilParamInt(WeaponAbilityData[CWAPI_WAD_CustomData], "TestInt"));
        CWAPI_GetAbilParamString(WeaponAbilityData[CWAPI_WAD_CustomData], "TestString", ParamStr, charsmax(ParamStr));
        log_amx("[Test]     ParamStr = %s", ParamStr);
        log_amx("[Test] ==================================");
    }
    ArrayDestroy(TestWeapons);
}