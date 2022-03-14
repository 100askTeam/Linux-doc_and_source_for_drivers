#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/console.h>
#include <linux/sysrq.h>
#include <linux/platform_device.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial_core.h>
#include <linux/serial.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/rational.h>
#include <linux/reset.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>

#include <asm/irq.h>

static struct uart_driver virt_uart_drv = {
	.owner          = THIS_MODULE,
	.driver_name    = "VIRT_UART",
	.dev_name       = "ttyVIRT",
	.major          = 0,
	.minor          = 0,
	.nr             = 1,
};


static int virtual_uart_probe(struct platform_device *pdev)
{	
	uart_add_one_port(struct uart_driver * drv, struct uart_port * uport);
	
	return 0;
}
static int virtual_uart_remove(struct platform_device *pdev)
{
	
	return 0;
}



static const struct of_device_id virtual_uart_of_match[] = {
	{ .compatible = "100ask,virtual_uart", },
	{ },
};


static struct platform_driver virtual_uart_driver = {
	.probe		= virtual_uart_probe,
	.remove		= virtual_uart_remove,
	.driver		= {
		.name	= "100ask_virtual_uart",
		.of_match_table = of_match_ptr(virtual_uart_of_match),
	}
};


/* 1. 入口函数 */
static int __init virtual_uart_init(void)
{	
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	
	int ret = uart_register_driver(&virt_uart_drv);

	if (ret)
		return ret;
	
	/* 1.1 注册一个platform_driver */
	return platform_driver_register(&virtual_uart_driver);
}


/* 2. 出口函数 */
static void __exit virtual_uart_exit(void)
{
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	/* 2.1 反注册platform_driver */
	platform_driver_unregister(&virtual_uart_driver);
}

module_init(virtual_uart_init);
module_exit(virtual_uart_exit);

MODULE_LICENSE("GPL");


