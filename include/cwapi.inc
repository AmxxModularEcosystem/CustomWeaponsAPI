#if defined _cwapi_included
	#endinput
#endif
#define _cwapi_included

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
    * Параметры: (const ItemId)
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
    //Trie:CWAPI_WAD_AdditionalData,
}

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
 * @param UserId            Идентификатор игрока, которому надо выдать оружие
 * @param WeaponName        Название оружия указанное в конфиге
 *
 * @return      Идентификатор выданного предмета. -1 в случае ошибки
 */
native CWAPI_GiveWeapon(const UserId, const WeaponName[]);

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
 * @note        Если указываются модели, то они должны быть в прекеше.
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
 * Вызывается после загрузки всех пушек из конфига
 *
 * @noreturn
 */
forward CWAPI_LoadWeaponsPost();