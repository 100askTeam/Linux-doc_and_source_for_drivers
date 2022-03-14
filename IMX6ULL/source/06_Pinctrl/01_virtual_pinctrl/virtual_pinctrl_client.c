#include <linux/err.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/mfd/syscon.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/pinctrl/machine.h>
#include <linux/pinctrl/pinconf.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/pinctrl/pinmux.h>
#include <linux/slab.h>
#include <linux/regmap.h>

static const struct of_device_id virtual_client_of_match[] = {
	{ .compatible = "100ask,virtual_i2c", },
	{ },
};

static int virtual_client_probe(struct platform_device *pdev)
{
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}
static int virtual_client_remove(struct platform_device *pdev)
{

	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}


static struct platform_driver virtual_client_driver = {
	.probe		= virtual_client_probe,
	.remove		= virtual_client_remove,
	.driver		= {
		.name	= "100ask_virtual_client",
		.of_match_table = of_match_ptr(virtual_client_of_match),
	}
};


/* 1. 入口函数 */
static int __init virtual_client_init(void)
{	
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	/* 1.1 注册一个platform_driver */
	return platform_driver_register(&virtual_client_driver);
}


/* 2. 出口函数 */
static void __exit virtual_client_exit(void)
{
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	/* 2.1 反注册platform_driver */
	platform_driver_unregister(&virtual_client_driver);
}

module_init(virtual_client_init);
module_exit(virtual_client_exit);

MODULE_LICENSE("GPL");



