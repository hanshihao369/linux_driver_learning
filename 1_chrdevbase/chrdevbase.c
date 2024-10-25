#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>


#define CHRDEVBASE_MAJOR  200
#define CHRDEVBASE_NAME  "chrdevbase"

static char readbuf[100];   //读缓冲
static char writebuf[100];   //写缓冲
static char kerneldata[] = {"KERNEL data"};

static int chrdevbase_open(struct inode *inode, struct file *filp)
{
    printk("chardevbase open\r\n");
    return 0;
}

static int chrdevbase_release(struct inode *inode, struct file *filp)
{
    //printk("chardevbase release\r\n");
    return 0;
}

static ssize_t chrdevbase_read(struct file *filp, char __user *buf,size_t cnt, loff_t *offt)
{
    int ret = 0;
    //printk("chardevbase read\r\n");
    memcpy(readbuf,kerneldata,sizeof(kerneldata));
    ret = copy_to_user(buf,readbuf,cnt);
    if(ret == 0)
    {

    }
    else
    {

    }
    return 0;
}

static ssize_t chrdevbase_write(struct file *filp, const char __user *buf,size_t cnt, loff_t *offt)
{
    int ret = 0;
    //printk("chardevbase write\r\n");
    ret = copy_from_user(writebuf,buf,cnt);
    if(ret == 0)
    {
        printk("kernel recevdata:%s\r\n",writebuf);
    }
    else
    {

    }
    return 0;
}

static struct file_operations chrdevbase_fops = {
    .owner = THIS_MODULE,  
    .open = chrdevbase_open,
    .release = chrdevbase_release,
    .read = chrdevbase_read,
    .write = chrdevbase_write,
};

static int __init chrdevbase_init(void)
{
    int ret = 0;
    printk("chardevbase_init\r\n");
    //注册字符设备
    ret = register_chrdev(CHRDEVBASE_MAJOR, CHRDEVBASE_NAME, &chrdevbase_fops);
    if(ret < 0)
    {
        printk("chrdevbase_init_failed");
    }
    return 0;
}

static void __exit chrdevbase_exit(void)
{
	printk("chardevbase_exit\r\n");
    //注销字符设备
    unregister_chrdev(CHRDEVBASE_MAJOR, CHRDEVBASE_NAME);
}

//模块入口与出口
module_init(chrdevbase_init);
module_exit(chrdevbase_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("HAN");

