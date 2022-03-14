#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/random.h>
#include <linux/irq.h>
#include <linux/irqdomain.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/slab.h>
#include <linux/gpio/driver.h>
/* FIXME: for gpio_get_value() replace this with direct register read */
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/bug.h>
#include <linux/random.h>

static struct irq_domain *virtual_intc_domain;

static int virtual_intc_get_hwirq(void)
{
	return get_random_int() & 0x3;
}

static void virtual_intc_irq_handler(struct irq_desc *desc)
{
	/* 它的功能时分辨是哪一个hwirq, 调用对应的irq_desc[].handle_irq */
	int hwirq;
	
	struct irq_chip *chip = irq_desc_get_chip(desc);

	chained_irq_enter(chip, desc);

	/* a. 分辨中断 */
	hwirq = virtual_intc_get_hwirq();

	/* b. 调用irq_desc[].handle_irq(handleC) */
	generic_handle_irq(irq_find_mapping(virtual_intc_domain, hwirq));

	chained_irq_exit(chip, desc);
}

static void virtual_intc_irq_ack(struct irq_data *data)
{
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
}

static void virtual_intc_irq_mask(struct irq_data *data)
{
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
}

static void virtual_intc_irq_mask_ack(struct irq_data *data)
{
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
}

static void virtual_intc_irq_unmask(struct irq_data *data)
{
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
}

static void virtual_intc_irq_eoi(struct irq_data *data)
{
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
}

static struct irq_chip virtual_intc_irq_chip = {
	.name			= "100ask_virtual_intc",
	.irq_ack	   = virtual_intc_irq_ack	   ,
	.irq_mask	   = virtual_intc_irq_mask	   ,
	.irq_mask_ack  = virtual_intc_irq_mask_ack ,
	.irq_unmask    = virtual_intc_irq_unmask   ,
	.irq_eoi	   = virtual_intc_irq_eoi	   ,
};

static int virtual_intc_irq_map(struct irq_domain *h, unsigned int virq,
			  irq_hw_number_t hw)
{
	/* 1. 给virq提供处理函数
	 * 2. 提供irq_chip用来mask/unmask中断
	 */
	
	irq_set_chip_data(virq, h->host_data);
	//irq_set_chip_and_handler(virq, &virtual_intc_irq_chip, handle_edge_irq); /* handle_edge_irq就是handleC */
	irq_set_chip_and_handler(virq, &virtual_intc_irq_chip, handle_level_irq); /* handle_level_irq就是handleC */
	//irq_set_nested_thread(virq, 1);
	//irq_set_noprobe(virq);

	return 0;
}


static const struct irq_domain_ops virtual_intc_domain_ops = {
	.xlate = irq_domain_xlate_onetwocell,
	.map   = virtual_intc_irq_map,
};

static int virtual_intc_probe(struct platform_device *pdev)
{	
	struct device_node *np = pdev->dev.of_node;
	int irq_to_parent;
	//int irq_base;
	
	/* 1. virutal intc 会向GIC发出n号中断 */
	/* 1.1 从设备树里获得virq_n */
	irq_to_parent = platform_get_irq(pdev, 0);
	printk("virtual_intc_probe irq_to_parent = %d\n", irq_to_parent);
	
	/* 1.2 设置它的irq_desc[].handle_irq, 它的功能时分辨是哪一个hwirq, 调用对应的irq_desc[].handle_irq */
	irq_set_chained_handler_and_data(irq_to_parent, virtual_intc_irq_handler, NULL);

	
	/* 2. 分配/设置/注册一个irq_domain */
	//irq_base = irq_alloc_descs(-1, 0, 4, numa_node_id());
	//printk("virtual_intc_probe irq_base = %d\n", irq_base);

	/* Usage:
	 *  a. dts: 定义使用哪个hwirq
	 *  b. 内核解析设备树时分配irq_desc，得到virq
	 *  c. (hwirq, virq) ==>存入domain
	 */

	virtual_intc_domain = irq_domain_add_linear(np, 4, 
					     &virtual_intc_domain_ops, NULL);
	
	return 0;
}
static int virtual_intc_remove(struct platform_device *pdev)
{
	return 0;
}



static const struct of_device_id virtual_intc_of_match[] = {
	{ .compatible = "100ask,virtual_intc", },
	{ },
};


static struct platform_driver virtual_intc_driver = {
	.probe		= virtual_intc_probe,
	.remove		= virtual_intc_remove,
	.driver		= {
		.name	= "100ask_virtual_intc",
		.of_match_table = of_match_ptr(virtual_intc_of_match),
	}
};


/* 1. 入口函数 */
static int __init virtual_intc_init(void)
{	
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	/* 1.1 注册一个platform_driver */
	return platform_driver_register(&virtual_intc_driver);
}


/* 2. 出口函数 */
static void __exit virtual_intc_exit(void)
{
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	/* 2.1 反注册platform_driver */
	platform_driver_unregister(&virtual_intc_driver);
}

module_init(virtual_intc_init);
module_exit(virtual_intc_exit);

MODULE_LICENSE("GPL");


