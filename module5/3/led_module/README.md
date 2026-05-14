#Linux Kernel Module: Keyboard LED Control via sysfs and ioctl
## Описание задачи

Проект представляет собой Linux kernel module, который реализует управление клавиатурными LED-индикаторами (Caps Lock, Num Lock, Scroll Lock) через интерфейс sysfs.

Передача данных из userspace в kernel space осуществляется через запись в файл /sys/kernel/myleds/leds, после чего значение обрабатывается в модуле ядра и применяется через системный вызов ioctl (KDSETLED).

##Принцип работы
Пользователь записывает число в sysfs файл:
- echo 4 | sudo tee /sys/kernel/myleds/leds
Значение попадает в функцию leds_store():
преобразуется в int
сохраняется в переменную led_mask
Вызывается функция управления LED:
используется vc_cons[fg_console].d->port.tty->driver
выполняется ioctl:
KDSETLED
Ядро включает соответствующие LED.

##Примеры использования

Включить Caps Lock:

- echo 4 | sudo tee /sys/kernel/myleds/leds

Включить Num Lock:

- echo 2 | sudo tee /sys/kernel/myleds/leds

Включить все LED:

- echo 7 | sudo tee /sys/kernel/myleds/leds

- Выключить все LED:

echo 0 | sudo tee /sys/kernel/myleds/leds
##Сборка модуля
- make
#Загрузка модуля
- sudo insmod myleds.ko
#Выгрузка модуля
- sudo rmmod myleds
#Проверка загрузки
- dmesg | tail
- ls /sys/kernel/myleds/
