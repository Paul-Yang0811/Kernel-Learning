#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/ide.h>
//#include <sys/types.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define LED_MAJOR 100
#define LED_NAME  "led_dev"

#define GPK1CON_BASE 0x11000060 
#define GPK1DAT_BASE 0x11000064 

#define LED_ON  1
#define LED_OFF 0

static void __iomem *GPK1CON;
static void __iomem *GPK1DAT;

static char read_buf[100];
static char write_buf[100];
static char kernel_data[] = {"led data!"};

struct led_dev {
    dev_t dev_id;
    struct cdev cdev;
    struct class *class;

}; 

static struct led_dev led_dev;

void led_control(char status)
{
    int val = 0;
    if (status == 1) {
        //set led on
        val = readl(GPK1DAT);
        val &= ~(0x01 << 1);
        val |=  (0x01 << 1);
        writel(val, GPK1DAT);
    } else if (status == 0) {
        //set led off
        val = readl(GPK1DAT);
        val &=  ~(0x0f << 1);
        writel(val, GPK1DAT);
    } else {
        printk("err led control cmd\n");
    }
}

static int led_open(struct inode *inode, struct file *filp)
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
static ssize_t led_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
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
static ssize_t led_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
    int ret = 0;
    char led_sta = 0;

    ret = copy_from_user(write_buf, buf, cnt);
    if (ret == 0) {
        printk("write buffer:%s\n", write_buf);
    } else {
        printk("write buffer failed\n");
    }

    led_sta = write_buf[0];

    if (led_sta == LED_ON) {
        led_control(LED_ON);
    } else if (led_sta == LED_OFF) {
        led_control(LED_OFF);
    } else {
        printk("unknow led cmd\n");
    }

    printk("%s: write...\n", __func__);
    return ret;
}

static int led_release(struct inode *inode, struct file *filp)
{
    printk("%s: release...\n", __func__);
    return 0;
}

static struct file_operations led_fops = {
    .owner      = THIS_MODULE,
    .open       = led_open,
    .read       = led_read,
    .write      = led_write,
    .release    = led_release,
};

static void config_led(void)
{
    int val = 0;
    
    //remap led address
    GPK1CON = ioremap(GPK1DAT_BASE, 4);
    GPK1DAT = ioremap(GPK1DAT_BASE, 4);

    //config pin as output
    val = readl(GPK1CON);
    val &= ~(0x0f << 4);
    val |=  (0x01 << 4);
    writel(val, GPK1CON);

    printk("%s: config led...\n", __func__);
}

static int init_led(void)
{
    int ret = 0;
    
    config_led();

    led_dev.dev_id = MKDEV(LED_MAJOR, 0);

    //create device num
    ret = register_chrdev_region(led_dev.dev_id, 1, "led_dev");
    if (ret < 0) {
        printk("register_chrdev failed\n");
    }

    //init device
    led_dev.cdev.owner = THIS_MODULE;
    cdev_init(&led_dev.cdev, &led_fops);

    //add device
    cdev_add(&led_dev.cdev, led_dev.dev_id, 1);

    //crete classe
    led_dev.class = class_create(THIS_MODULE, "led_class");
    device_create(led_dev.class, NULL, led_dev.dev_id, NULL, "led_device");

    printk("%s: init...\n", __func__);
    return ret;
}

static void exit_led(void)
{
    unregister_chrdev(LED_MAJOR, LED_NAME);
    printk("%s: exit...\n", __func__);
}

module_init(init_led);
module_exit(exit_led);

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