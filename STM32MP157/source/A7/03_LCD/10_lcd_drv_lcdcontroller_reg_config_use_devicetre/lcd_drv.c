#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>
#include <linux/io.h>

#include <asm/div64.h>

#include <asm/mach/map.h>

struct stm32mp157_lcdif {
	volatile unsigned int LTDC_IDR; 						 
	volatile unsigned int LTDC_LCR; 					   
	volatile unsigned int LTDC_SSCR;						 
	volatile unsigned int LTDC_BPCR;	   
	volatile unsigned int LTDC_AWCR;							 
	volatile unsigned int LTDC_TWCR;						 
	volatile unsigned int LTDC_GCR; 					  
	volatile unsigned int LTDC_GC1R;   
	volatile unsigned int LTDC_GC2R;							
	volatile unsigned int LTDC_SRCR;	
		unsigned char RESERVED_0[4];
	volatile unsigned int LTDC_BCCR;	 
		unsigned char RESERVED_1[4];
	volatile unsigned int LTDC_IER; 					   
	volatile unsigned int LTDC_ISR;   
	volatile unsigned int LTDC_ICR; 						 
	volatile unsigned int LTDC_LIPCR;						 
	volatile unsigned int LTDC_CPSR;						  
	volatile unsigned int LTDC_CDSR; 
		unsigned char RESERVED_2[56];	
	volatile unsigned int LTDC_L1CR;					  
	volatile unsigned int LTDC_L1WHPCR; 					
	volatile unsigned int LTDC_L1WVPCR; 				   
	volatile unsigned int LTDC_L1CKCR;							
	volatile unsigned int LTDC_L1PFCR;							
	volatile unsigned int LTDC_L1CACR;							
	volatile unsigned int LTDC_L1DCCR;							 
	volatile unsigned int LTDC_L1BFCR;
		unsigned char RESERVED_3[8]; 
	volatile unsigned int LTDC_L1CFBAR;   
	volatile unsigned int LTDC_L1CFBLR; 					   
	volatile unsigned int LTDC_L1CFBLNR;
		unsigned char RESERVED_4[12];  
	volatile unsigned int LTDC_L1CLUTWR; 
		unsigned char RESERVED_5[60];	
	volatile unsigned int LTDC_L2CR;  
	volatile unsigned int LTDC_L2WHPCR; 					   
	volatile unsigned int LTDC_L2WVPCR; 					   
	volatile unsigned int LTDC_L2CKCR;						  
	volatile unsigned int LTDC_L2PFCR;	 
	volatile unsigned int LTDC_L2CACR;	
	volatile unsigned int LTDC_L2DCCR;	
	volatile unsigned int LTDC_L2BFCR;	
		unsigned char RESERVED_6[8];   
	volatile unsigned int LTDC_L2CFBAR; 					   
	volatile unsigned int LTDC_L2CFBLR; 					   
	volatile  unsigned int LTDC_L2CFBLNR;
		unsigned char RESERVED_7[12]; 
	volatile unsigned int LTDC_L2CLUTWR;							   
};


struct lcd_regs {
	volatile unsigned int fb_base_phys;
	volatile unsigned int fb_xres;
	volatile unsigned int fb_yres;
	volatile unsigned int fb_bpp;	
};

static struct lcd_regs *mylcd_regs;

static struct fb_info *myfb_info;

static unsigned int pseudo_palette[16];

static struct gpio_desc *bl_gpio;

static struct clk *pixel_clk;

struct stm32mp157_lcdif *lcdif;

static void Stm32mp157_lcd_controller_enable(struct stm32mp157_lcdif *lcdif)
{	
	lcdif->LTDC_SRCR |= 1;         /*加载LAYER的参数*/
	lcdif->LTDC_GCR  |= 1<<0;      /* 使能STM32MP157的LCD控制器 */
}

static int lcd_controller_init(struct stm32mp157_lcdif *lcdif, struct display_timing *dt, int lcd_bpp, int fb_bpp, unsigned int fb_phy)
{
	int bpp_mode;
	int pol_vclk = 0;
	int pol_vsync = 0;
	int pol_hsync = 0;
	int pol_de = 0;
	
	lcd_bpp = lcd_bpp;
	
	/*[11:0]垂直同步信号宽度tvp，[27:16]水平同步信号宽度thp*/
	lcdif->LTDC_SSCR = (dt->vsync_len.typ << 0) | (dt->hsync_len.typ << 16);
	
	/*清空LTDC_BPCR寄存器*/
	lcdif->LTDC_BPCR = 0 ;
	/*[11:0] VSYNC宽度tvp + 上黑框tvb - 1*/
	lcdif->LTDC_BPCR |= (dt->vsync_len.typ +  dt->vback_porch.typ - 1) << 0 ;
	/*[27:16]HSYNC宽度thp + 左黑框thb - 1*/
	lcdif->LTDC_BPCR |=	(dt->hsync_len.typ + dt->hback_porch.typ - 1) << 16;
	
	/*清空LTDC_AWCR寄存器*/
	lcdif->LTDC_AWCR = 0 ;
	/*[11:0]  VSYNC宽度tvp + 上黑框tvb + 垂直有效高度yres - 1*/
	lcdif->LTDC_AWCR |= (dt->vsync_len.typ + dt->vback_porch.typ + dt->vactive.typ - 1) << 0;
	/*[27:16] HSYNC宽度thp + 左黑框thb +  水平有效高度xres - 1*/ 
	lcdif->LTDC_AWCR |= (dt->hsync_len.typ + dt->hback_porch.typ + dt->hactive.typ - 1) << 16;
	
	/*清空LTDC_TWCR寄存器*/
	lcdif->LTDC_TWCR = 0;
	/*[11:0]  VSYNC宽度tvp + 上黑框tvb + 垂直有效高度yres + 下黑框tvf - 1 ， 即垂直方向上的总周期*/
	lcdif->LTDC_TWCR |= (dt->vsync_len.typ + dt->vback_porch.typ + dt->vactive.typ + dt->vfront_porch.typ - 1) << 0;
	/*[27:16] HSYNC宽度thp + 左黑框thb + 垂直有效高度xres + 右黑框thf - 1 ， 即水平方向上的总周期*/
	lcdif->LTDC_TWCR |= (dt->hsync_len.typ + dt->hback_porch.typ + dt->hactive.typ + dt->hfront_porch.typ - 1) << 16;

	if (dt->flags & DISPLAY_FLAGS_PIXDATA_POSEDGE)
		pol_vclk = 1;
	if (dt->flags & DISPLAY_FLAGS_DE_HIGH)
		pol_de = 1;
	if (dt->flags & DISPLAY_FLAGS_VSYNC_HIGH)
		pol_vsync = 1;
	if (dt->flags & DISPLAY_FLAGS_HSYNC_HIGH)
		pol_hsync = 1;
		
	/*清空LTDC_GCR寄存器*/
	lcdif->LTDC_GCR &= ~(0xF << 28);
	/*  1 : DOTCLK下降沿有效 ，根据屏幕配置文件将其设置为1    */
	lcdif->LTDC_GCR |= pol_vclk  << 28;
	/*  1 : ENABLE信号高电平有效，根据屏幕配置文件将其设置为1 */
	lcdif->LTDC_GCR |= pol_de    << 29;
	/*  0 : VSYNC低电平有效  ,根据屏幕配置文件将其设置为0     */
	lcdif->LTDC_GCR |= pol_vsync << 30 ;
	/*  0 : HSYNC低电平有效 , 根据屏幕配置文件将其设置为0     */
	lcdif->LTDC_GCR |= pol_hsync << 31 ;

	/*layer 1的相关设置如下*/
	lcdif->LTDC_L1WHPCR |= (dt->hsync_len.typ + dt->hback_porch.typ + dt->hactive.typ - 1) << 16 | (dt->hsync_len.typ + dt->hback_porch.typ ) ;

	lcdif->LTDC_L1WVPCR |= (dt->vsync_len.typ + dt->vback_porch.typ + dt->vactive.typ - 1) << 16 | (dt->vsync_len.typ + dt->vback_porch.typ ) ;

	lcdif->LTDC_L1CFBLR = (dt->hactive.typ * (fb_bpp>>3) + 7) | (dt->hactive.typ * (fb_bpp>>3))<< 16;
	 
	lcdif->LTDC_L1CFBLNR = dt->vactive.typ;/*显存总共的行数*/
	
	/*透明度填充值，当选的bpp格式是ARGB8888，ARGB1555等会使用到，如选的是RGB565,RBG888等者不设置也可以*/
	lcdif->LTDC_L1CACR = 0xff;

	/*
	 *BC = BF1 x C + BF2 x Cs 
	 *BF1为LTDC_L1BFCR设置的[10:8]值,设置为100：constant alpha即LTDC_L1CACR设置的值0xff，表示完全不透明
	 *BF2为LTDC_L1BFCR设置的[2:0]值，设置为101：constant alpha即LTDC_L1CACR设置的值0xff，表示完全不透明
	 *C为当前层的颜色,
	 *Cs为背景色，不设置，默认值为0，即黑色
	 *LTDC_L1BFCR寄存器也是针对有透明度的像素格式而设置，如用RGB565等也可不设置
	 */
	lcdif->LTDC_L1BFCR = (4<<8) | (5<<0);
	
	 /*当bpp为16时，数据格式为RGB565 , 当bpp为32时，数据格式为ARGB8888*/
     switch(fb_bpp)
	{
		case 16:{
			bpp_mode = 0x2;break;}
		case 32:{
			bpp_mode = 0x0;break;}
		default:{
			bpp_mode = 0x0;break;}
	}
	 
	 lcdif->LTDC_L1PFCR  =       0 ;
	 lcdif->LTDC_L1PFCR |= bpp_mode; /*设置像素格式*/
	  
	 lcdif->LTDC_L1CFBAR = fb_phy; /*设置显存地址*/
	 
	 lcdif->LTDC_L1CR |= 0x1;/*1 layer 使能*/
}



/* from pxafb.c */
static inline unsigned int chan_to_field(unsigned int chan,
					 struct fb_bitfield *bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf->length;
	return chan << bf->offset;
}

static int mylcd_setcolreg(unsigned regno,
			       unsigned red, unsigned green, unsigned blue,
			       unsigned transp, struct fb_info *info)
{
	unsigned int val;

	/* dprintk("setcol: regno=%d, rgb=%d,%d,%d\n",
		   regno, red, green, blue); */

	switch (info->fix.visual) {
	case FB_VISUAL_TRUECOLOR:
		/* true-colour, use pseudo-palette */

		if (regno < 16) {
			u32 *pal = info->pseudo_palette;

			val  = chan_to_field(red,   &info->var.red);
			val |= chan_to_field(green, &info->var.green);
			val |= chan_to_field(blue,  &info->var.blue);

			pal[regno] = val;
		}
		break;

	default:
		return 1;	/* unknown type */
	}

	return 0;
}


static struct fb_ops myfb_ops = {
	.owner		= THIS_MODULE,
	.fb_setcolreg	= mylcd_setcolreg,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};

static int mylcd_probe(struct platform_device *pdev)
{
	struct device_node *display_np;
	dma_addr_t phy_addr;
	int ret;
	int width;
	int bits_per_pixel;
	struct display_timings *timings = NULL;
	struct display_timing *timing;
	struct resource *res;

	display_np = of_parse_phandle(pdev->dev.of_node, "display", 0);

	/* get common info */
	ret = of_property_read_u32(display_np, "bus-width", &width);
	ret = of_property_read_u32(display_np, "bits-per-pixel",
				   &bits_per_pixel);

    /* get timming */
	timings = of_get_display_timings(display_np);
	timing  = timings->timings[timings->native_mode];
	
	/* get gpio from device tree */
	bl_gpio = gpiod_get(&pdev->dev, "backlight", 0);

	/* config bl_gpio as output */
	gpiod_direction_output(bl_gpio, 1);

	/* set val: gpiod_set_value(bl_gpio, status); */

	/* get clk from device tree */
	pixel_clk = devm_clk_get(&pdev->dev, "lcd");

	/* set rate */
	clk_set_rate(pixel_clk, timing->pixelclock.typ);

	/* enable clk */
	clk_prepare_enable(pixel_clk);
	
	/* 1.1 分配fb_info */
	myfb_info = framebuffer_alloc(0, NULL);

	/* 1.2 设置fb_info */
	/* a. var : LCD分辨率、颜色格式 */
	myfb_info->var.xres_virtual = myfb_info->var.xres = timing->hactive.typ;
	myfb_info->var.yres_virtual = myfb_info->var.yres = timing->vactive.typ;
	
	myfb_info->var.bits_per_pixel = bits_per_pixel;  /* rgb565 */
	if (bits_per_pixel == 16)
	{
		myfb_info->var.red.offset = 11;
		myfb_info->var.red.length = 5;

		myfb_info->var.green.offset = 5;
		myfb_info->var.green.length = 6;

		myfb_info->var.blue.offset = 0;
		myfb_info->var.blue.length = 5;
	}
	else if (bits_per_pixel == 24 || bits_per_pixel == 32)
	{
		myfb_info->var.red.offset = 16;
		myfb_info->var.red.length = 8;

		myfb_info->var.green.offset = 8;
		myfb_info->var.green.length = 8;

		myfb_info->var.blue.offset = 0;
		myfb_info->var.blue.length = 8;
	}
	else
	{
		return -EINVAL;
	}

	/* b. fix */
	strcpy(myfb_info->fix.id, "100ask_lcd");
	myfb_info->fix.smem_len = myfb_info->var.xres * myfb_info->var.yres * myfb_info->var.bits_per_pixel / 8;
	if (myfb_info->var.bits_per_pixel == 24)
		myfb_info->fix.smem_len = myfb_info->var.xres * myfb_info->var.yres * 4;


	/* fb的虚拟地址 */
	myfb_info->screen_base = dma_alloc_wc(NULL, myfb_info->fix.smem_len, &phy_addr,
					 GFP_KERNEL);
	myfb_info->fix.smem_start = phy_addr;  /* fb的物理地址 */
	
	myfb_info->fix.type = FB_TYPE_PACKED_PIXELS;
	myfb_info->fix.visual = FB_VISUAL_TRUECOLOR;

	myfb_info->fix.line_length = myfb_info->var.xres * myfb_info->var.bits_per_pixel / 8;
	if (myfb_info->var.bits_per_pixel == 24)
		myfb_info->fix.line_length = myfb_info->var.xres * 4;
	

	/* c. fbops */
	myfb_info->fbops = &myfb_ops;
	myfb_info->pseudo_palette = pseudo_palette;


	/* 1.3 注册fb_info */
	register_framebuffer(myfb_info);

	/* 1.4 硬件操作 */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	lcdif = devm_ioremap_resource(&pdev->dev, res);
	//lcdif = ioremap(0x5a001000, sizeof(struct stm32mp157_lcdif));
	lcd_controller_init(lcdif, timing, width, bits_per_pixel, phy_addr);

	/* enable lcd controller */
	Stm32mp157_lcd_controller_enable(lcdif);
	
	/* enable backlight */
	gpiod_set_value(bl_gpio, 1);

	return 0;
}


static int mylcd_remove(struct platform_device *pdev)
{
	/* 反过来操作 */
	/* 2.1 反注册fb_info */
	unregister_framebuffer(myfb_info);

	/* 2.2 释放fb_info */
	framebuffer_release(myfb_info);
	
	//iounmap(lcdif);

	return 0;
}


static const struct of_device_id mylcd_of_match[] = {
	{ .compatible = "100ask,lcd_drv", },
	{ },
};
MODULE_DEVICE_TABLE(of, simplefb_of_match);

static struct platform_driver mylcd_driver = {
	.driver = {
		.name = "mylcd",
		.of_match_table = mylcd_of_match,
	},
	.probe = mylcd_probe,
	.remove = mylcd_remove,
};

static int __init lcd_drv_init(void)
{
	int ret;
	struct device_node *np;

	ret = platform_driver_register(&mylcd_driver);
	if (ret)
		return ret;

	return 0;
}

/* 2. 出口 */
static void __exit lcd_drv_exit(void)
{
	platform_driver_unregister(&mylcd_driver);
}


module_init(lcd_drv_init);
module_exit(lcd_drv_exit);

MODULE_AUTHOR("www.100ask.net");
MODULE_DESCRIPTION("Framebuffer driver for the linux");
MODULE_LICENSE("GPL");

