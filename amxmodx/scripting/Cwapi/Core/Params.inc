#include <amxmodx>
#include <json>
#include <cwapi>
#include <ParamsController>
#include "Cwapi/Core/CustomWeapons"

public ParamsController_OnRegisterTypes() {
    // Возможно выглядит странно, но вроде норм)
    CWAPI_Init();

    ParamsController_RegSimpleType("CWAPI-Weapon", "@OnWeaponParamRead");
}

@OnWeaponParamRead(const JSON:jValue) {
    new sWeaponName[CWAPI_WEAPON_NAME_MAX_LEN];
    json_get_string(jValue, sWeaponName, charsmax(sWeaponName));

    new T_CustomWeapon:iWeapon = CWeapons_Find(sWeaponName);
    if (iWeapon == Invalid_CustomWeapon) {
        ThrowError("Custom weapon '%s' not found.", sWeaponName);
        return false;
    }
    
    return ParamsController_SetCell(iWeapon);
}
