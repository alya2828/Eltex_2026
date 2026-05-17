# Character Device Kernel Module
## Описание проекта

Данный проект представляет собой простой модуль ядра Linux, реализующий символьное устройство (character device), обеспечивающее обмен данными между userspace и kernel space через файл устройства `/dev/mychardev`.
Модуль разработан для ядра Linux версии **6.8.0-111-generic**.
---
## Функциональность

Модуль реализует:

- создание символьного устройства `/dev/mychardev`
- запись данных из userspace в kernel space
- чтение данных из kernel space в userspace
- обработку базовых операций:
  - open
  - read
  - write
  - release
---
###Сборка модуля
- make
##Загрузка модуля
- sudo insmod mychardev.ko
##Проверка загрузки:
- dmesg | tail
##Устройство
После загрузки модуля создаётся символьное устройство:

/dev/mychardev

##Запись данных
- echo "Hello Kernel" > /dev/mychardev
##Чтение данных
- cat /dev/mychardev
##Выгрузка модуля
- sudo rmmod mychardev

