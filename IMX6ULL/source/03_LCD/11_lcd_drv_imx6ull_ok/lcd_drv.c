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
#include <video/display_timing.h>
#include <video/of_display_timing.h>
#include <linux/gpio/consumer.h>

#include <asm/div64.h>

#include <asm/mach/map.h>

struct imx6ull_lcdif {
  volatile unsigned int CTRL;                              
  volatile unsigned int CTRL_SET;                        
  volatile unsigned int CTRL_CLR;                         
  volatile unsigned int CTRL_TOG;                         
  volatile unsigned int CTRL1;                             
  volatile unsigned int CTRL1_SET;                         
  volatile unsigned int CTRL1_CLR;                       
  volatile unsigned int CTRL1_TOG;                       
  volatile unsigned int CTRL2;                            
  volatile unsigned int CTRL2_SET;                       
  volatile unsigned int CTRL2_CLR;                        
  volatile unsigned int CTRL2_TOG;                        
  volatile unsigned int TRANSFER_COUNT;   
       unsigned char RESERVED_0[12];
  volatile unsigned int CUR_BUF;                          
       unsigned char RESERVED_1[12];
  volatile unsigned int NEXT_BUF;                        
       unsigned char RESERVED_2[12];
  volatile unsigned int TIMING;                          
       unsigned char RESERVED_3[12];
  volatile unsigned int VDCTRL0;                         
  volatile unsigned int VDCTRL0_SET;                      
  volatile unsigned int VDCTRL0_CLR;                     
  volatile unsigned int VDCTRL0_TOG;                     
  volatile unsigned int VDCTRL1;                          
       unsigned char RESERVED_4[12];
  volatile unsigned int VDCTRL2;                          
       unsigned char RESERVED_5[12];
  volatile unsigned int VDCTRL3;                          
       unsigned char RESERVED_6[12];
  volatile unsigned int VDCTRL4;                           
       unsigned char RESERVED_7[12];
  volatile unsigned int DVICTRL0;    
  	   unsigned char RESERVED_8[12];
  volatile unsigned int DVICTRL1;                         
       unsigned char RESERVED_9[12];
  volatile unsigned int DVICTRL2;                        
       unsigned char RESERVED_10[12];
  volatile unsigned int DVICTRL3;                        
       unsigned char RESERVED_11[12];
  volatile unsigned int DVICTRL4;                          
       unsigned char RESERVED_12[12];
  volatile unsigned int CSC_COEFF0;  
  	   unsigned char RESERVED_13[12];
  volatile unsigned int CSC_COEFF1;                        
       unsigned char RESERVED_14[12];
  volatile unsigned int CSC_COEFF2;                        
       unsigned char RESERVED_15[12];
  volatile unsigned int CSC_COEFF3;                        
       unsigned char RESERVED_16[12];
  volatile unsigned int CSC_COEFF4;   
  	   unsigned char RESERVED_17[12];
  volatile unsigned int CSC_OFFSET;  
       unsigned char RESERVED_18[12];
  volatile unsigned int CSC_LIMIT;  
       unsigned char RESERVED_19[12];
  volatile unsigned int DATA;                              
       unsigned char RESERVED_20[12];
  volatile unsigned int BM_ERROR_STAT;                     
       unsigned char RESERVED_21[12];
  volatile unsigned int CRC_STAT;                        
       unsigned char RESERVED_22[12];
  volatile  unsigned int STAT;                             
       unsigned char RESERVED_23[76];
  volatile unsigned int THRES;                             
       unsigned char RESERVED_24[12];
  volatile unsigned int AS_CTRL;                           
       unsigned char RESERVED_25[12];
  volatile unsigned int AS_BUF;                            
       unsigned char RESERVED_26[12];
  volatile unsigned int AS_NEXT_BUF;                     
       unsigned char RESERVED_27[12];
  volatile unsigned int AS_CLRKEYLOW;                    
       unsigned char RESERVED_28[12];
  volatile unsigned int AS_CLRKEYHIGH;                   
       unsigned char RESERVED_29[12];
  volatile unsigned int SYNC_DELAY;                      
} ;




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
static struct clk* clk_pix;
static struct clk* clk_axi;

static void lcd_controller_enable(struct imx6ull_lcdif *lcdif)
{
	lcdif->CTRL |= (1<<0);
}

static int lcd_controller_init(struct imx6ull_lcdif *lcdif, struct display_timing *dt, int lcd_bpp, int fb_bpp, unsigned int fb_phy)
{
	int lcd_data_bus_width;
	int fb_width;
	int vsync_pol = 0;
	int hsync_pol = 0;
	int de_pol = 0;
	int clk_pol = 0;

	if (dt->flags & DISPLAY_FLAGS_HSYNC_HIGH)
		hsync_pol = 1;
	if (dt->flags & DISPLAY_FLAGS_VSYNC_HIGH)
		vsync_pol = 1;
	if (dt->flags & DISPLAY_FLAGS_DE_HIGH)
		de_pol = 1;
	if (dt->flags & DISPLAY_FLAGS_PIXDATA_POSEDGE)
		clk_pol = 1;
	
	if (lcd_bpp == 24)
		lcd_data_bus_width = 0x3;
	else if (lcd_bpp == 18)
		lcd_data_bus_width = 0x2;
	else if (lcd_bpp == 8)
		lcd_data_bus_width = 0x1;
	else if (lcd_bpp == 16)
		lcd_data_bus_width = 0x0;
	else
		return -1;

	if (fb_bpp == 24 || fb_bpp == 32)
		fb_width = 0x3;
	else if (fb_bpp == 18)
		fb_width = 0x2;
	else if (fb_bpp == 8)
		fb_width = 0x1;
	else if (fb_bpp == 16)
		fb_width = 0x0;
	else
		return -1;

	/* 
     * 初始化LCD控制器的CTRL寄存器
     * [19]       :  1      : DOTCLK和DVI modes需要设置为1 
     * [17]       :  1      : 设置为1工作在DOTCLK模式
     * [15:14]    : 00      : 输入数据不交换（小端模式）默认就为0，不需设置
     * [13:12]    : 00      : CSC数据不交换（小端模式）默认就为0，不需设置
     * [11:10]    : 11		: 数据总线为24bit
     * [9:8]    根据显示屏资源文件bpp来设置：8位0x1 ， 16位0x0 ，24位0x3
     * [5]        :  1      : 设置elcdif工作在主机模式
     * [1]        :  0      : 24位数据均是有效数据，默认就为0，不需设置
	 */	
	lcdif->CTRL = (0<<30) | (0<<29) | (0<<28) | (1<<19) | (1<<17) | (lcd_data_bus_width << 10) |\
	              (fb_width << 8) | (1<<5);

	/*
	* 设置ELCDIF的寄存器CTRL1
	* 根据bpp设置，bpp为24或32才设置
	* [19:16]  : 111  :表示ARGB传输格式模式下，传输24位无压缩数据，A通道不用传输）
	*/	  
	if(fb_bpp == 24 || fb_bpp == 32)
	{	  
		  lcdif->CTRL1 &= ~(0xf << 16); 
		  lcdif->CTRL1 |=  (0x7 << 16); 
	}
	else
		lcdif->CTRL1 |= (0xf << 16); 
	  
	/*
	* 设置ELCDIF的寄存器TRANSFER_COUNT寄存器
	* [31:16]  : 垂直方向上的像素个数  
	* [15:0]   : 水平方向上的像素个数
	*/
	lcdif->TRANSFER_COUNT  = (dt->vactive.typ << 16) | (dt->hactive.typ << 0);

	/*
	* 设置ELCDIF的VDCTRL0寄存器
	* [29] 0 : VSYNC输出  ，默认为0，无需设置
	* [28] 1 : 在DOTCLK模式下，设置1硬件会产生使能ENABLE输出
	* [27] 0 : VSYNC低电平有效	,根据屏幕配置文件将其设置为0
	* [26] 0 : HSYNC低电平有效 , 根据屏幕配置文件将其设置为0
	* [25] 1 : DOTCLK下降沿有效 ，根据屏幕配置文件将其设置为1
	* [24] 1 : ENABLE信号高电平有效，根据屏幕配置文件将其设置为1
	* [21] 1 : 帧同步周期单位，DOTCLK mode设置为1
	* [20] 1 : 帧同步脉冲宽度单位，DOTCLK mode设置为1
	* [17:0] :  vysnc脉冲宽度 
	*/
	  lcdif->VDCTRL0 = (1 << 28)|( vsync_pol << 27)\
					  |( hsync_pol << 26)\
					  |( clk_pol << 25)\
					  |( de_pol << 24)\
					  |(1 << 21)|(1 << 20)|( dt->vsync_len.typ << 0);

	/*
	* 设置ELCDIF的VDCTRL1寄存器
	* 设置垂直方向的总周期:上黑框tvb+垂直同步脉冲tvp+垂直有效高度yres+下黑框tvf
	*/	  
	lcdif->VDCTRL1 = dt->vback_porch.typ + dt->vsync_len.typ + dt->vactive.typ + dt->vfront_porch.typ;  

	/*
	* 设置ELCDIF的VDCTRL2寄存器
	* [18:31]  : 水平同步信号脉冲宽度
	* [17: 0]   : 水平方向总周期
	* 设置水平方向的总周期:左黑框thb+水平同步脉冲thp+水平有效高度xres+右黑框thf
	*/ 

	lcdif->VDCTRL2 = (dt->hsync_len.typ << 18) | (dt->hback_porch.typ + dt->hsync_len.typ + dt->hactive.typ + dt->hfront_porch.typ);

	/*
	* 设置ELCDIF的VDCTRL3寄存器
	* [27:16] ：水平方向上的等待时钟数 =thb + thp
	* [15:0]  : 垂直方向上的等待时钟数 = tvb + tvp
	*/ 

	lcdif->VDCTRL3 = ((dt->hback_porch.typ + dt->hsync_len.typ) << 16) | (dt->vback_porch.typ + dt->vsync_len.typ);

	/*
	* 设置ELCDIF的VDCTRL4寄存器
	* [18]	   使用VSHYNC、HSYNC、DOTCLK模式此为置1
	* [17:0]  : 水平方向的宽度
	*/ 

	lcdif->VDCTRL4 = (1<<18) | (dt->hactive.typ);

	/*
	* 设置ELCDIF的CUR_BUF和NEXT_BUF寄存器
	* CUR_BUF	 :	当前显存地址
	* NEXT_BUF :	下一帧显存地址
	* 方便运算，都设置为同一个显存地址
	*/ 

	lcdif->CUR_BUF  =  fb_phy;
	lcdif->NEXT_BUF =  fb_phy;

	return 0;
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
	struct display_timing *dt = NULL;
	struct imx6ull_lcdif *lcdif;
	struct resource *res;

	display_np = of_parse_phandle(pdev->dev.of_node, "display", 0);

	/* get common info */
	ret = of_property_read_u32(display_np, "bus-width", &width);
	ret = of_property_read_u32(display_np, "bits-per-pixel",
				   &bits_per_pixel);

    /* get timming */
	timings = of_get_display_timings(display_np);
	dt = timings->timings[timings->native_mode];

	/* get gpio from device tree */
	bl_gpio = gpiod_get(&pdev->dev, "backlight", 0);

	/* config bl_gpio as output */
	gpiod_direction_output(bl_gpio, 1);

	/* set val: gpiod_set_value(bl_gpio, status); */

	/* get clk from device tree */
	clk_pix = devm_clk_get(&pdev->dev, "pix");
	clk_axi = devm_clk_get(&pdev->dev, "axi");

	/* set clk rate */
	clk_set_rate(clk_pix, dt->pixelclock.typ);

	/* enable clk */
	clk_prepare_enable(clk_pix);
	clk_prepare_enable(clk_axi);
	
	/* 1.1 分配fb_info */
	myfb_info = framebuffer_alloc(0, NULL);

	/* 1.2 设置fb_info */
	/* a. var : LCD分辨率、颜色格式 */
	myfb_info->var.xres_virtual = myfb_info->var.xres = dt->hactive.typ;
	myfb_info->var.yres_virtual = myfb_info->var.yres = dt->vactive.typ;
	
	myfb_info->var.bits_per_pixel = 16;  /* rgb565 */
	myfb_info->var.red.offset = 11;
	myfb_info->var.red.length = 5;

	myfb_info->var.green.offset = 5;
	myfb_info->var.green.length = 6;

	myfb_info->var.blue.offset = 0;
	myfb_info->var.blue.length = 5;
	

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
	//lcdif = ioremap(0x021C8000, sizeof(*lcdif));
	
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	lcdif = devm_ioremap_resource(&pdev->dev, res);
	
	lcd_controller_init(lcdif, dt, bits_per_pixel, 16, phy_addr);

	lcd_controller_enable(lcdif);
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
	
	iounmap(mylcd_regs);

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

