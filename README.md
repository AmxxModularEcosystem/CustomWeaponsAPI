<img height=64 src="https://github.com/ArKaNeMaN/amxx-CustomWeaponsAPI/blob/master/.github/IconBar-96.png?raw=true" alt="CWAPI-IconBar"/>

## Описание

_В процессе реинжиниринга!_ Стабильная версия в ветке `0.7.x` и в релизах.

Создание кастомного оружия через конфиг-файлы с возможностью расширения способностей оружия при помощи API.

[Ресурс на Dev-CS](https://dev-cs.ru/resources/852/)

## Зависимости

- [Params Controller](https://github.com/AmxxModularEcosystem/ParamsController) v1.0.0-b4 или новее

## Миграция с версии 0.7.x на версию 0.8.0

[Мигратор конфигов оружия...](https://amxxmodularecosystem.github.io/vue-cwapi-weapons-migrator/)

Все расширения для версии 0.7.x несовместимы с версией 0.8.0.

В файлах кастомных оружий были переименованы некоторые поля:

- Поле `DefaultName` переименовано в `Reference`. Также, в его значении теперь должно быть указано полное название стандартного оружия, включая `weapon_`.
- Поле `ClipSize` переименовано в `MaxClip`.
- Подполя `v`, `p` и `w` поля `Models` переименованы в `View`, `Player` и `World` соответственно.

### Пример миграции файла оружия

0.7.x:

```jsonc
{
  "DefaultName": "deagle",
  "Models": {
    "v": "models/v_deagle.mdl",
    "p": "models/p_deagle.mdl",
    "w": "models/w_deagle.mdl"
  },
  "ClipSize": 10
}
```

0.8.0:

```jsonc
{
  "Reference": "weapon_deagle",
  "Models": {
    "View": "models/v_deagle.mdl",
    "Player": "models/p_deagle.mdl",
    "World": "models/w_deagle.mdl"
  },
  "MaxClip": 10
}
```

## Требования

- [AmxModX 1.9.0](https://www.amxmodx.org/downloads-new.php)
- [ReAPI 5.15.x или новее](https://github.com/s1lentq/reapi/releases/latest)
- [Fix Custom iMaxClip](https://goldsrc.ru/threads/4165/)
  - Необязательно. Исправляет баг с анимацией перезарядки при полном магазине.

## Благодарность

[Dev-CS: [ReAPI] Пример кастомного оружия с дополнительними свойствами](https://dev-cs.ru/threads/1983/)

[Dev-CS: За помощь на форуме](https://dev-cs.ru/threads/7718/)
