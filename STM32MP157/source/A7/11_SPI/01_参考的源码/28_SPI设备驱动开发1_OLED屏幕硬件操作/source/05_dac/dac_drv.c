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
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/uaccess.h>


struct spi_device *dac_dev;
static int major;
static struct class *dac_class;

static 	ssize_t dac_write(struct file *file, const char __user *buf, size_t size, loff_t * offset)
{
	unsigned short val = 0;
	unsigned char ker_buf[4];
	int err;
	
	if (size != 2)
		return -EINVAL;

	copy_from_user(&val, buf, 2);

	//val = 0x554;
	val <<= 2;
	val = val & 0x0fff; /* DAC数据格式: 高4位、低2位为0 */

	ker_buf[0] = val >> 8;
	ker_buf[1] = val;

	err = spi_write(dac_dev, ker_buf, 2);

	//spi_write(dac_dev, &val, 2);
	
	//printk("dac_write err = %d, val = 0x%x, %d, ker_buf[0] = 0x%x, ker_buf[1] = 0x%x\n", err, val, val, ker_buf[0], ker_buf[1]);

	return 2;
}

/* 定义自己的file_operations结构体                                              */
static struct file_operations dac_fops = {
	.owner	 = THIS_MODULE,
	.write   = dac_write,
};

static int dac_probe(struct spi_device *spi)
{	
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

	dac_dev = spi;

	/* register_chrdev */
	major = register_chrdev(0, "dac", &dac_fops);  

	/* class_create */
	dac_class = class_create(THIS_MODULE, "dac_class");

	/* device_create */
	device_create(dac_class, NULL, MKDEV(major, 0), NULL, "mydac");
	
	return 0;
}

static int dac_remove(struct spi_device *spi)
{
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

	device_destroy(dac_class, MKDEV(major, 0));
	class_destroy(dac_class);
	unregister_chrdev(major, "dac");

	return 0;
}

//static const struct spi_device_id dac_spi_ids[] = {
//	{"100ask,dac", },
//	{}
//};

static const struct of_device_id dac_of_match[] = {
	{.compatible = "100ask,dac"},
	{}
};

static struct spi_driver dac_driver = {
	.driver = {
		.name	= "dac",
		.of_match_table = dac_of_match,
	},
	.probe		= dac_probe,
	.remove		= dac_remove,
	//.id_table	= dac_spi_ids,
};

int dac_init(void)
{
	return spi_register_driver(&dac_driver);
}

static void dac_exit(void)
{
	spi_unregister_driver(&dac_driver);
}

module_init(dac_init);
module_exit(dac_exit);

MODULE_LICENSE("GPL v2");

