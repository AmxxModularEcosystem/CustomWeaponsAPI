#if defined _cwapi_included
	#endinput
#endif
#define _cwapi_included

#define CWAPI_VER_MAJOR 0
#define CWAPI_VER_MINOR 7
#define CWAPI_VER_PATCH 2

#define CWAPI_VER_TO_STR(%1.%2.%3) fmt("%d.%d.%d",%1,%2,%3)
#define CWAPI_VER_TO_NUM(%1.%2.%3) ((%1*10000)+(%2*100)+%3)

#define CWAPI_VERSION CWAPI_VER_TO_STR(CWAPI_VER_MAJOR.CWAPI_VER_MINOR.CWAPI_VER_PATCH)
#define CWAPI_VERSION_NUM CWAPI_VER_TO_NUM(CWAPI_VER_MAJOR.CWAPI_VER_MINOR.CWAPI_VER_PATCH)

stock const CWAPI_LIBRARY[] = "CustomWeaponsAPI";
stock const CWAPI_VERSION_CVAR[] = "CWAPI_VERSION";
stock const CWAPI_VERSION_NUM_CVAR[] = "CWAPI_VERSION_NUM";

#define CWAPI_IMPULSE_OFFSET 4354
#define var_CWAPI_ItemOwner var_iuser4

enum CWAPI_WeaponModels{
    CWAPI_WM_V[PLATFORM_MAX_PATH],
    CWAPI_WM_P[PLATFORM_MAX_PATH],
    CWAPI_WM_W[PLATFORM_MAX_PATH],
};

enum CWAPI_WeaponSounds{
    CWAPI_WS_Shot[PLATFORM_MAX_PATH],
    CWAPI_WS_ShotSilent[PLATFORM_MAX_PATH], // Only M4A1 & USP-S
};

enum CWAPI_WeaponEvents{

    /**
    * Описание: Вызывается при основной атаке
    *
    * Параметры: (const ItemId)
    */
    CWAPI_WE_PrimaryAttack = 1,

    /**
    * Описание: Вызывается при вторичной атаке
    *
    * Параметры: (const ItemId)
    */
    CWAPI_WE_SecondaryAttack,

    /**
    * Описание: Вызывается при перезарядке
    *
    * Параметры: (const ItemId)
    */
    CWAPI_WE_Reload,

    /**
    * Описание: Вызывается при доставании оружия
    *
    * Параметры: (const ItemId)
    */
    CWAPI_WE_Deploy,

    /**
    * Описание: Вызывается при убирании оружия
    *
    * Параметры: (const ItemId)
    */
    CWAPI_WE_Holster,

    /**
    * Описание: Вызывается при нанесении урона при помощи оружия
    *
    * Параметры: (const ItemId, const Victim, const Float:Damage, const DamageBits)
    */
    CWAPI_WE_Damage,

    /**
    * Описание: Вызывается при появлении оружия в мире (Выбрасывании)
    *
    * Параметры: (const ItemId, const WeaponBox)
    */
    CWAPI_WE_Droped,

    /**
    * Описание: Вызывается при добавлении оружия в инвентарь
    *
    * Параметры: (const ItemId)
    */
    CWAPI_WE_AddItem,

    /**
    * Описание: Вызывается при выдаче оружия
    *
    * Параметры: (const WeaponId, const UserId)
    */
    CWAPI_WE_Take,

    /**
    * Описание: Вызывается при убийстве при помощи оружия. ПОКА НЕ РАБОТАЕТ
    * 
    * Параметры: (const ItemId, const VictimId)
    */
    CWAPI_WE_Kill,
};

enum _:CWAPI_WeaponData{
    CWAPI_WD_Name[32],
    CWAPI_WD_DefaultName[32],
    CWAPI_WD_Models[CWAPI_WeaponModels],
    CWAPI_WD_Sounds[CWAPI_WeaponSounds],
    CWAPI_WD_ClipSize,
    CWAPI_WD_MaxAmmo,
    Float:CWAPI_WD_MaxWalkSpeed,
    CWAPI_WD_Weight,
    Array:CWAPI_WD_CustomHandlers[CWAPI_WeaponEvents],
    Float:CWAPI_WD_DamageMult,
    CWAPI_WD_Price,
    Float:CWAPI_WD_Accuracy,
    Float:CWAPI_WD_DeployTime,
    Float:CWAPI_WD_ReloadTime,
    Float:CWAPI_WD_PrimaryAttackRate,
    Float:CWAPI_WD_SecondaryAttackRate,
    bool:CWAPI_WD_HasSecondaryAttack,
    Float:CWAPI_WD_Damage,
    bool:CWAPI_WD_HasCustomHud,
};

enum {

    // Продолжить вызов обработчиков и обработать событие
    CWAPI_RET_CONTINUE = 0,

    // Прекратить вызов обработчиков и отменить событие
    CWAPI_RET_HANDLED,
};

enum _:CWAPI_WeaponAbilityData{
    CWAPI_WAD_WeaponName[32],
    Trie:CWAPI_WAD_CustomData,
}

enum CWAPI_GiveType{
    CWAPI_GT_SMART = -1, // Для ножей GT_REPLACE, для гранат GT_APPEND, для остального GT_DROP_AND_REPLACE
    CWAPI_GT_APPEND, // Аналог GT_APPEND
    CWAPI_GT_REPLACE, // Аналог GT_REPLACE
    CWAPI_GT_DROP, // Аналог GT_DROP_AND_REPLACE
}

/*===========[ Natives ]===========*/

/**
 * Регистрирует хук события оружия
 *
 * @param WeaponName        Название оружия указанное в конфиге
 * @param Event             Событие
 * @param HandlerFuncName   Название функции-обработчика
 *
 * @return      Идентификатор хука. -1 в случае ошибки
 */
native CWAPI_RegisterHook(const WeaponName[], const CWAPI_WeaponEvents:Event, const HandlerFuncName[]);

/**
 * Выдаёт кастомное оружие игроку
 *
 * @param UserId      Идентификатор игрока, которому надо выдать оружие
 * @param WeaponName  Название оружия указанное в конфиге
 * @param Type        Тип выдачи (см. CWAPI_GiveType)
 *
 * @return      Идентификатор выданного предмета. -1 в случае ошибки
 */
native CWAPI_GiveWeapon(const UserId, const WeaponName[], const CWAPI_GiveType:Type = CWAPI_GT_SMART);

/**
 * Выдаёт кастомное оружие игроку
 *
 * @param UserId    Идентификатор игрока, которому надо выдать оружие
 * @param WeaponId  Индекс к астомного оружия
 * @param Type      Тип выдачи (см. CWAPI_GiveType)
 *
 * @return      Идентификатор выданного предмета. -1 в случае ошибки
 */
native CWAPI_GiveWeaponById(const UserId, const WeaponId, const CWAPI_GiveType:Type = CWAPI_GT_SMART);

/**
 * Возвращает Array массив со списком кастомных пушек.
 * Надо разрушить массив (ArrayDestroy) после использования
 *
 * @return      Дескриптор Array массива
 */
native Array:CWAPI_GetWeaponsList();

/**
 * Получает индекс кастомного оружия
 *
 * @param WeaponName   Название кастомного оружия
 *
 * @return      Индекс кастомного оружия. -1, если такого оружия не существует.
 */
native CWAPI_GetWeaponId(const WeaponName[]);

/**
 * Получает информацию о кастомном оружии
 *
 * @param WeaponId   Индекс кастомного оружия
 * @param Data       Буфер для записи информации об оружии
 *
 * @return      true в случае удачи, иначе false
 */
native bool:CWAPI_GetWeaponData(const WeaponId, Data[CWAPI_WeaponData]);

/**
 * Добавляет кастомное оружие
 *
 * @note        Если указываются модели, то они должны быть в прекеше.
 *
 * @param WeaponData    Данные о добавляемом оружии
 *
 * @return      Индекс добавленного оружия. -1 в случае ошибки.
 */
native CWAPI_AddCustomWeapon(const WeaponData[CWAPI_WeaponData]);

/**
 * Поиск кастомного оружия
 *
 * @param StartWeaponId    Индекс оружия, с которого начинать поиск
 * @param Field            Параметр по которому искать оружие (Из CWAPI_WeaponData)
 * @param Value            Нужное значение параметра
 *
 * @return      Индекс найденного оружия. -1, если оружие не найдено.
 */
native CWAPI_FindWeapon(const StartWeaponId, const Field, any:...);

/**
 * Возвращает Array массив со списком пушек использующих указанную способность.
 *
 * @param AbilityName   Название способности
 *
 * @return      Дескриптор Array массива
 */
native Array:CWAPI_GetAbilityWeaponsList(const AbilityName[]);

/**
 * Получает индекс кастомного оружия по индексу предмета
 *
 * @param ItemId    Индекс предмета
 *
 * @return  Индекс кастомного оружия. -1 в случае ошибки.
 */
native CWAPI_GetWeaponIdFromEnt(const ItemId);

/**
 * Проверяет кастомное ли оружие по его индексу
 *
 * @param WeaponId    Индекс оружия
 *
 * @return  true если кастомное оружие с таким индексом существует, иначе false.
 */
native bool:CWAPI_IsCustomWeapon(const WeaponId);

/*===========[ Forwards ]===========*/

/**
 * Вызывается после загрузки всех пушек из конфига
 *
 * @noreturn
 */
forward CWAPI_LoadWeaponsPost();

/*===========[ Stocks ]===========*/

/**
 * Получает целочисленное значение из массива параметров способности.
 *
 * @param Params    Дескриптор Trie массива с параметрами способности (CWAPI_WAD_CustomData).
 * @param Name      Название параметра.
 * @param Default   Значение по умолчанию. Будет возвращено, если параметр не задан.
 *
 * @return  Значение параметра.
 */
stock CWAPI_GetAbilParamInt(Trie:Params, const Name[], const Default = 0){
    new Buf[11];
    if(Params != Invalid_Trie && TrieGetString(Params, Name, Buf, charsmax(Buf)))
        return str_to_num(Buf);
    else return Default;
}

/**
 * Получает булевое значение из массива параметров способности.
 *
 * @param Params    Дескриптор Trie массива с параметрами способности (CWAPI_WAD_CustomData).
 * @param Name      Название параметра.
 * @param Default   Значение по умолчанию. Будет возвращено, если параметр не задан.
 *
 * @return  Значение параметра.
 */
stock bool:CWAPI_GetAbilParamBool(Trie:Params, const Name[], const bool:Default = false){
    return bool:CWAPI_GetAbilParamInt(Params, Name, _:Default);
}

/**
 * Получает дробное значение из массива параметров способности.
 *
 * @param Params    Дескриптор Trie массива с параметрами способности (CWAPI_WAD_CustomData).
 * @param Name      Название параметра.
 * @param Default   Значение по умолчанию. Будет возвращено, если параметр не задан.
 *
 * @return  Значение параметра.
 */
stock Float:CWAPI_GetAbilParamFloat(Trie:Params, const Name[], const Float:Default = 0.0){
    new Buf[11];
    if(Params != Invalid_Trie && TrieGetString(Params, Name, Buf, charsmax(Buf)))
        return str_to_float(Buf);
    else return Default;
}

/**
 * Получает флаги доступа в виде битсуммы из массива параметров способности.
 *
 * @param Params    Дескриптор Trie массива с параметрами способности (CWAPI_WAD_CustomData).
 * @param Name      Название параметра.
 * @param Default   Значение по умолчанию. Будет возвращено, если параметр не задан.
 *
 * @return  Значение параметра.
 */
stock CWAPI_GetAbilParamFlags(Trie:Params, const Name[], const Default = ADMIN_ALL){
    new Buf[16];
    if(Params != Invalid_Trie && TrieGetString(Params, Name, Buf, charsmax(Buf)))
        return read_flags(Buf);
    else return Default;
}

/**
 * Получает строку из массива параметров способности.
 *
 * @param Params    Дескриптор Trie массива с параметрами способности (CWAPI_WAD_CustomData).
 * @param Name      Название параметра.
 * @param Out       Буфер для записи полученного значения.
 * @param Len       Длина буфера для записи полученного значения.
 * @param Default   Значение по умолчанию. Будет записано в Out, если параметр не задан.
 *
 * @return  Длина полученной строки.
 */
stock CWAPI_GetAbilParamString(Trie:Params, const Name[], Out[], Len, const Default[] = ""){
    if(Params == Invalid_Trie || !TrieGetString(Params, Name, Out, Len))
        copy(Out, Len, Default);
    return strlen(Out);
}

/**
 * Проверяет наличие запущенного CustomWeaponsAPI и его версию.
 *
 * @note Если оба параметра не указаны, то будет проверено только наличие плагина.
 *
 * @param Min    Минимальная версия.
 * @param Max    Максимальная версия.
 *
 * @return  true, если CWAPI запущен и текущая версия входит в указанный диапазон.
 */
stock bool:CWAPI_CheckVersion(const Min = -1, const Max = -1){
    if(!LibraryExists(CWAPI_LIBRARY, LibType_Library))
        return false;

    new CurVer = CWAPI_GetVersion();
    if(
        (Min != -1 && CurVer < Min)
        || (Max != -1 && CurVer > Max)
    ) return false;

    return true;
}
#define CWAPI_CheckVersionV1(%1) CWAPI_CheckVersion(CWAPI_VER_TO_NUM(%1))
#define CWAPI_CheckVersionV2(%1,%2) CWAPI_CheckVersion(CWAPI_VER_TO_NUM(%1),CWAPI_VER_TO_NUM(%2))

/**
 * Возвращает текущую версию Custom Weapons API
 *
 * @return Значение CWAPI_VERSION_NUM с которым был скомпилирован CWAPI
 */
stock CWAPI_GetVersion(){
    static pCvar;
    if(!pCvar)
        pCvar = get_cvar_pointer(CWAPI_VERSION_NUM_CVAR);
    return get_pcvar_num(pCvar);
}