obj-m += netlink_kernel.o

KDIR := /lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)

all:
	# Сборка kernel module
	$(MAKE) -C $(KDIR) M=$(PWD) modules

	# Сборка userspace программы
	gcc -Wall -Wextra -std=c11 user_app.c -o user_app

clean:
	# Очистка файлов kernel build
	$(MAKE) -C $(KDIR) M=$(PWD) clean

	# Удаление userspace бинарника
	rm -f user_app
