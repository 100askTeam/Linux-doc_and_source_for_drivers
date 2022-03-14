
#include <linux/module.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/mfd/syscon.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/gpio/consumer.h>
#include <linux/gpio/driver.h>
#include <linux/slab.h>
#include <linux/regmap.h>

static struct gpio_chip * g_virt_gpio;
static int g_gpio_val = 0;

static const struct of_device_id virtual_gpio_of_match[] = {
	{ .compatible = "100ask,virtual_gpio", },
	{ },
};

static int virt_gpio_direction_output(struct gpio_chip *gc,
		unsigned offset, int val)
{
	printk("set pin %d as output %s\n", offset, val ? "high" : "low");
	return 0;
}

static int virt_gpio_direction_input(struct gpio_chip *chip,
					unsigned offset)
{
	printk("set pin %d as input\n", offset);
	return 0;
}


static int virt_gpio_get_value(struct gpio_chip *gc, unsigned offset)
{
	int val;
	val = (g_gpio_val & (1<<offset)) ? 1 : 0;
	printk("get pin %d, it's val = %d\n", offset, val);
	return val;
}

static void virt_gpio_set_value(struct gpio_chip *gc,
		unsigned offset, int val)
{
	printk("set pin %d as %d\n", offset, val);
	if (val)
		g_gpio_val |= (1 << offset);
	else
		g_gpio_val &= ~(1 << offset);
}

static int virtual_gpio_probe(struct platform_device *pdev)
{
	int ret;
	unsigned int val;
	
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	
	/* 1. 分配gpio_chip */
	g_virt_gpio = devm_kzalloc(&pdev->dev, sizeof(*g_virt_gpio), GFP_KERNEL);
	
	/* 2. 设置gpio_chip */
	
	/* 2.1 设置函数 */
	g_virt_gpio->label = pdev->name;
	g_virt_gpio->direction_output = virt_gpio_direction_output;
	g_virt_gpio->direction_input  = virt_gpio_direction_input;
	g_virt_gpio->get = virt_gpio_get_value;
	g_virt_gpio->set = virt_gpio_set_value;
	
	g_virt_gpio->parent = &pdev->dev;
	g_virt_gpio->owner = THIS_MODULE;
	
	/* 2.2 设置base、ngpio值 */
	g_virt_gpio->base = -1;
	ret = of_property_read_u32(pdev->dev.of_node, "ngpios", &val);
	g_virt_gpio->ngpio = val;
	
	/* 3. 注册gpio_chip */
	ret = devm_gpiochip_add_data(&pdev->dev, g_virt_gpio, NULL);
	
	return 0;
}
static int virtual_gpio_remove(struct platform_device *pdev)
{
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}


static struct platform_driver virtual_gpio_driver = {
	.probe		= virtual_gpio_probe,
	.remove		= virtual_gpio_remove,
	.driver		= {
		.name	= "100ask_virtual_gpio",
		.of_match_table = of_match_ptr(virtual_gpio_of_match),
	}
};


/* 1. 入口函数 */
static int __init virtual_gpio_init(void)
{	
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	/* 1.1 注册一个platform_driver */
	return platform_driver_register(&virtual_gpio_driver);
}


/* 2. 出口函数 */
static void __exit virtual_gpio_exit(void)
{
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	/* 2.1 反注册platform_driver */
	platform_driver_unregister(&virtual_gpio_driver);
}

module_init(virtual_gpio_init);
module_exit(virtual_gpio_exit);

MODULE_LICENSE("GPL");


