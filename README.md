# Custom Weapons API

## Описание

API для создания кастомного оружия

[Ресурс на Dev-CS](https://dev-cs.ru/resources/852/)

## Требования

- [AmxModX 1.9.0](https://www.amxmodx.org/downloads-new.php)
- [ReAPI 5.8.0.163 или новее](http://teamcity.rehlds.org/project.html?projectId=Reapi)
- [Fix Custom iMaxClip](https://goldsrc.ru/threads/4165/)
  - Необязательно. Исправляет баг с анимацией перезарядки при полном магазине.

## Настройки оружий [`/amxmodx/configs/plugins/CustomWeaponAPI/Weapons/<WeaponName>.json`]

### Структура
<details>
    <summary>Спойлер</summary>

```js
{
    "DefaultName": [String] Название дефолтного оружие, на котором будет основано кастомное,
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
    "ClipSize": [Int] Максимальное кол-во патронов в обойме,
    "MaxAmmo": [Int] Общее кол-во патронов,
    "DamageMult": [Float] Множитель урона,
    "Damage": [Float] Базовый урон,
    "Accuracy": [Float] Точность (До конца не уверен работает ли),
    "Weight": [Int] Вес оружия,
    "Price": [Int] Цена оружия (Если не указать то купить нельзя будет),
    "DeployTime": [Float] Длительность доставания оружия,
    "ReloadTime": [Float] Длительность перезарядки (Для дробовика время докидывания одного патрона),
    "PrimaryAttackRate": [Float] Интервал между первичными атаками,
    "HasSecondaryAttack": [Bool] Есть ли у оружия вторичная атака*,
    "SecondaryAttackRate": [Float] Интервал между вторичными атаками (Например, снятие\надевание глушителя)
}
```
*Если она есть изначально, то отключить её нельзя.
</details>

### Пример
<details>
    <summary>Спойлер</summary>

`/amxmodx/configs/plugins/CustomWeaponAPI/Weapons/AugA3War.json`:

```json
{
    "DefaultName": "aug",
    "ClipSize": 40,
    "MaxAmmo": 240,
    "Models": {
        "v": "models/CustomWeapons/AugA3War/v_aug.mdl",
        "p": "models/CustomWeapons/AugA3War/p_aug.mdl",
        "w": "models/CustomWeapons/AugA3War/w_aug.mdl"
    },
    "Sounds": {
        "Shot": "CustomWeapons/AugA3War/aug-1.wav",
        "OnlyPrecache": [
            "weapons/AUG/Mercury/bolt.wav",
            "weapons/AUG/Mercury/boltback.wav",
            "weapons/AUG/Mercury/boltrelease.wav",
            "weapons/AUG/Mercury/bullet.wav",
            "weapons/AUG/Mercury/draw.wav",
            "weapons/AUG/Mercury/magin.wav",
            "weapons/AUG/Mercury/magout.wav",
            "weapons/AUG/Mercury/magtap.wav"
        ]
    },
    "MaxWalkSpeed": 800,
    "DamageMult": 1.15,
    "Weight": 120,
    "Price": 7500,
    "DeployTime": 1.6,
    "PrimaryAttackRate": 0.12,
    "SecondaryAttackRate": 0.5
}
```
</details>

## Команды

### CWAPI_Buy \<WeaponName\>
* Покупка кастомного оружия
* Пример: `CWAPI_Buy NoveskeDiplomat`

### CWAPI_Give \<WeaponName\>
* Выдача себе кастомного оружия
* Пример: `CWAPI_Give NoveskeDiplomat`
* _Работает, только если плагин скомпилирован с дефайном `DEBUG`_

## [API](https://github.com/ArKaNeMaN/amxx-CustomWeaponsAPI/blob/master/include/cwapi.inc)

### [Пример использования API](https://github.com/ArKaNeMaN/amxx-CustomWeaponsAPI/blob/master/CWAPI_Example_FireDeagle.sma)

## Благодарность
[Dev-CS: [ReAPI] Пример кастомного оружия с дополнительними свойствами](https://dev-cs.ru/threads/1983/)

[Noveske Diplomat WAR-custom - Ripper](https://dev-cs.ru/resources/805/) - использовал эту модель для тестов