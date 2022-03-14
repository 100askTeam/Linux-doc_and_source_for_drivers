
#include <linux/module.h>

#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/kmod.h>
#include <linux/gfp.h>
#include <linux/platform_device.h>

static struct resource resources[] = {
    {
            .start = (3<<8)|(1),			
            .flags = IORESOURCE_IRQ,
    },
};


static void led_dev_release(struct device *dev)
{
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
}


static struct platform_device led_dev = {
        .name = "100ask_led",
		.num_resources = ARRAY_SIZE(resources),
		.resource = resources,
        .dev = {
                .release = led_dev_release,
         },
};

static int __init led_dev_init(void)
{
    int err;
    
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    err = platform_device_register(&led_dev);   
    
    return err;
}

static void __exit led_dev_exit(void)
{
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    platform_device_unregister(&led_dev);
}

module_init(led_dev_init);
module_exit(led_dev_exit);

MODULE_LICENSE("GPL");

