# Netlink Kernel Module 
## Описание проекта

Данный проект реализует обмен сообщениями между user space и kernel space через механизм Netlink sockets.

Пользовательское приложение отправляет текстовое сообщение в ядро, ядро принимает его, обрабатывает и отправляет ответ "OK"обратно.
---
## Структура проекта

- `netlink_kernel.c` — kernel module (ядро Linux)
- `user_app.c` — userspace приложение
- `Makefile` — сборка kernel module
---
## Как работает

1. Пользователь запускает `user_app`
2. Вводит сообщение (до 9–10 символов)
3. Сообщение отправляется в kernel через Netlink
4. Kernel принимает сообщение в callback `recv_message`
5. Kernel формирует ответ `"OK"`
6. Ответ отправляется обратно пользователю через `nlmsg_unicast`
7. Пользователь получает ответ через `recvmsg`

---
## Сборка
make

---
## Загрузка модуля
- sudo insmod netlink_kernel.ko

---
##Проверка загрузки:
- dmesg | tail

---
## Выгрузка модуля
- sudo rmmod netlink_kernel
