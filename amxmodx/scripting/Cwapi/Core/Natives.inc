#include <amxmodx>
#include "Cwapi/Natives"

public plugin_natives() {
    Natives_Init("CWAPI");

    Natives_Reg("Init");

    CWeapons_Natives_Reg();
}

@_Init() {
    PluginInit();
}
