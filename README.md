# Custom Weapons API

## Описание

API для создания кастомного оружия

## Настройки оружий [_configs/plugins/CustomWeaponAPI/Weapons.json_]

### Структура

```js
[
    {
        "DefaultName": "Название дефолтного оружие, на котором будет основано кастомное",
        "Name": "Название кастомного оружия (Желательно без пробелов и спецсимволов)",
        "ClipSize": [Int] Максимальное кол-во патронов в обойме,
        "MaxAmmo": [Int] Общее кол-во патронов,
        "Models": {
            "v": "v_ модель оружия (Опционально)",
            "p": "p_ модель оружия (Опционально)",
            "w": "w_ модель оружия (Опционально)"
        },
        "Sounds": {
            "Shot": "Звук выстрела",
            "ShotSilent": "Звук выстрела с глушителем (Только для M4A1 и USP-S)",
            "OnlyPrecache": [
                "Звуковой файл используемый самой моделькой оружия",
                "..."
            ]
        },
        "MaxWalkSpeed": [Int] Скорость бега с оружием в руках,
        "DamageMult": [Float] Множитель урона,
        "Weight": [Int] Вес оружия,
        "Price": [Int] Цена оружия (Если не указать то купить нельзя будет)
    },
    {...}
]
```

### Пример

```json
[
    {
        "DefaultName": "m4a1",
        "Name": "NoveskeDiplomat",
        "ClipSize": 35,
        "MaxAmmo": 150,
        "Models": {
            "v": "models/CustomWeapons/Noveske Diplomat/v_m4a1.mdl",
            "p": "models/CustomWeapons/Noveske Diplomat/p_m4a1.mdl",
            "w": "models/CustomWeapons/Noveske Diplomat/w_m4a1.mdl"
        },
        "Sounds": {
            "Shot": "CustomWeapons/Noveske Diplomat/m4a1_unsil-1.wav",
            "ShotSilent": "CustomWeapons/Noveske Diplomat/m4a1-1.wav",
            "OnlyPrecache": [
                "weapons/M4A1/Ripper/boltback.wav",
                "weapons/M4A1/Ripper/boltrelease.wav",
                "weapons/M4A1/Ripper/bullet.wav",
                "weapons/M4A1/Ripper/draw.wav",
                "weapons/M4A1/Ripper/inspect.wav",
                "weapons/M4A1/Ripper/magin.wav",
                "weapons/M4A1/Ripper/magout.wav",
                "weapons/M4A1/Ripper/magtap.wav",
                "weapons/M4A1/Ripper/siloff.wav",
                "weapons/M4A1/Ripper/silon.wav",
                "weapons/M4A1/Ripper/silpush.wav",
                "weapons/M4A1/Ripper/maghit.wav"
            ]
        },
        "MaxWalkSpeed": 800,
        "DamageMult": 1.1,
        "Weight": 100,
        "Price": 6000
    }
]
```

## API

```cpp
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
 * Вызывается после загрузки всех пушек из конфига
 *
 * @noreturn
 */
forward CWAPI_LoawWeaponsPost();
```

### [Пример использования API](https://github.com/ArKaNeMaN/amxx-CustomWeaponsAPI/blob/master/CWAPI_Example_FireDeagle.sma)

## Благодарность
[Dev-CS: [ReAPI] Пример кастомного оружия с дополнительними свойствами](https://dev-cs.ru/threads/1983/)

[Noveske Diplomat WAR-custom - Ripper](https://dev-cs.ru/resources/805/) - использовал эту модель для тестов