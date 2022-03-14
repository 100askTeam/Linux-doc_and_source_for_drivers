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
#include <linux/mod_devicetable.h>


#define LED_MAX_CNT 10

struct led_desc {
	int pin;
	int minor;
};

/* 1. 确定主设备号 */ 
static int major = 0;
static struct class *led_class;

static int g_ledcnt = 0;
static struct led_desc leds_desc[LED_MAX_CNT];



/* 3. 实现对应的open/read/write等函数，填入file_operations结构体                   */
static ssize_t led_drv_write (struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
	int err;
	char status;
	struct inode *inode = file_inode(file);
	int minor = iminor(inode);
	
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	err = copy_from_user(&status, buf, 1);

	/* 根据次设备号和status控制LED */
	printk("set led pin 0x%x as %d\n", leds_desc[minor].pin, status);
	
	return 1;
}

static int led_drv_open (struct inode *node, struct file *file)
{
	int minor = iminor(node);
	
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

	/* 根据次设备号初始化LED */
	printk("init led pin 0x%x as output\n", leds_desc[minor].pin);
	
	return 0;
}


/* 2. 定义自己的file_operations结构体                                              */
static struct file_operations led_drv = {
	.owner	 = THIS_MODULE,
	.open    = led_drv_open,
	.write   = led_drv_write,
};

/* B.1 实现platform_driver的probe函数  */
static int led_probe(struct platform_device *pdev)
{	
	int minor;
	int i = 0;

	struct resource *res;
		
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

	res = platform_get_resource(pdev, IORESOURCE_IRQ, i++);
	if (!res)
		return -EINVAL;

	/* 记录引脚 */
	minor = g_ledcnt;
	leds_desc[minor].pin = res->start;
	leds_desc[minor].minor = minor;

	/* 7.2 辅助信息 */
	/* 创建设备节点 */
	device_create(led_class, NULL, MKDEV(major, minor), NULL, "100ask_led%d", minor); /* /dev/100ask_led0,1,... */
	
	platform_set_drvdata(pdev, &leds_desc[minor]);
	
	g_ledcnt++;
	
    return 0;
}

/* B.2 实现platform_driver的remove函数  */
static int led_remove(struct platform_device *pdev)
{
	struct led_desc *led = platform_get_drvdata(pdev);

	device_destroy(led_class, MKDEV(major, led->minor)); /* /dev/100ask_led0,1,... */

    return 0;
}

static const struct platform_device_id led_id_table[] = {
	{"100ask_led",   1},
	{"100ask_led_3", 2},
	{"100ask_led_4", 3},
	{ },
};


/* A. 实现platform_driver  */
static struct platform_driver led_driver = {
    .probe      = led_probe,
    .remove     = led_remove,
    .driver     = {
        .name   = "100ask_led",
    },
    .id_table = led_id_table,
};



/* 4. 把file_operations结构体告诉内核：注册驱动程序register_chrdev                                */
/* 5. 谁来注册驱动程序啊？得有一个入口函数：安装驱动程序时，就会去调用这个入口函数 */
static int __init led_init(void)
{
	int err;
	
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	major = register_chrdev(0, "100ask_led", &led_drv);  /* /dev/led */


	/* 7.1 辅助信息 */
	led_class = class_create(THIS_MODULE, "100ask_led_class");
	err = PTR_ERR(led_class);
	if (IS_ERR(led_class)) {
		printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
		unregister_chrdev(major, "led");
		return -1;
	}

	/* C. 注册platform_driver	*/
    err = platform_driver_register(&led_driver); 
    
    return err;
}

/* 6. 有入口函数就应该有出口函数：卸载驱动程序时，就会去调用这个出口函数           */
static void __exit led_exit(void)
{
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

	/* C. 反注册platform_driver	*/
    platform_driver_unregister(&led_driver); 

	class_destroy(led_class);
	unregister_chrdev(major, "100ask_led");
}


/* 7. 其他完善：提供设备信息，自动创建设备节点                                     */

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");


