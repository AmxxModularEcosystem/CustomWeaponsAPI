#include <amxmodx>
#include "Cwapi/Natives"
#include "Cwapi/Core/CustomWeapons"
#include "Cwapi/Core/CWeapons/Abilities"

public plugin_natives() {
    Natives_Init("CWAPI");

    Natives_Reg("Init");

    CWeapons_Natives_Reg();
    WAbilities_Natives_Reg();
}

@_Init() {
    PluginInit();
}
