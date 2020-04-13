#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/device.h>
//#include <sys/types.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define HELLO_MAJOR 100
#define HELLO_NAME  "hello_dev"
#define HELLO_DEVICE_NAME "hello_node"

static char read_buf[100];
static char write_buf[100];
static char kernel_data[] = {"hello data!"};
static struct class *hello_class;
static struct cdev hello_dev;
static dev_t dev_id;

static int hello_open(struct inode *inode, struct file *filp)
{
    printk("%s: open...\n", __func__);
    return 0;
}

/*
 * @description : 从设备读取数据
 * @param - filp : 要打开的设备文件(文件描述符)
 * @param - buf : 返回给用户空间的数据缓冲区
 * @param - cnt : 要读取的数据长度
 * @param - offt : 相对于文件首地址的偏移
 * @return : 读取的字节数，如果为负值，表示读取失败
*/
static ssize_t hello_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
    int ret = 0;
    
    memcpy(read_buf, kernel_data, sizeof(kernel_data));
    ret = copy_to_user(buf, read_buf, cnt);
    if (ret == 0) {
        printk("read buffer successful\n");
    } else {
        printk("read buffer failed\n");
    }

    printk("%s: read...\n", __func__);
    return ret;
}

/*
 * @description : 向设备写数据
 * @param - filp : 设备文件，表示打开的文件描述符
 * @param - buf : 要写给设备写入的数据
 * @param - cnt : 要写入的数据长度
 * @param - offt : 相对于文件首地址的偏移
 * @return : 写入的字节数，如果为负值，表示写入失败
*/
static ssize_t hello_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
    int ret = 0;

    ret = copy_from_user(write_buf, buf, cnt);
    if (ret == 0) {
        printk("write buffer:%s\n", write_buf);
    } else {
        printk("write buffer failed\n");
    }

    printk("%s: write...\n", __func__);
    return ret;
}

static int hello_release(struct inode *inode, struct file *filp)
{
    printk("%s: release...\n", __func__);
    return 0;
}

static struct file_operations hello_fops = {
    .owner      = THIS_MODULE,
    .open       = hello_open,
    .read       = hello_read,
    .write      = hello_write,
    .release    = hello_release,
};

static int init_hello(void)
{
    int ret = 0;
    
    dev_id = MKDEV(HELLO_MAJOR, 0);

    //create device num
    ret = register_chrdev_region(dev_id, 1, HELLO_NAME);
    if (ret < 0) {
        printk("register_chrdev failed\n");
    }

    //init device
    hello_dev.owner = THIS_MODULE;
    cdev_init(&hello_dev, &hello_fops);

    //add device
    cdev_add(&hello_dev, dev_id, 1);

    //crete classe
    hello_class = class_create(THIS_MODULE, HELLO_DEVICE_NAME);
    device_create(hello_class, NULL, dev_id, NULL, HELLO_DEVICE_NAME);

    printk("%s: init...[1],dev_id=%d\n", __func__, dev_id);
    return ret;
}

static void exit_hello(void)
{
    cdev_del(&hello_dev);
    unregister_chrdev_region(dev_id, 1);

    device_destroy(hello_class, HELLO_MAJOR);
    class_destroy(hello_class);
    printk("%s: exit...\n", __func__);
}

module_init(init_hello);
module_exit(exit_hello);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Paul Yang");


/*
 * 其中高 12 位为主设备号，低 20 位为次设备号。因此 Linux系统中主设备号范围为 0~4095
*/

/*
 * linux中__init的作用
         在init.h中定义的宏，#define __init        __section(.init.text) __cold notrace  
         gcc在编译时会将被修饰的内容放到这些宏所代表的section。
         __init，标记内核启动时所用的初始化代码，内核启动完成后就不再使用。
         其所修饰的内容被放到.init.text section中


         __init 宏最常用的地方是驱动模块初始化函数的定义处，其目的是将驱动模块的初始化函数放入名叫.init.text的输入段。
         当内核启动完毕后，这个段中的内存会被释放掉供其他使用。
*/