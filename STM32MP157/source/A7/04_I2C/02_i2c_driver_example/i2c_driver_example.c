
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/mod_devicetable.h>
#include <linux/bitops.h>
#include <linux/jiffies.h>
#include <linux/property.h>
#include <linux/acpi.h>
#include <linux/i2c.h>
#include <linux/nvmem-provider.h>
#include <linux/regmap.h>
#include <linux/pm_runtime.h>
#include <linux/gpio/consumer.h>

static const struct of_device_id of_match_ids_example[] = {
	{ .compatible = "com_name,chip_name",		.data = NULL },
	{ /* END OF LIST */ },
};

static const struct i2c_device_id example_ids[] = {
	{ "chip_name",	(kernel_ulong_t)NULL },
	{ /* END OF LIST */ }
};

static int i2c_driver_example_probe(struct i2c_client *client)
{
	return 0;
}

static int i2c_driver_example_remove(struct i2c_client *client)
{
	return 0;
}

static struct i2c_driver i2c_example_driver = {
	.driver = {
		.name = "example",
		.of_match_table = of_match_ids_example,
	},
	.probe_new = i2c_driver_example_probe,
	.remove = i2c_driver_example_remove,
	.id_table = example_ids,
};


static int __init i2c_driver_example_init(void)
{
	return i2c_add_driver(&i2c_example_driver);
}
module_init(i2c_driver_example_init);

static void __exit i2c_driver_example_exit(void)
{
	i2c_del_driver(&i2c_example_driver);
}
module_exit(i2c_driver_example_exit);

MODULE_AUTHOR("www.100ask.net");
MODULE_LICENSE("GPL");


