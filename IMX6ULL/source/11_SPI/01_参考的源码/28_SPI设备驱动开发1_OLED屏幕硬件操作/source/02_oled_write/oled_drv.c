#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/list.h>
#include <linux/spi/spi.h>
#include <linux/err.h>
#include <linux/module.h>

struct spi_device *oled_dev;

static int major;
static struct class *oled_class;

static long oled_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct spi_message msg;
	struct spi_transfer trans[3];

	trans[0].tx_buf = buf0;
	trans[0].len    = len0;

	trans[1].rx_buf = buf1;
	trans[1].len    = len1;

	trans[2].tx_buf = buf2;
	trans[2].rx_buf = buf3;
	trans[2].len    = len2;

	spi_message_init(&msg);
	spi_message_add_tail(&trans[0], &msg);
	spi_message_add_tail(&trans[1], &msg);
	spi_message_add_tail(&trans[2], &msg);
	
	return 0;
}

/* 定义自己的file_operations结构体                                              */
static struct file_operations oled_fops = {
	.owner	 = THIS_MODULE,
	.unlocked_ioctl = oled_ioctl,
};

static int oled_probe(struct spi_device *spi)
{	
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

	oled_dev = spi;

	/* register_chrdev */
	major = register_chrdev(0, "oled", &oled_fops);  

	/* class_create */
	oled_class = class_create(THIS_MODULE, "oled_class");

	/* device_create */
	device_create(oled_class, NULL, MKDEV(major, 0), NULL, "myoled");
	
	return 0;
}

static int oled_remove(struct spi_device *spi)
{
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

	device_destroy(oled_class, MKDEV(major, 0));
	class_destroy(oled_class);
	unregister_chrdev(major, "oled");

	return 0;
}

//static const struct spi_device_id oled_spi_ids[] = {
//	{"100ask,oled", },
//	{}
//};

static const struct of_device_id oled_of_match[] = {
	{.compatible = "100ask,oled"},
	{}
};

static struct spi_driver oled_driver = {
	.driver = {
		.name	= "oled",
		.of_match_table = oled_of_match,
	},
	.probe		= oled_probe,
	.remove		= oled_remove,
	//.id_table	= oled_spi_ids,
};

int oled_init(void)
{
	return spi_register_driver(&oled_driver);
}

static void oled_exit(void)
{
	spi_unregister_driver(&oled_driver);
}

module_init(oled_init);
module_exit(oled_exit);

MODULE_LICENSE("GPL v2");

