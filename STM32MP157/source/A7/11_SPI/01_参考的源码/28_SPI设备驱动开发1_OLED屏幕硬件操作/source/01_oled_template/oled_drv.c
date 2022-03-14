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

static int oled_probe(struct spi_device *spi)
{
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}

static int oled_remove(struct spi_device *spi)
{
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
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

