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

#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>



//模块入口
static int __init dtsof_init(void)
{
    int ret = 0;
    struct device_node *bl_nd = NULL;
    struct property *compro = NULL;
    const char *str;
    u32 def_value;
    u32 *brival;
    int i = 0;

    //找到backlight节点，路径是：/backlight
    bl_nd = of_find_node_by_path("/backlight");
    if(bl_nd == NULL)
    {
        ret = -EINVAL;
        goto fail_findnd;
    }
    
    //获取属性
    compro = of_find_property(bl_nd, "compatible",NULL);
    if(compro == NULL)
    {   
        ret = -EINVAL;
        goto fail_finpro;
    }
    else
    {
        printk("compatible: %s\n", (char*)compro->value);
    }

    ret = of_property_read_string(bl_nd,"status", &str);
    if(ret != 0)
    {
        goto fail_rs;
    }
    else
    {
        printk("status: %s\n", str);
    }

    ret = of_property_read_u32(bl_nd, "default-brightness-level",&def_value);
    if(ret != 0)
    {
        goto fail_r3;
    }
    else
    {
        printk("default-brightness-level: %u\n", def_value);
    }

    //获取数组类型的属性
    ret = of_property_count_elems_of_size(bl_nd,"brightness-levels",sizeof(u32));
    if(ret < 0)
    {
        goto fail_readele;
    }
    else
    {
        printk("brightness-levels: %d\n", ret);
    }
    //申请内存
    brival = kmalloc(ret*sizeof(u32),GFP_KERNEL);
    if(!brival)
        goto fail_ele;
    //获取数组
    ret = of_property_read_u32_array(bl_nd,"brightness-levels",brival,ret);
    if(ret < 0)
    {
        goto fail_read32arry; 
    }
    else
    {
        printk("brightness-levels: ");
        for(i=0;i<8;i++)
        {
            printk("%d ",*(brival + i));
        }
        printk("\n");
    }
    kfree(brival);
    return 0;
fail_read32arry:
    kfree(brival);
fail_ele:
fail_readele:
fail_r3:
fail_rs:
fail_finpro:
fail_findnd:
    return ret;
}

//模块出口
static void __exit dtsof_exit(void)
{

}


//模块入口和出口
module_init(dtsof_init);
module_exit(dtsof_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("HAN");