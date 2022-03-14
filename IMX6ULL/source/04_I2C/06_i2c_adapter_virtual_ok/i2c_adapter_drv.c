#include <linux/completion.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
//#include <linux/i2c-algo-bit.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/of.h>
//#include <linux/platform_data/i2c-gpio.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

static struct i2c_adapter *g_adapter;

static unsigned char eeprom_buffer[512];
static int eeprom_cur_addr = 0;
static void eeprom_emulate_xfer(struct i2c_adapter *i2c_adap, struct i2c_msg *msg)
{
	int i;
	if (msg->flags & I2C_M_RD)
	{
		for (i = 0; i < msg->len; i++)
		{
			msg->buf[i] = eeprom_buffer[eeprom_cur_addr++];
			if (eeprom_cur_addr == 512)
				eeprom_cur_addr = 0;
		}
	}
	else
	{
		if (msg->len >= 1)
		{
			eeprom_cur_addr = msg->buf[0];
			for (i = 1; i < msg->len; i++)
			{
				eeprom_buffer[eeprom_cur_addr++] = msg->buf[i];
				if (eeprom_cur_addr == 512)
					eeprom_cur_addr = 0;
			}
		}
	}
}

static int i2c_bus_virtual_master_xfer(struct i2c_adapter *i2c_adap,
		    struct i2c_msg msgs[], int num)
{
	int i;

	// emulate eeprom , addr = 0x50
	for (i = 0; i < num; i++)
	{
		if (msgs[i].addr == 0x50)
		{
			eeprom_emulate_xfer(i2c_adap, &msgs[i]);
		}
		else
		{
			i = -EIO;
			break;
		}
	}
	
	return i;
}

static u32 i2c_bus_virtual_func(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_NOSTART | I2C_FUNC_SMBUS_EMUL |
	       I2C_FUNC_SMBUS_READ_BLOCK_DATA |
	       I2C_FUNC_SMBUS_BLOCK_PROC_CALL |
	       I2C_FUNC_PROTOCOL_MANGLING;
}


const struct i2c_algorithm i2c_bus_virtual_algo = {
	.master_xfer   = i2c_bus_virtual_master_xfer,
	.functionality = i2c_bus_virtual_func,
};


static int i2c_bus_virtual_probe(struct platform_device *pdev)
{
	/* get info from device tree, to set i2c_adapter/hardware  */
	
	/* alloc, set, register i2c_adapter */
	g_adapter = kzalloc(sizeof(*g_adapter), GFP_KERNEL);

	g_adapter->owner = THIS_MODULE;
	g_adapter->class = I2C_CLASS_HWMON | I2C_CLASS_SPD;
	g_adapter->nr = -1;
	snprintf(g_adapter->name, sizeof(g_adapter->name), "i2c-bus-virtual");

	g_adapter->algo = &i2c_bus_virtual_algo;

	i2c_add_adapter(g_adapter); // i2c_add_numbered_adapter(g_adapter);
	
	return 0;
}

static int i2c_bus_virtual_remove(struct platform_device *pdev)
{
	i2c_del_adapter(g_adapter);
	return 0;
}
static const struct of_device_id i2c_bus_virtual_dt_ids[] = {
	{ .compatible = "100ask,i2c-bus-virtual", },
	{ /* sentinel */ }
};

static struct platform_driver i2c_bus_virtual_driver = {
	.driver		= {
		.name	= "i2c-gpio",
		.of_match_table	= of_match_ptr(i2c_bus_virtual_dt_ids),
	},
	.probe		= i2c_bus_virtual_probe,
	.remove		= i2c_bus_virtual_remove,
};


static int __init i2c_bus_virtual_init(void)
{
	int ret;

	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	ret = platform_driver_register(&i2c_bus_virtual_driver);
	if (ret)
		printk(KERN_ERR "i2c-gpio: probe failed: %d\n", ret);

	return ret;
}
module_init(i2c_bus_virtual_init);

static void __exit i2c_bus_virtual_exit(void)
{
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	platform_driver_unregister(&i2c_bus_virtual_driver);
}
module_exit(i2c_bus_virtual_exit);

MODULE_AUTHOR("www.100ask.net");
MODULE_LICENSE("GPL");

