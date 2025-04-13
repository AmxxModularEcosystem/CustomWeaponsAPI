#include <amxmodx>
#include <json>
#include <cwapi>
#include <ParamsController>
#include "Cwapi/Core/CustomWeapons"

public ParamsController_OnRegisterTypes() {
    CWeapons_Init();

    ParamsController_RegSimpleType(CWAPI_WEAPON_PARAM_TYPE_NAME, "@OnWeaponParamRead");
    ParamsController_RegSimpleType(CWAPI_GIVE_TYPE_PARAM_TYPE_NAME, "@OnGiveTypeParamRead");
}

@OnWeaponParamRead(const JSON:jValue) {
    new sWeaponName[CWAPI_WEAPON_NAME_MAX_LEN];
    json_get_string(jValue, sWeaponName, charsmax(sWeaponName));

    new T_CustomWeapon:iWeapon = CWeapons_Find(sWeaponName);
    if (iWeapon == Invalid_CustomWeapon) {
        PCJson_LogForFile(jValue, "ERROR", "Custom weapon '%s' not found.", sWeaponName);
        return false;
    }
    
    return ParamsController_SetCell(iWeapon);
}

@OnGiveTypeParamRead(const JSON:jValue) {
    new sValue[32];
    json_get_string(jValue, sValue, charsmax(sValue));
    
    new bool:bIsValid;
    new CWeapon_GiveType:iGiveType = CWeaponUtils_StrToGiveType(sValue, bIsValid);

    if (!bIsValid) {
        PCJson_LogForFile(jValue, "ERROR", "Give type '%s' in invalid.", sValue);
        return false;
    }
    
    return ParamsController_SetCell(iGiveType);
}
