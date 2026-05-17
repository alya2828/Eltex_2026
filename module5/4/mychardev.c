#include <linux/module.h>      
#include <linux/kernel.h>      
#include <linux/fs.h>  // file_operations
#include <linux/cdev.h> // cdev
#include <linux/uaccess.h>    
#include <linux/device.h>      

#define DEVICE_NAME "mychardev"
#define BUFFER_SIZE 1024

// буфер в ядре
static char kernel_buffer[BUFFER_SIZE];  // сюда пишем данные из userspace
static size_t buffer_size = 0;           // текущий размер данных

// device number
static dev_t dev_number;     // major, minor
static struct cdev my_cdev;  // структура character device

//class/device для /dev
static struct class *my_class;
static struct device *my_device;

//open 
static int dev_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "mychardev: device opened\n");
    return 0;
}

//release
static int dev_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "mychardev: device closed\n");
    return 0;
}

//read
static ssize_t dev_read(struct file *file, char __user *user_buf,
                        size_t len, loff_t *offset)
{
    // если offset больше данных — ничего не читаем
    if (*offset >= buffer_size)
        return 0;

    // сколько реально можем отдать
    size_t to_copy = buffer_size - *offset;
    if (to_copy > len)
        to_copy = len;

    // копируем из kernel space в user space
    if (copy_to_user(user_buf, kernel_buffer + *offset, to_copy))
        return -EFAULT;

    *offset += to_copy;

    printk(KERN_INFO "mychardev: read %zu bytes\n", to_copy);

    return to_copy;
}

//write
static ssize_t dev_write(struct file *file, const char __user *user_buf,
                         size_t len, loff_t *offset)
{
    // ограничиваем размер
    if (len > BUFFER_SIZE)
        len = BUFFER_SIZE;

    // копируем из user space в kernel space
    if (copy_from_user(kernel_buffer, user_buf, len))
        return -EFAULT;

    buffer_size = len;
    kernel_buffer[len] = '\0'; // чтобы можно было печатать как строку

    printk(KERN_INFO "mychardev: received from user: %s\n", kernel_buffer);

    return len;
}

// file_operations
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release,
};

// init
static int __init mychardev_init(void)
{
    printk(KERN_INFO "mychardev: initializing...\n");

    // 1. получить major/minor динамически
    if (alloc_chrdev_region(&dev_number, 0, 1, DEVICE_NAME) < 0) {
        printk(KERN_ERR "mychardev: failed to allocate major number\n");
        return -1;
    }

    printk(KERN_INFO "mychardev: major=%d minor=%d\n",
           MAJOR(dev_number), MINOR(dev_number));

    // 2. инициализация cdev
    cdev_init(&my_cdev, &fops);
    my_cdev.owner = THIS_MODULE;

    if (cdev_add(&my_cdev, dev_number, 1) < 0) {
        printk(KERN_ERR "mychardev: cannot add cdev\n");
        unregister_chrdev_region(dev_number, 1);
        return -1;
    }

    // 3. создаём класс (/sys/class)
    my_class = class_create(DEVICE_NAME);

    // 4. создаём device (/dev/mychardev)
    my_device = device_create(my_class, NULL, dev_number, NULL, DEVICE_NAME);

    printk(KERN_INFO "mychardev: device created at /dev/%s\n", DEVICE_NAME);

    return 0;
}

// exit 
static void __exit mychardev_exit(void)
{
    // удаляем /dev/mychardev
    device_destroy(my_class, dev_number);

    // удаляем класс
    class_destroy(my_class);

    // удаляем cdev
    cdev_del(&my_cdev);

    // освобождаем номер устройства
    unregister_chrdev_region(dev_number, 1);

    printk(KERN_INFO "mychardev: module unloaded\n");
}

module_init(mychardev_init);
module_exit(mychardev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alina");
