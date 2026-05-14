#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/kobject.h>      // kobject_create_and_add
#include <linux/sysfs.h>        // sysfs_create_file
#include <linux/fs.h>

#include <linux/tty.h>          // tty структуры
#include <linux/vt_kern.h>      // vc_cons, fg_console
#include <linux/kd.h>           // KDSETLED

static struct kobject *my_kobj;
static int led_mask = 0;//переменная хранения маски LED из sysfs

//(из kbleds.c) функция управления LED
  
static void update_leds(int mask)
{
    struct tty_driver *driver;

    driver = vc_cons[fg_console].d->port.tty->driver;//олучаем драйвер текущей активной консоли

    driver->ops->ioctl(vc_cons[fg_console].d->port.tty,KDSETLED,mask);// вызываем ioctl KDSETLED
}

//из sysfs, отвечает за чтение файла /sys/kernel/myleds/leds)
   
static ssize_t leds_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", led_mask); //текущее значение маски
}

//из sysfs, отвечает за запись файла

static ssize_t leds_store(struct kobject *kobj, struct kobj_attribute *attr,const char *buf,size_t count)
{
    int value;

    //перевод строки в число 
    if (kstrtoint(buf, 10, &value))
    return -EINVAL;
    led_mask = value;

   // вызов ioctl (из kbleds.c)
    update_leds(led_mask);

    return count;
}

//описание sysfs файла
static struct kobj_attribute leds_attr = __ATTR(leds, 0660, leds_show, leds_store);

//Инициализация модуля
static int __init my_init(void)
{
     int error = 0;
 
	pr_debug("Module initialized successfully \n");

    // создаём /sys/kernel/myleds
    my_kobj = kobject_create_and_add("myleds", kernel_kobj);

    if (!my_kobj)
        return -ENOMEM;

    /* создаём файл leds */
    error = sysfs_create_file(my_kobj, &leds_attr.attr);

    if (error)
        pr_debug("failed to create the foo file in /sys/kernel/systest \n");

    return error;
}

//exit модуля
static void __exit my_exit(void)
{
    pr_debug ("Module un initialized successfully \n");

    kobject_put(my_kobj);
}

MODULE_LICENSE("GPL");

module_init(my_init);
module_exit(my_exit);
