#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>

#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define NEWCHRLED_NAME "newchrled"
#define NEWCHRLED_COUNT 1

#if 1
//寄存器物理地址
#define CCM_CCGR1_BASE  (0X020C406C)
#define SW_MUX_GPIO1_IO03_BASE (0X020E0068)
#define SW_PAD_GPIO1_IO03_BASE (0X020E02F4)
#define GPIO1_DR_BASE (0X0209C000)
#define GPIO1_GDIR_BASE (0X0209C004)

//地址映射后的虚拟i地址指针
static void __iomem *IMX6U_CCM_CCGR1;
static void __iomem *SW_MUX_GPIO1_IO03;
static void __iomem *SW_PAD_GPIO1_IO03;
static void __iomem *GPIO1_DR;
static void __iomem *GPIO1_GDIR;
#endif

#define LEDOFF 0
#define LEDON 1

struct newchrled_dev{
    struct cdev cdev;   //字符设备
    dev_t devid;
    struct class *class; //类
    struct device *device; //设备
    int major;
    int minor;
};

struct newchrled_dev newchrled;

void led_switch(u8 sta)
{
    u32 val = 0;
    if(sta == LEDON)
    {
        val = readl(GPIO1_DR);
        val &= ~(1 << 3);
        writel(val,GPIO1_DR);
    }
    else if(sta == LEDOFF)
    {
        val = readl(GPIO1_DR);
        val |= (1 << 3);
        writel(val,GPIO1_DR);
    }
}

static int newled_open(struct inode *inode, struct file *filp)
{
    return 0;
}

static int newled_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static ssize_t newled_write(struct file *filp, const char __user *buf,
			 size_t count, loff_t *ppos)
{
    unsigned int retvalue;
    unsigned char databuf[1];
    
    retvalue = copy_from_user(databuf,buf,count);
    if(retvalue < 0)
    {
        printk("kernel write failed");
        return -EFAULT;
    }

    led_switch(databuf[0]);

    return 0;
}

static const struct file_operations newchrled_fops = {
    .owner = THIS_MODULE,
    .open = newled_open,
    .write = newled_write,
    .release = newled_release,
};
//入口 
static int __init newchrled_init(void)
{
    int ret = 0;
    unsigned int val = 0;
    printk("newchrled_init\n");
    //初始化LED
    //初始化LED灯，地址映射
    IMX6U_CCM_CCGR1 = ioremap(CCM_CCGR1_BASE,4);
    SW_MUX_GPIO1_IO03 = ioremap(SW_MUX_GPIO1_IO03_BASE,4);
    SW_PAD_GPIO1_IO03 = ioremap(SW_PAD_GPIO1_IO03_BASE,4);
    GPIO1_DR = ioremap(GPIO1_DR_BASE,4);
    GPIO1_GDIR = ioremap(GPIO1_GDIR_BASE,4);

    //初始化
    val = readl(IMX6U_CCM_CCGR1);
    val &= ~(3 << 26);  //先清除一下以前配置的bit26，27
    val |= 3 << 26; 
    writel(val,IMX6U_CCM_CCGR1);

    writel(0x5,SW_MUX_GPIO1_IO03);  //设置复用
    writel(0x10b0,SW_PAD_GPIO1_IO03);  //设置电气属性

    val = readl(GPIO1_GDIR);
    val |= 1 << 3;
    writel(val,GPIO1_GDIR);

    val = readl(GPIO1_DR);
    val &= ~(1 << 3);
    writel(val,GPIO1_DR);
    //注册字符设备
    if(newchrled.major)
    {
        newchrled.devid = MKDEV(newchrled.major,0);
        ret = register_chrdev_region(newchrled.devid,NEWCHRLED_COUNT,NEWCHRLED_NAME);
    }
    else
    {
        ret = alloc_chrdev_region(&newchrled.devid,0,NEWCHRLED_COUNT,NEWCHRLED_NAME);
        newchrled.major = MAJOR(newchrled.devid);
        newchrled.minor = MINOR(newchrled.devid); 
    }

    if(ret < 0)
    {
        printk("newchrdev chrdev_region err\n");
        return -1;
    }

    printk("newchrdev major=%d,minor=%d\r\n",newchrled.major, newchrled.minor);

    //注册字符设备  
    newchrled.cdev.owner = THIS_MODULE;           
    cdev_init(&newchrled.cdev, &newchrled_fops);

    ret = cdev_add(&newchrled.cdev, newchrled.devid, NEWCHRLED_COUNT);

    //自动创建设备节点
    newchrled.class = class_create(THIS_MODULE,NEWCHRLED_NAME);
    if(IS_ERR(newchrled.class))
        return PTR_ERR(newchrled.class);

    //创建设备
    newchrled.device = device_create(newchrled.class, NULL, newchrled.devid, NULL, NEWCHRLED_NAME);
    if(IS_ERR(newchrled.device))
        return PTR_ERR(newchrled.device);
    return 0;
}


//出口
static void __exit newchrled_exit(void)
{
    unsigned int val = 0;

    printk("newchrled_exit\n");

    val = readl(GPIO1_DR);
    val |= (1 << 3);
    writel(val,GPIO1_DR);
    //取消地址映射
    iounmap(IMX6U_CCM_CCGR1);
    iounmap(SW_MUX_GPIO1_IO03);
    iounmap(SW_PAD_GPIO1_IO03);
    iounmap(GPIO1_DR);
    iounmap(GPIO1_GDIR);


    //删除字符设备
    cdev_del(&newchrled.cdev);
    //注销设备号
    unregister_chrdev_region(newchrled.devid,NEWCHRLED_COUNT);
    //摧毁设备
    device_destroy(newchrled.class, newchrled.devid);
    //摧毁类
    class_destroy(newchrled.class);
}





//注册驱动和卸载驱动
module_init(newchrled_init);
module_exit(newchrled_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("HAN");