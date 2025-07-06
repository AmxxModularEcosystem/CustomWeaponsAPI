#if defined _cwapi_included
	#endinput
#endif
#define _cwapi_included

#include <amxmodx>
#include <reapi>

#define _CWAPI_VERSION "1.0.1"

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
 * Название папки с файлами оружия CWAPI.
 * 
 * @note `amxmodx/configs/plugins/<CWAPI_CONFIGS_FOLDER>/<CWAPI_WEAPONS_FOLDER>`
 */
stock const CWAPI_WEAPONS_FOLDER[] = "Weapons";

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
 * Ключ параметра способности, в котором хранится хендлер этой способности.
 */
stock const CWAPI_ABILITY_HANDLER_PARAM_KEY[] = "_ability_handler";


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
     * (const T_CustomWeapon:weapon, const item, const Trie:p = Invalid_Trie)
     * 
     * @param weapon Хендлер кастомного оружия
     * @param item Индекс сущности предмета оружия
     * @param p Параметры способности (только для событий, зарегистрированных для способностей)
     * 
     * @return При возвращении CWAPI_STOP_MAIN создание оружия будет полностью отменено. Натив CWAPI_Weapons_Give при этом вернёт -1.
     *         При возвращении CWAPI_STOP оружие будет создано, но кастомные параметры не будут установлены.
     */
    CWeapon_OnSpawn,

    /**
     * Вызывается при установке world модели на weaponbox.
     * 
     * (const T_CustomWeapon:weapon, const weaponBox, const item, const Trie:p = Invalid_Trie)
     * 
     * @param weapon Хендлер кастомного оружия
     * @param weaponBox Индекс сущности weaponbox'a
     * @param item Индекс сущности предмета оружия
     * @param p Параметры способности (только для событий, зарегистрированных для способностей)
     * 
     * @return При возвращении CWAPI_STOP_MAIN weaponbox будет удалён.
     *         При возвращении CWAPI_STOP модель не будет заменена на кастомную.
     */
    CWeapon_OnSetWeaponBoxModel,

    /**
     * Вызывается при добавлении оружия игроку в инвентарь.
     * 
     * (const T_CustomWeapon:weapon, const item, const playerIndex, const Trie:p = Invalid_Trie)
     * 
     * @param weapon Хендлер кастомного оружия
     * @param item Индекс сущности предмета оружия
     * @param playerIndex Индекс игрока
     * @param p Параметры способности (только для событий, зарегистрированных для способностей)
     * 
     * @noreturn Возвращаемое значение игнорируется.
     */
    CWeapon_OnAddPlayerItem,

    /**
     * Вызывается при взятии оружия в руки.
     * 
     * (const T_CustomWeapon:weapon, const item, &Float:deployTime, const Trie:p = Invalid_Trie)
     * 
     * @param weapon Хендлер кастомного оружия
     * @param item Индекс сущности предмета оружия
     * @param deployTime Время доставания оружия, может быть изменено
     * @param p Параметры способности (только для событий, зарегистрированных для способностей)
     * 
     * @noreturn Возвращаемое значение игнорируется.
     */
    CWeapon_OnDeploy,

    /**
     * Вызывается при убирании оружия из рук.
     * 
     * (const T_CustomWeapon:weapon, const item, const Trie:p = Invalid_Trie)
     * 
     * @param weapon Хендлер кастомного оружия
     * @param item Индекс сущности предмета оружия
     * @param p Параметры способности (только для событий, зарегистрированных для способностей)
     * 
     * @noreturn Возвращаемое значение игнорируется.
     */
    CWeapon_OnHolster,

    /**
     * Вызывается при удалении оружия из инвентаря игрока.
     * 
     * (const T_CustomWeapon:weapon, const item, const playerIndex, const Trie:p = Invalid_Trie)
     * 
     * @param weapon Хендлер кастомного оружия
     * @param item Индекс сущности предмета оружия
     * @param playerIndex Индекс игрока
     * @param p Параметры способности (только для событий, зарегистрированных для способностей)
     * 
     * @noreturn Возвращаемое значение игнорируется.
     */
    CWeapon_OnRemovePlayerItem,

    /**
     * Вызывается при убийстве игрока оружием.
     * 
     * (const T_CustomWeapon:weapon, const item, const victimIndex, const killerIndex, const Trie:p = Invalid_Trie)
     * 
     * @param weapon Хендлер кастомного оружия
     * @param item Индекс сущности предмета оружия
     * @param victimIndex Индекс игрока жертвы
     * @param killerIndex Индекс игрока убийцы
     * @param p Параметры способности (только для событий, зарегистрированных для способностей)
     * 
     * @noreturn Возвращаемое значение игнорируется.
     */
    CWeapon_OnPlayerKilled,

    /**
     * Вызывается после полного создания кастомного оружия.
     * 
     * (const T_CustomWeapon:weapon, const item, const Trie:p = Invalid_Trie)
     * 
     * @param weapon Хендлер кастомного оружия
     * @param item Индекс сущности предмета оружия
     * @param p Параметры способности (только для событий, зарегистрированных для способностей)
     * 
     * @noreturn Возвращаемое значение игнорируется.
     */
    CWeapon_OnSpawnPost,

    /**
     * Вызывается при нанесении урона кастомным оружием.
     * 
     * (const T_CustomWeapon:weapon, const item, const victimIndex, const inflictorIndex, const attackerIndex, &Float:damage, &damageBits, const Trie:p = Invalid_Trie)
     * 
     * @param weapon Хендлер кастомного оружия
     * @param item Индекс сущности предмета оружия (см. примечание ниже).
     * @param victimIndex Индекс игрока жертвы
     * @param inflictorIndex Индекс сущности, нанёсшей урон
     * @param attackerIndex Индекс атакующего игрока
     * @param damage Величина нанесённого урона, может быть изменена
     * @param damageBits Битсумма типов нанесённого урона, может быть изменена
     * @param p Параметры способности (только для событий, зарегистрированных для способностей)
     * 
     * @note В параметре item может приходить ноль, если урон был нанесён гранатой или другим не моментальным снарядом.
     *       В том числе, когда урон нанесён через натив CWAPI_Weapons_EmitDamage.
     * 
     * @return При возвращении CWAPI_STOP_MAIN урон нанесён не будет.
     */
    CWeapon_OnDamage,

    /**
     * Вызывается в начале перезарядки.
     * 
     * (const T_CustomWeapon:weapon, const item, &clipSize, &animation, &Float:delay, const Trie:p = Invalid_Trie)
     * 
     * @param weapon Хендлер кастомного оружия
     * @param item Индекс сущности предмета оружия
     * @param clipSize Размер магазина (до какого кол-ва патронов надо зарядить)
     * @param animation Порядковый номер анимации перезарядки
     * @param delay Задержка перед началом стрельбы (длительность перезарядки)
     * @param p Параметры способности (только для событий, зарегистрированных для способностей)
     * 
     * @return При возвращении CWAPI_STOP_MAIN перезарядка будет отменена.
     */
    CWeapon_OnReload,

    /**
     * Вызывается при попытке игрока поднять оружие.
     * 
     * (const T_CustomWeapon:weapon, const item, const playerIndex, const Trie:p = Invalid_Trie)
     * 
     * @param weapon Хендлер кастомного оружия
     * @param item Индекс сущности предмета оружия
     * @param playerIndex Индекс игрока
     * @param p Параметры способности (только для событий, зарегистрированных для способностей)
     * 
     * @return При возвращении CWAPI_STOP_MAIN поднятие будет заблокировано.
     */
    CWeapon_OnPlayerCanHaveWeapon,

    /**
     * Вызывается перед выстрелом.
     * 
     * (const T_CustomWeapon:weapon, const item, const Trie:p = Invalid_Trie)
     * 
     * @param weapon Хендлер кастомного оружия
     * @param item Индекс сущности предмета оружия
     * @param p Параметры способности (только для событий, зарегистрированных для способностей)
     * 
     * @return При возвращении CWAPI_STOP_MAIN выстрел будет заблокирован.
     */
    CWeapon_OnPrimaryAttackPre,

    /**
     * Вызывается после выстрела.
     * 
     * (const T_CustomWeapon:weapon, const item, const Trie:p = Invalid_Trie)
     * 
     * @param weapon Хендлер кастомного оружия
     * @param item Индекс сущности предмета оружия
     * @param p Параметры способности (только для событий, зарегистрированных для способностей)
     * 
     * @noreturn Возвращаемое значение игнорируется.
     */
    CWeapon_OnPrimaryAttackPost,

    /**
     * Вызывается при касании weaponbox'а игроком.
     * 
     * (const T_CustomWeapon:weapon, const weaponBox, const item, const playerIndex, const Trie:p = Invalid_Trie)
     * 
     * @param weapon Хендлер кастомного оружия
     * @param weaponBox Индекс сущности weaponbox'a
     * @param item Индекс сущности предмета оружия
     * @param playerIndex Индекс игрока
     * @param p Параметры способности (только для событий, зарегистрированных для способностей)
     * 
     * @return При возвращении CWAPI_STOP_MAIN касание будет заблокировано.
     */
    CWeapon_OnPlayerTouchWeaponBox,

    /**
     * Вызывается при броске гранаты игроком.
     * 
     * (const T_CustomWeapon:weapon, const item, const playerIndex, Float:vecSrc[3], Float:vecThrow[3], &Float:time, const usEvent, const Trie:p = Invalid_Trie)
     * 
     * @param weapon Хендлер кастомного оружия
     * @param item Индекс сущности предмета оружия
     * @param playerIndex Индекс игрока
     * @param vecSrc Начальная точка полёта гранаты
     * @param vecThrow // TODO
     * @param time Время до взрыва гранаты
     * @param usEvent // TODO
     * @param p Параметры способности (только для событий, зарегистрированных для способностей)
     * 
     * @return При возвращении CWAPI_STOP_MAIN бросок будет заблокирован.
     */
    CWeapon_OnPlayerThrowGrenade,
}

/**
 * Публиные параметры кастомного оружия
 * 
 * @see CWAPI_Weapons_GetAttribute
 * @see CWAPI_Weapons_iGetAttribute
 */
enum E_CWeapon_Attribute {
    /**
     * Название стандартного оружия, на котором основано кастомное
     * 
     * Тип: String
     * Только для чтения
     */
    CWeaponAttr_Reference,

    /**
     * Название кастомного оружия
     * 
     * Тип: String
     * Только для чтения
     */
    CWeaponAttr_Name,

    /**
     * Множитель урона.
     * 
     * Тип: Float
     */
    CWeaponAttr_DamageMult,

    /**
     * Индекс стандартного оружия, на котором основано кастомное.
     * 
     * Тип: Integer
     * Только для чтения
     */
    CWeaponAttr_ReferenceId,

    /**
     * Значимость оружия.
     * 
     * Тип: Integer
     */
    CWeaponAttr_Weight,

    /**
     * Размер магазина.
     * 
     * Тип: Integer
     */
    CWeaponAttr_MaxClip,

    /**
     * Максимальное кол-во запасных патронов.
     * 
     * Тип: Integer
     */
    CWeaponAttr_MaxAmmo,

    /**
     * Время доставания.
     * 
     * Тип: Float
     */
    CWeaponAttr_DeployTime,

    /**
     * Время перезарядки.
     * 
     * Тип: Float
     */
    CWeaponAttr_ReloadTime,

    /**
     * Максимальная скорость ходьбы с оружием в руках.
     * 
     * Тип: Float
     */
    CWeaponAttr_MaxWalkSpeed,
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
 * @param weaponName   Название кастомного оружия.
 * 
 * @return              Хендлер кастомного оружия или Invalid_CustomWeapon в случае неудачи.
 */
native T_CustomWeapon:CWAPI_Weapons_Find(const weaponName[]);

/**
 * Поиск кастомного оружия по сущности предмета.
 * 
 * @param item    Индекс сущности предмета.
 * 
 * @return          Хендлер кастомного оружия или Invalid_CustomWeapon в случае неудачи.
 */
native T_CustomWeapon:CWAPI_Weapons_FindByItem(const item);

/**
 * Проверка, является ли предмет кастомным оружием.
 * 
 * @param item    Индекс сущности предмета.
 * 
 * @return          true, если предмет является кастомным оружием, иначе false.
 */
stock bool:CWAPI_Weapons_IsCustomItem(const item) {
    return CWAPI_Weapons_FindByItem(item) != Invalid_CustomWeapon;
}

/**
 * Регистрация обработчика события кастомного оружия.
 * 
 * @param weapon   Хендлер кастомного оружия.
 * @param event    Событие.
 * @param callback Название функции-обработчика.
 * 
 * @noreturn
 */
native CWAPI_Weapons_AddEventListener(const T_CustomWeapon:weapon, const E_CWeapon_Event:event, const callback[]);

/**
 * Выдача кастомного оружия игроку.
 * 
 * @param playerIndex        Индекс игрока.
 * @param weapon       Хендлер кастомного оружия.
 * @param giveType     Метод выдачи оружия (см. enum CWeapon_GiveType).
 * @param forceActive  Сделать выданное оружие активным.
 * @param initBpAmmo    Сколько патронов выдавать вместе с оружием. -1 - пополнять до максимума (MaxAmmo в кфг оружия или соответствующее дефолтное значение).
 * 
 * @return          Индекс созданной ентити предмета. -1, если оружие по какой-то причине не было выдано.
 */
native CWAPI_Weapons_Give(
    const playerIndex,
    const T_CustomWeapon:weapon,
    const CWeapon_GiveType:giveType = CWAPI_GT_SMART,
    const bool:forceActive = false,
    const initBpAmmo = -1
);

/**
 * Выдача игроку кастомного оружия по его названию.
 * 
 * @param playerIndex        Индекс игрока.
 * @param weaponName   Название кастомного оружия.
 * @param giveType     Метод выдачи оружия (см. enum CWeapon_GiveType).
 * @param forceActive  Сделать выданное оружие активным.
 * 
 * @return              Индекс созданной ентити предмета. -1, если оружие по какой-то причине не было выдано.
 */
stock CWAPI_Weapons_GiveByName(
    const playerIndex,
    const weaponName[],
    const CWeapon_GiveType:giveType = CWAPI_GT_SMART,
    const bool:forceActive = false
) {
    new T_CustomWeapon:weapon = CWAPI_Weapons_Find(weaponName);
    if (weapon == Invalid_CustomWeapon) {
        return -1;
    }

    return CWAPI_Weapons_Give(playerIndex, weapon, giveType, forceActive);
}

/**
 * Получение названия кастомного оружия.
 * 
 * @param weapon   Хендлер кастомного оружия.
 * @param out      Буфер для записи названия кастомного оружия.
 * @param outLen   Размер буфера.
 * 
 * @return          Количество записанных в буфер ячеек.
 */
native CWAPI_Weapons_GetName(const T_CustomWeapon:weapon, out[], const outLen);

/**
 * Получение названия кастомного оружия.
 * 
 * @param weapon   Хендлер кастомного оружия.
 * 
 * @return          Название кастомного оружия.
 */
stock CWAPI_Weapons_iGetName(const T_CustomWeapon:weapon) {
    new weaponName[CWAPI_WEAPON_NAME_MAX_LEN];
    CWAPI_Weapons_GetName(weapon, weaponName, charsmax(weaponName));

    return weaponName;
}

/**
 * Получение атрибута кастомного оружия.
 * 
 * @param weapon       Хендлер кастомного оружия.
 * @param attribute    Атрибут (см. enum E_CWeapon_Attribute).
 * 
 * @note                Для строковых атрибутов:
 *                      CWAPI_Weapons_GetAttribute(weapon, attribute, out, outLen);
 *                      - out - буфер для записи атрибута;
 *                      - outLen - размер буфера.
 *                      - Вернёт количество записанных в буфер ячеек.
 *  
 * @note                Для числовых атрибутов:
 *                      CWAPI_Weapons_GetAttribute(weapon, attribute);
 *                      - Вернёт значение атрибута.
 * 
 * @return              Количество записанных в буфер ячеек.
 */
native any:CWAPI_Weapons_GetAttribute(const T_CustomWeapon:weapon, const E_CWeapon_Attribute:attribute, any:...);

// TODO: CWAPI_Weapons_SetAttribute

/**
 * Получение названия кастомного оружия.
 * 
 * @param weapon       Хендлер кастомного оружия.
 * @param attribute    Ключ атрибута (см. enum E_CWeapon_Attribute).
 * 
 * @note                Только для строковых атрибутов.
 * 
 * @return              Значение атрибута.
 */
stock CWAPI_Weapons_iGetAttribute(const T_CustomWeapon:weapon, const E_CWeapon_Attribute:attribute) {
    new sAttrValue[MAX_FMT_LENGTH];
    CWAPI_Weapons_GetAttribute(weapon, attribute, sAttrValue, charsmax(sAttrValue));

    return sAttrValue;
}

/**
 * Нанесение урона через кастомное оружие.
 * 
 * @param weaponIndex       Индекс кастомного оружия.
 * @param victimIndex       Индекс жертвы, получающей урон.
 * @param inflictorIndex    Индекс сущности, которая наносит урон. Например, летящая гранта.
 * @param attackerIndex     Индекс атакующего игрока. Например, того, кто кинул гранату.
 * @param damage            Значение наносимого урона. Может быть изменено в 
 * @param damageBits        Тип наносимого урона.
 * @param itemIndex         Индекс предмета оружия. Если урон нанесён не напрямую оружием (inflictorIndex != attackerIndex), то можно передавать ноль.
 * @param ignoreDamageMult  Игнорировать ли атрибут "DamageMult" из кфг оружия.
 * 
 * @note    Вызывает событие CWeapon_OnDamage.
 * @note    Урон может быть заблокирован способностью. В таком случае натив вернёт ноль.
 * @note    Реализовано через ExecuteHamB, поэтому вызывает другие хуки.
 * 
 * @return  Итоговое значение урона, после применения DamageMult (если не игнорируется) и способностей.
 */
native Float:CWAPI_Weapons_EmitDamage(
    const T_CustomWeapon:weaponIndex,
    const victimIndex,
    const inflictorIndex,
    const attackerIndex,
    const Float:damage,
    const damageBits = DMG_GENERIC,
    const itemIndex = 0,
    const bool:ignoreDamageMult = false
);

/**
 * Получение параметров указанной способности у указанного кастомного оружия.
 * 
 * @param weaponIndex   Хендлер кастомного оружия.
 * @param ability       Хендлер способности.
 * 
 * @return Trie с параметрами, либо Invalid_Trie, если у указанного оружия нет указанной способности.
 */
native Trie:CWAPI_Weapons_GetAbilityParams(const T_CustomWeapon:weaponIndex, const T_WeaponAbility:ability);

stock Trie:CWAPI_Weapons_GetAbilityParamsOrInvalid(const T_CustomWeapon:weaponIndex, const T_WeaponAbility:ability) {
    if (weaponIndex == Invalid_CustomWeapon || ability == Invalid_WeaponAbility) {
        return Invalid_Trie;
    }

    return CWAPI_Weapons_GetAbilityParams(weaponIndex, ability);
}

stock Trie:CWAPI_Weapons_GetAbilityParamsByItem(const item, const T_WeaponAbility:ability) {
    return CWAPI_Weapons_GetAbilityParamsOrInvalid(CWAPI_Weapons_FindByItem(item), ability);
}

stock Trie:CWAPI_Weapons_GetAbilityParamsByNames(const weaponName[], const abilityName[]) {
    return CWAPI_Weapons_GetAbilityParamsOrInvalid(CWAPI_Weapons_Find(weaponName), CWAPI_Abilities_Find(abilityName));
}

stock Trie:CWAPI_Weapons_GetAbilityParamsByItemAndName(const item, const abilityName[]) {
    return CWAPI_Weapons_GetAbilityParamsOrInvalid(CWAPI_Weapons_FindByItem(item), CWAPI_Abilities_Find(abilityName));
}

/**
 * Регистрация способности кастомного оружия.
 * 
 * @param abilityName  Название способности.
 * 
 * @return              Хендлер зарегистрированной способности.
 */
native T_WeaponAbility:CWAPI_Abilities_Register(const abilityName[]);

/**
 * Добавление параметров для способности кастомного оружия.
 * 
 * @param ability  Хендлер способности.
 * @param any:...   Перечисление параметров.
 * 
 * @note            Формат перечисления параметров:
 *                      CWAPI_Abilities_AddParams(..., const paramName[], const paramType[], const bool:paramRequired, ...);
 * 
 * @note            paramName - Название параметра;
 *                  paramType - Название типа параметра;
 *                  paramRequired - Обязателен ли параметр.
 * 
 * @note            Пример:
 *                      CWAPI_Abilities_AddParams(ability,
 *                          "Param1", "String", true,
 *                          "Param2", "Integer", false
 *                      );
 * 
 * @noreturn
 */
native CWAPI_Abilities_AddParams(const T_WeaponAbility:ability, any:... /* Params */);

/**
 * Добавление обработчика события кастомного оружия для способности.
 * 
 * @param ability  Хендлер способности.
 * @param event    Элемент перечисления события.
 * @param callback Название функции-обработчика.
 * 
 * @noreturn
 */
native CWAPI_Abilities_AddEventListener(const T_WeaponAbility:ability, const E_CWeapon_Event:event, const callback[]);

/**
 * Поиск хендлера способности по её названию.
 * 
 * @param abilityName   Название способности.
 * 
 * @return  Хендлер способности. Invalid_WeaponAbility, если способность с таким названием не зарегистрирована.
 */
native T_WeaponAbility:CWAPI_Abilities_Find(const abilityName[]);
