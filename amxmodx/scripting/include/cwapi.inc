#if defined _cwapi_included
	#endinput
#endif
#define _cwapi_included

#include <amxmodx>

#define _CWAPI_VERSION "1.0.0-b1"

/**
 * Строковое представление версии CWAPI.
 */
stock const CWAPI_VERSION[] = _CWAPI_VERSION;

/**
 * Название библиотеки, регистрируемой CWAPI.
 */
stock const CWAPI_LIBRARY[] = "CustomWeaponsAPI";

/**
 * Название квара, хранящего текущую версию CWAPI.
 */
stock const CWAPI_VERSION_CVAR[] = "CWAPI_VERSION";

/**
 * Название папки с конфигурационными файлами CWAPI.
 * 
 * @note `amxmodx/configs/plugins/<CWAPI_CONFIGS_FOLDER>/`
 */
stock const CWAPI_CONFIGS_FOLDER[] = "CustomWeaponsAPI";

/**
 * Название типа параметра для кастомного оружия.
 * 
 * @note После чтения такого параметра в Trie будет лежать хендлер оружия.
 */
stock const CWAPI_WEAPON_PARAM_TYPE_NAME[] = "CWAPI-Weapon";

/**
 * Название типа параметра для способа выдачи кастомного оружия.
 * 
 * @note После чтения такого параметра в Trie будет лежать элемент перечисления CWeapon_GiveType.
 */
stock const CWAPI_GIVE_TYPE_PARAM_TYPE_NAME[] = "CWAPI-GiveType";


/**
 * Максимальная длина названия способности.
 */
#define CWAPI_ABILITY_NAME_MAX_LEN 32

/**
 * Максимальная длина названия кастомного оружия.
 */
#define CWAPI_WEAPON_NAME_MAX_LEN 32

/**
 * Максимальная длина названия функции-обработчика события.
 */
#define CWAPI_CALLBACK_MAX_LEN 128

/**
 * Смещение значения var/pev_impulse для кастомного оружия.
 */
#define CWAPI_IMPULSE_OFFSET 4354

#define var_CWAPI_ItemOwner var_iuser4
#define var_CWAPI_DefaultDeployAnimIndex var_iuser3
#define EV_INT_CWAPI_ItemOwner EV_INT_iuser4

/**
 * Тэг хендлера кастомного оружия.
 */
enum T_CustomWeapon { Invalid_CustomWeapon = -1 }

/**
 * Тэг хендлера способности.
 */
enum T_WeaponAbility { Invalid_WeaponAbility = -1 }

/**
 * События кастомного оружия
 */
enum E_CWeapon_Event {
    /**
     * Вызывается при создании кастомного оружия перед установкой параметров.
     * 
     * (const T_CustomWeapon:iWeapon, const ItemId, const Trie:tAbilityParams = Invalid_Trie)
     * 
     * @param iWeapon Хендлер кастомного оружия
     * @param ItemId Индекс сущности предмета оружия
     * @param tAbilityParams Параметры способности (только для событий, зарегистрированных для способностей)
     * 
     * @return При возвращении CWAPI_STOP_MAIN создание оружия будет полностью отменено. Натив CWAPI_Weapons_Give при этом вернёт -1.
     *         При возвращении CWAPI_STOP оружие будет создано, но кастомные параметры не будут установлены.
     */
    CWeapon_OnSpawn,

    /**
     * Вызывается при установке world модели на weaponbox.
     * 
     * (const T_CustomWeapon:iWeapon, const iWeaponBox, const ItemId, const Trie:tAbilityParams = Invalid_Trie)
     * 
     * @param iWeapon Хендлер кастомного оружия
     * @param iWeaponBox Индекс сущности weaponbox'a
     * @param ItemId Индекс сущности предмета оружия
     * @param tAbilityParams Параметры способности (только для событий, зарегистрированных для способностей)
     * 
     * @return При возвращении CWAPI_STOP_MAIN weaponbox будет удалён.
     *         При возвращении CWAPI_CONTINUE модель не будет заменена на кастомную.
     */
    CWeapon_OnSetWeaponBoxModel,

    /**
     * Вызывается при добавлении оружия игроку в инвентарь.
     * 
     * (const T_CustomWeapon:iWeapon, const ItemId, const UserId, const Trie:tAbilityParams = Invalid_Trie)
     * 
     * @param iWeapon Хендлер кастомного оружия
     * @param ItemId Индекс сущности предмета оружия
     * @param UserId Индекс игрока
     * @param tAbilityParams Параметры способности (только для событий, зарегистрированных для способностей)
     * 
     * @noreturn Возвращаемое значение игнорируется.
     */
    CWeapon_OnAddPlayerItem,

    /**
     * Вызывается при взятии оружия в руки.
     * 
     * (const T_CustomWeapon:iWeapon, const ItemId, &Float:fDeployTime, const Trie:tAbilityParams = Invalid_Trie)
     * 
     * @param iWeapon Хендлер кастомного оружия
     * @param ItemId Индекс сущности предмета оружия
     * @param fDeployTime Время доставания оружия, может быть изменено
     * @param tAbilityParams Параметры способности (только для событий, зарегистрированных для способностей)
     * 
     * @noreturn Возвращаемое значение игнорируется.
     */
    CWeapon_OnDeploy,

    /**
     * Вызывается при убирании оружия из рук.
     * 
     * (const T_CustomWeapon:iWeapon, const ItemId, const Trie:tAbilityParams = Invalid_Trie)
     * 
     * @param iWeapon Хендлер кастомного оружия
     * @param ItemId Индекс сущности предмета оружия
     * @param tAbilityParams Параметры способности (только для событий, зарегистрированных для способностей)
     * 
     * @noreturn Возвращаемое значение игнорируется.
     */
    CWeapon_OnHolster,

    /**
     * Вызывается при удалении оружия из инвентаря игрока.
     * 
     * (const T_CustomWeapon:iWeapon, const ItemId, const UserId, const Trie:tAbilityParams = Invalid_Trie)
     * 
     * @param iWeapon Хендлер кастомного оружия
     * @param ItemId Индекс сущности предмета оружия
     * @param UserId Индекс игрока
     * @param tAbilityParams Параметры способности (только для событий, зарегистрированных для способностей)
     * 
     * @noreturn Возвращаемое значение игнорируется.
     */
    CWeapon_OnRemovePlayerItem,

    /**
     * Вызывается при убийстве игрока оружием.
     * 
     * (const T_CustomWeapon:iWeapon, const ItemId, const VictimId, const KillerId, const Trie:tAbilityParams = Invalid_Trie)
     * 
     * @param iWeapon Хендлер кастомного оружия
     * @param ItemId Индекс сущности предмета оружия
     * @param VictimId Индекс игрока жертвы
     * @param KillerId Индекс игрока убийцы
     * @param tAbilityParams Параметры способности (только для событий, зарегистрированных для способностей)
     * 
     * @noreturn Возвращаемое значение игнорируется.
     */
    CWeapon_OnPlayerKilled,

    /**
     * Вызывается после полного создания кастомного оружия.
     * 
     * (const T_CustomWeapon:iWeapon, const ItemId, const Trie:tAbilityParams = Invalid_Trie)
     * 
     * @param iWeapon Хендлер кастомного оружия
     * @param ItemId Индекс сущности предмета оружия
     * @param tAbilityParams Параметры способности (только для событий, зарегистрированных для способностей)
     * 
     * @noreturn Возвращаемое значение игнорируется.
     */
    CWeapon_OnSpawnPost,

    /**
     * Вызывается при нанесении урона кастомным оружием.
     * 
     * (const T_CustomWeapon:iWeapon, const ItemId, const VictimId, const InflictorId, const AttackerId, &Float:fDamage, &iDamageBits, const Trie:tAbilityParams = Invalid_Trie)
     * 
     * @param iWeapon Хендлер кастомного оружия
     * @param ItemId Индекс сущности предмета оружия
     * @param VictimId Индекс игрока жертвы
     * @param InflictorId Индекс сущности, нанёсшей урон
     * @param AttackerId Индекс атакующего игрока
     * @param fDamage Величина нанесённого урона, может быть изменена
     * @param iDamageBits Битсумма типов нанесённого урона, может быть изменена
     * @param tAbilityParams Параметры способности (только для событий, зарегистрированных для способностей)
     * 
     * @return При возвращении CWAPI_STOP_MAIN урон нанесён не будет.
     */
    CWeapon_OnDamage,

    /**
     * Вызывается в начале перезарядки.
     * 
     * (const T_CustomWeapon:iWeapon, const ItemId, &iClipSize, &iAnim, &Float:fDelay, const Trie:tAbilityParams = Invalid_Trie)
     * 
     * @param iWeapon Хендлер кастомного оружия
     * @param ItemId Индекс сущности предмета оружия
     * @param iClipSize Размер магазина (до какого кол-ва патронов надо зарядить)
     * @param iAnim Порядковый номер анимации перезарядки
     * @param fDelay Задержка перед началом стрельбы (длительность перезарядки)
     * @param tAbilityParams Параметры способности (только для событий, зарегистрированных для способностей)
     * 
     * @return При возвращении CWAPI_STOP_MAIN перезарядка будет отменена.
     */
    CWeapon_OnReload,

    /**
     * Вызывается при попытке игрока поднять оружие.
     * 
     * (const T_CustomWeapon:iWeapon, const ItemId, const UserId, const Trie:tAbilityParams = Invalid_Trie)
     * 
     * @param iWeapon Хендлер кастомного оружия
     * @param ItemId Индекс сущности предмета оружия
     * @param UserId Индекс игрока
     * @param tAbilityParams Параметры способности (только для событий, зарегистрированных для способностей)
     * 
     * @return При возвращении CWAPI_STOP_MAIN поднятие будет заблокировано.
     */
    CWeapon_OnPlayerCanHaveWeapon,

    /**
     * Вызывается перед выстрелом.
     * 
     * (const T_CustomWeapon:iWeapon, const ItemId, const Trie:tAbilityParams = Invalid_Trie)
     * 
     * @param iWeapon Хендлер кастомного оружия
     * @param ItemId Индекс сущности предмета оружия
     * @param tAbilityParams Параметры способности (только для событий, зарегистрированных для способностей)
     * 
     * @return При возвращении CWAPI_STOP_MAIN выстрел будет заблокирован.
     */
    CWeapon_OnPrimaryAttackPre,

    /**
     * Вызывается после выстрела.
     * 
     * (const T_CustomWeapon:iWeapon, const ItemId, const Trie:tAbilityParams = Invalid_Trie)
     * 
     * @param iWeapon Хендлер кастомного оружия
     * @param ItemId Индекс сущности предмета оружия
     * @param tAbilityParams Параметры способности (только для событий, зарегистрированных для способностей)
     * 
     * @noreturn Возвращаемое значение игнорируется.
     */
    CWeapon_OnPrimaryAttackPost,

    /**
     * Вызывается при касании weaponbox'а игроком.
     * 
     * (const T_CustomWeapon:iWeapon, const iWeaponBox, const ItemId, const UserId, const Trie:tAbilityParams = Invalid_Trie)
     * 
     * @param iWeapon Хендлер кастомного оружия
     * @param iWeaponBox Индекс сущности weaponbox'a
     * @param ItemId Индекс сущности предмета оружия
     * @param UserId Индекс игрока
     * @param tAbilityParams Параметры способности (только для событий, зарегистрированных для способностей)
     * 
     * @return При возвращении CWAPI_STOP_MAIN касание будет заблокировано.
     */
    CWeapon_OnPlayerTouchWeaponBox,
}

/**
 * Публиные параметры кастомного оружия
 */
enum E_CWeapon_Attribute {
    /**
     * Название стандартного оружия, на котором основано кастомное
     * 
     * Тип: Строка
     */
    CWeaponAttr_Reference,

    /**
     * Название кастомного оружия
     * 
     * Тип: Строка
     */
    CWeaponAttr_Name,
}

/**
 * Возможные возвращаемые значения для обработчиков событий.
 */
enum {
    /**
     * Продолжить вызов обработчиков и обработать событие
     */
    CWAPI_CONTINUE = 0,

    /**
     * Прекратить вызов обработчиков и прервать действия CWAPI, если они есть
     */
    CWAPI_STOP,

    /**
     * Прекратить вызов обработчиков, попытаться прервать действия по-умолчанию и прервать действия CWAPI, если они есть
     */
    CWAPI_STOP_MAIN,
};

enum CWeapon_GiveType {
    /**
     * Для ножей GT_REPLACE, для гранат GT_APPEND, для остального GT_DROP_AND_REPLACE
     */
    CWAPI_GT_SMART = -1,

    /**
     * Аналог GT_APPEND
     */
    CWAPI_GT_APPEND,

    /**
     * Аналог GT_REPLACE
     */
    CWAPI_GT_REPLACE,

    /**
     * Аналог GT_DROP_AND_REPLACE
     */
    CWAPI_GT_DROP,
}

/**
 * Принудительная загрузка Custom Weapons API.
 * 
 * @note    Решает проблему порядка запуска плагинов.
 *          В плагинах, использующих CWAPI, можно вызывать этот натив там, где удобно,
 *          вместо ожидания форварда CWAPI_OnLoaded.
 * 
 * @note    Если CWAPI уже был загружен, ничего не произойдёт.
 * 
 * @noreturn
 */
native CWAPI_Init();

/**
 * Вызывается после инициализации и перед загрузкой оружия из файлов.
 * 
 * @note    Тут должны регистрироваться все способности.
 * 
 * @noreturn
 */
forward CWAPI_OnLoad();

/**
 * Вызывается после загрузки оружия из файлов.
 * 
 * @note    Вызов этого форварда означает полную готовность CWAPI к использованию.
 *          То есть все способности созданы и всё оружие загружено.
 * 
 * @noreturn
 */
forward CWAPI_OnLoaded();

/**
 * Поиск кастомного оружия по его названию.
 * 
 * @param sWeaponName   Название кастомного оружия.
 * 
 * @return              Хендлер кастомного оружия или Invalid_CustomWeapon в случае неудачи.
 */
native T_CustomWeapon:CWAPI_Weapons_Find(const sWeaponName[]);

/**
 * Поиск кастомного оружия по сущности предмета.
 * 
 * @param ItemId    Индекс сущности предмета.
 * 
 * @return          Хендлер кастомного оружия или Invalid_CustomWeapon в случае неудачи.
 */
native T_CustomWeapon:CWAPI_Weapons_FindByItem(const ItemId);

/**
 * Регистрация обработчика события кастомного оружия.
 * 
 * @param iWeapon   Хендлер кастомного оружия.
 * @param iEvent    Событие.
 * @param sCallback Название функции-обработчика.
 * 
 * @noreturn
 */
native CWAPI_Weapons_AddEventListener(const T_CustomWeapon:iWeapon, const E_CWeapon_Event:iEvent, const sCallback[]);

/**
 * Выдача кастомного оружия игроку.
 * 
 * @param UserId    Индекс игрока.
 * @param iWeapon   Хендлер кастомного оружия.
 * @param iGiveType Метод выдачи оружия (см. enum CWeapon_GiveType).
 * 
 * @return          Индекс созданный ентити предмета. -1, если оружие по какой-то причине не было выдано.
 */
native CWAPI_Weapons_Give(const UserId, const T_CustomWeapon:iWeapon, const CWeapon_GiveType:iGiveType = CWAPI_GT_SMART);

/**
 * Получение названия кастомного оружия.
 * 
 * @param iWeapon   Хендлер кастомного оружия.
 * @param sOut      Буфер для записи названия кастомного оружия.
 * @param iOutLen   Размер буфера.
 * 
 * @return          Количество записанных в буфер ячеек.
 */
native CWAPI_Weapons_GetName(const T_CustomWeapon:iWeapon, sOut[], const iOutLen);

/**
 * Получение названия кастомного оружия.
 * 
 * @param iWeapon   Хендлер кастомного оружия.
 * 
 * @return          Название кастомного оружия.
 */
stock CWAPI_Weapons_iGetName(const T_CustomWeapon:iWeapon) {
    new sWeaponName[CWAPI_WEAPON_NAME_MAX_LEN];
    CWAPI_Weapons_GetName(iWeapon, sWeaponName, charsmax(sWeaponName));

    return sWeaponName;
}

/**
 * Получение атрибута кастомного оружия.
 * 
 * @param iWeapon       Хендлер кастомного оружия.
 * @param iAttribute    Хендлер кастомного оружия.
 * 
 * @note                Для строковых атсрибутов:
 *                      CWAPI_Weapons_GetAttribute(iWeapon, iAttribute, sOut, iOutLen);
 *                      - sOut - буфер для записи атрибута;
 *                      - iOutLen - размер буфера.
 *                      - Вернёт количество записанных в буфер ячеек.
 *  
 * @note                Для числовых атрибутов:
 *                      CWAPI_Weapons_GetAttribute(iWeapon, iAttribute);
 *                      - Вернёт значение атрибута.
 * 
 * @return              Количество записанных в буфер ячеек.
 */
native any:CWAPI_Weapons_GetAttribute(const T_CustomWeapon:iWeapon, const E_CWeapon_Attribute:iAttribute, any:...);

/**
 * Регистрация способности кастомного оружия.
 * 
 * @param sAbilityName  Название способности.
 * 
 * @return              Хендлер зарегистрированной способности.
 */
native T_WeaponAbility:CWAPI_Abilities_Register(const sAbilityName[]);

/**
 * Добавление параметров для способности кастомного оружия.
 * 
 * @param iAbility  Хендлер способности.
 * @param any:...   Перечисление параметров.
 * 
 * @note            Формат перечисления параметров:
 *                      CWAPI_Abilities_AddParams(..., const sParamName[], const sParamType[], const bool:bParamRequired, ...);
 * 
 * @note            sParamName - Название параметра;
 *                  sParamType - Название типа параметра;
 *                  bParamRequired - Обязателен ли параметр.
 * 
 * @note            Пример:
 *                      CWAPI_Abilities_AddParams(iAbility,
 *                          "Param1", "String", true,
 *                          "Param2", "Integer", false
 *                      );
 * 
 * @noreturn
 */
native CWAPI_Abilities_AddParams(const T_WeaponAbility:iAbility, any:... /* Params */);

/**
 * Добавление обработчика события кастомного оружия для способности.
 * 
 * @param iAbility  Хендлер способности.
 * @param iEvent    Элемент перечисления события.
 * @param sCallback Название функции-обработчика.
 * 
 * @noreturn
 */
native CWAPI_Abilities_AddEventListener(const T_WeaponAbility:iAbility, const E_CWeapon_Event:iEvent, const sCallback[]);
