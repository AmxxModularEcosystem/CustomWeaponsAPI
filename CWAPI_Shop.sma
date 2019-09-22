#include <amxmodx>
#include <reapi>
#include <cwapi>

#pragma semicolon 1

new Menu_Shop;

new const PLUG_NAME[] = "[CWAPI] Shop";
new const PLUG_VER[] = "1.0.1";

public CWAPI_LoadWeaponsPost(){
    register_plugin(PLUG_NAME, PLUG_VER, "ArKaNeMaN");

    register_dictionary("cwapi.txt");

    new Array:WeaponsList = CWAPI_GetWeaponsList();

    Menu_Shop = menu_create("\r[\yCWAPI\r] \wShop", "MenuHandler_Shop");

    new WeaponData[CWAPI_WeaponData];
    for(new i = 0; i < ArraySize(WeaponsList); i++){
        ArrayGetArray(WeaponsList, i, WeaponData);
        //log_amx("Create menu: Add Item: i = %d | Name = %s | Price = %d", i, WeaponData[CWAPI_WD_Name], WeaponData[CWAPI_WD_Price]);
        if(WeaponData[CWAPI_WD_Price] < 1) continue;
        menu_additem(Menu_Shop, fmt("\r[$%d] \y%s", WeaponData[CWAPI_WD_Price], WeaponData[CWAPI_WD_Name]), WeaponData[CWAPI_WD_Name]);
    }

    ArrayDestroy(WeaponsList);

    register_clcmd("cwshop", "Cmd_OpenShop");
    register_clcmd("say /cwshop", "Cmd_OpenShop");
    register_clcmd("say_team /cwshop", "Cmd_OpenShop");

    server_print("[%s v%s] loaded.", PLUG_NAME, PLUG_VER);
}

public Cmd_OpenShop(const Id){
    if(!IsUserInBuyZone(Id)){
        client_print(Id, print_center, "%L", LANG_PLAYER, "OUT_OF_BUYZONE");
        return PLUGIN_HANDLED;
    }
    menu_display(Id, Menu_Shop);
    return PLUGIN_HANDLED;
}

public MenuHandler_Shop(const Id, const Menu, const Item){
    if(Item == MENU_EXIT){
        menu_cancel(Id);
        return;
    }

    static Access, Data[32];
    menu_item_getinfo(Menu, Item, Access, Data, charsmax(Data));

    client_cmd(Id, "CWAPI_Buy %s", Data);
    return;
}

// В зоне закупки ли игрок
bool:IsUserInBuyZone(const Id){
    static Signal[UnifiedSignals]; get_member(Id, m_signals, Signal);
    return (Signal[US_Signal] == _:SIGNAL_BUY);
}