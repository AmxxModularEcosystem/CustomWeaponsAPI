# Custom Weapons API

## Описание

API для создания кастомного оружия

## Требования

- [AmxModX 1.9.0](https://www.amxmodx.org/downloads-new.php)
- [ReAPI 5.8.0.163 или новее](http://teamcity.rehlds.org/project.html?projectId=Reapi)

## Настройки оружий [_configs/plugins/CustomWeaponAPI/Weapons.json_]

### Структура
<details>
    <summary>Спойлер</summary>

    ```js
    [
        {
            "DefaultName": [String] Название дефолтного оружие, на котором будет основано кастомное,
            "Name": [String] Название кастомного оружия (Желательно без пробелов и спецсимволов),
            "ClipSize": [Int] Максимальное кол-во патронов в обойме,
            "MaxAmmo": [Int] Общее кол-во патронов,
            "Models": {
                "v": [String] v_ модель оружия (Опционально),
                "p": [String] p_ модель оружия (Опционально),
                "w": [String] w_ модель оружия (Опционально)
            },
            "Sounds": {
                "Shot": [String] Звук выстрела,
                "ShotSilent": [String] Звук выстрела с глушителем (Только для M4A1 и USP-S),
                "OnlyPrecache": [
                    [String] Звуковой файл используемый самой моделькой оружия,
                    "..."
                ]
            },
            "MaxWalkSpeed": [Int] Скорость бега с оружием в руках,
            "DamageMult": [Float] Множитель урона,
            "DeployTime": [Float] Длительность доставания оружия,
            "PrimaryAttackRate": [Float] Интервал между первичными атаками,
            "SecondaryAttackRate": [Float] Интервал между вторичными атаками (Например, снятие\надевание глушителя),
            "HasSecondaryAttack": [Bool] Есть ли у оружия вторичная атака*,
            "Weight": [Int] Вес оружия,
            "Price": [Int] Цена оружия (Если не указать то купить нельзя будет)
        },
        {...}
    ]
    ```
    *Если она есть изначально, то отключить её нельзя.
</details>

### Пример
<details>
    <summary>Спойлер</summary>

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
</details>

## Команды

### CWAPI_Buy <WeaponName>
Покупка кастомного оружия

Пример: `CWAPI_Buy NoveskeDiplomat`

### CWAPI_Give <WeaponName>
Выдача себе кастомного оружия

Пример: `CWAPI_Give NoveskeDiplomat`

Работает, только если плагин скомпилирован с дефайном `DEBUG`

## [API](https://github.com/ArKaNeMaN/amxx-CustomWeaponsAPI/blob/master/include/cwapi.inc)

### [Пример использования API](https://github.com/ArKaNeMaN/amxx-CustomWeaponsAPI/blob/master/CWAPI_Example_FireDeagle.sma)

## Благодарность
[Dev-CS: [ReAPI] Пример кастомного оружия с дополнительними свойствами](https://dev-cs.ru/threads/1983/)

[Noveske Diplomat WAR-custom - Ripper](https://dev-cs.ru/resources/805/) - использовал эту модель для тестов