#include <amxmodx>
#include "Cwapi/Natives"

public plugin_natives() {
    Natives_Init("CWAPI");

    Natives_Reg("Init");
}

@_Init() {
    PluginInit();
}
