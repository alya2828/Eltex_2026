#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define PROC_NAME "hello" 
#define BUF_SIZE 10
 
static int len,temp;//из глобальной в статику для доступа только внутри файла модуля
static char *msg;
static struct proc_dir_entry *entry;// структура ядра, entry хранит указатель на созданный /proc файл,чтобы потом удалить его через proc_remove()
 
static ssize_t read_proc(struct file *filp, char __user *buf, size_t count, loff_t *offp ) {//static в начале забыли и __user чтобы указать что это пользовательское пространство
    if (*offp > 0 || temp == 0)
        return 0;//добавим проверку, чтобы ядро не вызывала много раз read файла, offp - позиция чтения файла
        
    if(count > temp) {
        count = temp;
    }
    temp = temp - count;
    
    if (copy_to_user(buf, msg, count)) // тут нужна проверка, скопировались ли данные верно
    return -EFAULT;
    
    if(count == 0)
        temp = len;
    return count;
}
 
static ssize_t write_proc(struct file *file, const char __user*buf, size_t count, loff_t *offp) {//static в начале забыли и __user
	
	if (count > BUF_SIZE - 1){//защита на переполнение буфера нужно место под '/0'
        count = BUF_SIZE - 1;
       }
    if (copy_from_user(msg, buf, count))
    return -EFAULT;//ошибка доступа памяти 
    msg[count] = '\0';//добавим конец строки
    len = count;
    temp = len;
    return count;
}
 
static const struct proc_ops proc_fops = {
    .proc_read = read_proc,// . и = новый стиль инициализации структуры
    .proc_write = write_proc,
};
//Инициализация модуля
 static int __init proc_init (void) {//static забыли и __init
   msg = kmalloc(BUF_SIZE * sizeof(char), GFP_KERNEL);// убрали магическое число 10
   entry = proc_create(PROC_NAME, 0666, NULL, &proc_fops); // DEFINE и делаем файл доступным на чтение и запись, а entry указвает на созданный файл;
     printk(KERN_INFO "proc module loaded\n");
    return 0;
}
 
static void __exit proc_cleanup(void) {//забыли static и нужно добавить __exit при выгрузке модуля
    proc_remove(entry);// теперь тут указатель entry; в версии 6.8 функция proc_remove
    kfree(msg);
}
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Aina");
module_init(proc_init);
module_exit(proc_cleanup);
