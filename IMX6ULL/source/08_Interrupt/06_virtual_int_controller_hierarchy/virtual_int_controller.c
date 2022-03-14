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


static struct irq_domain *virtual_intc_domain;
static u32 upper_hwirq_base;

static int virtual_intc_domain_translate(struct irq_domain *d,
				    struct irq_fwspec *fwspec,
				    unsigned long *hwirq,
				    unsigned int *type)
{
	if (is_of_node(fwspec->fwnode)) {
		if (fwspec->param_count != 2)
			return -EINVAL;

		*hwirq = fwspec->param[0];
		*type = fwspec->param[1];
		return 0;
	}

	return -EINVAL;
}

static void virtual_intc_irq_unmask(struct irq_data *d)
{
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	irq_chip_unmask_parent(d);
}

static void virtual_intc_irq_mask(struct irq_data *d)
{
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	irq_chip_mask_parent(d);
}

static struct irq_chip virtual_intc_chip = {
	.name			= "virtual_intc",
	.irq_mask		= virtual_intc_irq_mask,
	.irq_unmask 	= virtual_intc_irq_unmask,
};


static int virtual_intc_domain_alloc(struct irq_domain *domain,
				  unsigned int irq,
				  unsigned int nr_irqs, void *data)
{
	struct irq_fwspec *fwspec = data;
	struct irq_fwspec parent_fwspec;
	irq_hw_number_t hwirq;
	int i;
	
	/* 设置irq_desc[irq] */

	/* 1. 设置irq_desc[irq].irq_data, 里面含有virtual_intc irq_chip */
	hwirq = fwspec->param[0];
	for (i = 0; i < nr_irqs; i++)
		irq_domain_set_hwirq_and_chip(domain, irq + i, hwirq + i,
					      &virtual_intc_chip, NULL);

	/* 2. 设置irq_desc[irq].handle_irq,  来自GIC */
	parent_fwspec.fwnode = domain->parent->fwnode;
	parent_fwspec.param_count = 3;
	parent_fwspec.param[0]    = GIC_SPI;
	parent_fwspec.param[1]    = fwspec->param[0] + upper_hwirq_base;
	parent_fwspec.param[2]    = fwspec->param[1];
	
	return irq_domain_alloc_irqs_parent(domain, irq, nr_irqs,
					  &parent_fwspec);
}

static const struct irq_domain_ops virtual_intc_domain_ops = {
	.translate	= virtual_intc_domain_translate,
	.alloc		= virtual_intc_domain_alloc,
};

static int virtual_intc_probe(struct platform_device *pdev)
{	

	struct irq_domain *parent_domain;
	struct device_node *parent;
	int err;
		

	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	err = of_property_read_u32(pdev->dev.of_node, "upper_hwirq_base", &upper_hwirq_base);

	parent = of_irq_find_parent(pdev->dev.of_node);
	parent_domain = irq_find_host(parent);


	/* 分配/设置/注册irq_domain */
	virtual_intc_domain = irq_domain_add_hierarchy(parent_domain, 0, 4,
					  pdev->dev.of_node, &virtual_intc_domain_ops,
					  NULL);
	
	
	return 0;
}
static int virtual_intc_remove(struct platform_device *pdev)
{
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
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


