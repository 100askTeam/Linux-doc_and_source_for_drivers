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

static struct uart_port	*virt_port;
static unsigned char txbuf[1024];
static int tx_buf_r = 0;
static int tx_buf_w = 0;

static struct uart_driver virt_uart_drv = {
	.owner          = THIS_MODULE,
	.driver_name    = "VIRT_UART",
	.dev_name       = "ttyVIRT",
	.major          = 0,
	.minor          = 0,
	.nr             = 1,
};

static unsigned int virt_tx_empty(struct uart_port *port)
{
	/* 因为要发送的数据瞬间存入buffer */
	return 1;
}


/*
 * interrupts disabled on entry
 */
static void virt_start_tx(struct uart_port *port)
{
	struct circ_buf *xmit = &port->state->xmit;

	while (!uart_circ_empty(xmit) &&
	       !uart_tx_stopped(port)) {
		/* send xmit->buf[xmit->tail]
		 * out the port here */

		/* 把circ buffer中的数据全部存入txbuf */

		txbuf[tx_buf_w++] =  xmit->buf[xmit->tail];
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
		port->icount.tx++;
	}

   if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
	   uart_write_wakeup(port);

}

static void
virt_set_termios(struct uart_port *port, struct ktermios *termios,
		   struct ktermios *old)
{
	return;
}

static const struct uart_ops virt_pops = {
	.tx_empty	= virt_tx_empty,
	//.set_mctrl	= imx_set_mctrl,
	//.get_mctrl	= imx_get_mctrl,
	//.stop_tx	= imx_stop_tx,
	.start_tx	= virt_start_tx,
	//.stop_rx	= imx_stop_rx,
	//.enable_ms	= imx_enable_ms,
	//.break_ctl	= imx_break_ctl,
	//.startup	= imx_startup,
	//.shutdown	= imx_shutdown,
	//.flush_buffer	= imx_flush_buffer,
	.set_termios	= virt_set_termios,
	//.type		= imx_type,
	//.config_port	= imx_config_port,
	//.verify_port	= imx_verify_port,
};


static int virtual_uart_probe(struct platform_device *pdev)
{	
	int rxirq;
	//uart_add_one_port(struct uart_driver * drv, struct uart_port * uport);

	/* 从设备树获得硬件信息 */
	rxirq = platform_get_irq(pdev, 0);
	
	/* 分配设置注册uart_port */
	virt_port = devm_kzalloc(&pdev->dev, sizeof(*virt_port), GFP_KERNEL);

	virt_port->dev = &pdev->dev;
	virt_port->iotype = UPIO_MEM;
	virt_port->irq = rxirq;
	virt_port->fifosize = 32;
	virt_port->ops = &virt_pops;
	virt_port->flags = UPF_BOOT_AUTOCONF;
	
	return uart_add_one_port(&virt_uart_drv, virt_port);
	
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


