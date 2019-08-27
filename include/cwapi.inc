#if defined _cwapi_included
	#endinput
#endif
#define _cwapi_included

enum CWAPI_WeaponEvents{

    /**
    * Описание: Вызывается при выстреле
    *
    * Параметры: (const ItemId)
    */
    CWAPI_WE_Shot = 1,

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
}

enum {

    // Продолжить вызов обработчиков и обработка события
    CWAPI_RET_CONTINUE = 1,

    // Прекратить вызов обработчиков и отменить событие
    CWAPI_RET_HANDLED,
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
 * @param WeaponName        Название оружия указанное в конфиге
 * @param UserId            Идентификатор игрока, которому надо выдать оружие
 *
 * @return      Идентификатор выданного предмета. -1 в случае ошибки
 */
native CWAPI_GiveWeapon(const WeaponName[], const UserId);

/**
 * Возвращает Array массив со всеми кастомными пушками.
 * Надо разрушить массив (ArrayDestroy) после использования
 *
 * @return      Дескриптор Array массива
 */
native Array:CWAPI_GetWeaponsList();

/**
 * Вызывается после загрузки всех пушек из конфига
 *
 * @noreturn
 */
forward CWAPI_LoawWeaponsPost();
