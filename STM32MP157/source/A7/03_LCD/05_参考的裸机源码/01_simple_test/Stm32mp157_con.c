#include"Stm32mp157_soc.h"
#include"lcd_controller_manager.h"

static volatile unsigned int *RCC_PLL4FRACR ;
static volatile unsigned int *RCC_RCK4SELR  ; 
static volatile unsigned int *RCC_PLL4CFGR1 ; 
static volatile unsigned int *RCC_PLL4CFGR2 ;
static volatile unsigned int *RCC_PLL4CR  	;
static volatile unsigned int *RCC_MP_AHB4ENSETR ;
static volatile unsigned int *RCC_MP_APB4ENSETR ;

static volatile unsigned int *GPIOD_AFRH    ;
static volatile unsigned int *GPIOD_MODER	;
static volatile unsigned int *GPIOE_AFRL    ;
static volatile unsigned int *GPIOE_AFRH    ;
static volatile unsigned int *GPIOE_MODER	;
static volatile unsigned int *GPIOE_ODR  	;
static volatile unsigned int *GPIOG_AFRH    ;
static volatile unsigned int *GPIOG_AFRL    ;
static volatile unsigned int *GPIOG_MODER	;
static volatile unsigned int *GPIOH_AFRL    ;
static volatile unsigned int *GPIOH_AFRH    ;
static volatile unsigned int *GPIOH_MODER	;
static volatile unsigned int *GPIOI_AFRL    ;
static volatile unsigned int *GPIOI_AFRH    ;
static volatile unsigned int *GPIOI_MODER	;
static volatile unsigned int *GPIOD_OSPEEDR	;
static volatile unsigned int *GPIOE_OSPEEDR	;
static volatile unsigned int *GPIOG_OSPEEDR	;
static volatile unsigned int *GPIOH_OSPEEDR	;
static volatile unsigned int *GPIOI_OSPEEDR	;

static void Stm32mp157_clk_init(void)
{
	 RCC_RCK4SELR        = (volatile unsigned int *)(0x50000824);
	 RCC_PLL4CFGR1       = (volatile unsigned int *)(0x50000898);
	 RCC_PLL4CFGR2       = (volatile unsigned int *)(0x5000089C);
	 RCC_PLL4CR          = (volatile unsigned int *)(0x50000894);
	 RCC_MP_AHB4ENSETR   = (volatile unsigned int *)(0x50000A28);
	 RCC_MP_APB4ENSETR   = (volatile unsigned int *)(0x50000200);
	 
	 /* 像素时钟计算公式如下
	  * pixel_clk(pll4_q_ck)= HSE_FEQ / DIVM4 * (DIVN4+(FRACV4/8192)) / DIVQ4
      * 七寸屏时钟：51.2Mhz（根据规格书确定）,即pixel_clk = 51.2Mhz 
      * HSE_FEQ            	   = 24   采用外部时钟，所以为24Mhz
      * DIVM4                  = 2   （寄存器相应位设置为1）
      * DIVN4                  = 64  （寄存器相应位设置为63）
      * FRACV4        		   = 0   （寄存器相应位设置为0）
      * DIVQ4                  = 15  （寄存器相应位设置为14）
	  */ 
	  
	*RCC_RCK4SELR &= ~(0x3<<0); /*晶振作为时钟源，HSE*/ 
	*RCC_RCK4SELR |= 0x1; 		/*晶振作为时钟源，HSE*/ 
	 
	*RCC_PLL4CFGR1 &= ~(0xC7<<0) ;/*清空DIVN4*/
	*RCC_PLL4CFGR1 &= ~(0x3F<<16);/*清空DIVM4*/
	*RCC_PLL4CFGR1 |= (0x3F<<0) | (0x1<<16); /*DIVN4 = 63  , DIVM4 = 2 */
	
	*RCC_PLL4CFGR2 &= ~(0x7F<<8);  /*清空DIVP*/
	*RCC_PLL4CFGR2 |= (0xE<<8);    /*DIVQ4 = 14 */
	
	*RCC_PLL4CR |= 1<<0;		/*PLL4使能*/
	while(!(*RCC_PLL4CR & 0x2));/*等待PLL4起来*/
	*RCC_PLL4CR |= 1<<5;        /*pll4_q_ck使能*/

	*RCC_MP_AHB4ENSETR |= 1<<3 | 1<<4 | 1<<6 | 1<<7 | 1<<8; /*使能GPIOD，GPIOE，GPIOG，GPIOH，GPIOI时钟*/
	*RCC_MP_APB4ENSETR |= 0x1 << 0;							/*使能LCD控制器外设时钟*/
}
static void Stm32mp157_lcd_io_init(void)
{
	GPIOD_MODER 	 = (volatile unsigned int *)(0x50005000);
	GPIOD_AFRH	     = (volatile unsigned int *)(0x50005024);
	GPIOD_OSPEEDR	 = (volatile unsigned int *)(0x50005008);
	
	GPIOE_MODER 	 = (volatile unsigned int *)(0x50006000);
	GPIOE_AFRH	     = (volatile unsigned int *)(0x50006024);
	GPIOE_AFRL	     = (volatile unsigned int *)(0x50006020);
	GPIOE_ODR        = (volatile unsigned int *)(0x50006014);
	GPIOE_OSPEEDR	 = (volatile unsigned int *)(0x50006008);
	
	GPIOG_MODER 	 = (volatile unsigned int *)(0x50008000);
	GPIOG_AFRH	     = (volatile unsigned int *)(0x50008024);
	GPIOG_AFRL	     = (volatile unsigned int *)(0x50008020);	
	GPIOG_OSPEEDR	 = (volatile unsigned int *)(0x50008008);
	
	GPIOH_MODER 	 = (volatile unsigned int *)(0x50009000);
	GPIOH_AFRH	     = (volatile unsigned int *)(0x50009024);
	GPIOH_AFRL	     = (volatile unsigned int *)(0x50009020);
	GPIOH_OSPEEDR	 = (volatile unsigned int *)(0x50009008);
	
	GPIOI_MODER 	 = (volatile unsigned int *)(0x5000A000);
	GPIOI_AFRH	     = (volatile unsigned int *)(0x5000A024);
	GPIOI_AFRL	     = (volatile unsigned int *)(0x5000A020);
	GPIOI_OSPEEDR	 = (volatile unsigned int *)(0x5000A008);
	
	/*为方便起见，GPIO速度都设置为最高速度模式*/
	*GPIOD_OSPEEDR	 = 0xffffffff;
	*GPIOE_OSPEEDR	 = 0xffffffff;
	*GPIOG_OSPEEDR	 = 0xffffffff;
	*GPIOH_OSPEEDR	 = 0xffffffff;
	*GPIOI_OSPEEDR	 = 0xffffffff;
	
	*GPIOD_MODER  = 0x2<<20;                                 //PD10采用复用功能
	*GPIOD_AFRH   = 0xE<<8;		  						     //PD10复用功能为AF14
	
	*GPIOE_MODER  = 0x2<<12 | 0x2<<26 | 0x2<<28 | 0x2<<30;   //PE6,PE13,PE14,PE15采用复用功能
	*GPIOE_AFRH   = 0xE<<20 | 0xE<<24 | 0xE<<28;  			 //PE13,PE14,PE15复用功能为AF14
	*GPIOE_AFRL   = 0xE<<24;								 //PE6复用功能为AF14
	
	*GPIOG_MODER  = 0x2<<14 | 0x2<<20 | 0x2<<24;		     //PG7,PG10,PG12采用复用功能
	*GPIOG_AFRH   = 0xE<<8 | 0xE<<16;			  			 //PG10,PG12复用功能为AF14
	*GPIOG_AFRL   = 0xE<<28;								 //PG7复用功能为AF14
	
	*GPIOH_MODER  = 0x2<<4 | 0x2<<6 | 0x2<<16 | 0x2<<18 | 0x2<<20 | 0x2<<22 | 0x2<<24 | 0x2<<26 | 0x2<<28 | 0x2<<30;  //PH2,PH3,PH8,PH9,PH10,PH11,PH12 PH13,PH14,PH15采用复用功能
	*GPIOH_AFRH   = 0xE<<0 | 0xE<<4 | 0xE<<8 | 0xE<<12 | 0xE<<16 | 0xE<<20 | 0xE<<24 | 0xE<<28 ;			 //PH8,PH9,PH10,PH11,PH12 PH13,PH14,PH15复用功能为AF14
	*GPIOH_AFRL   = 0xE<<8 | 0xE<<12;						 //PH2,PH3复用功能为AF14		 
	
	*GPIOI_MODER  = 0x2<<0 | 0x2<<2 | 0x2<<4 | 0x2<<8 | 0x2<<10 | 0x2<<12 | 0x2<<14 | 0x2<<18 | 0x2<<20;  //PI0,PI1,PI2,PI4,PI5,PI6,PI7,PI9,PI10采用复用功能
	*GPIOI_AFRH   = 0xE<<4 | 0xE<<8;			  			 							//PI9,PI10复用功能为AF14
	*GPIOI_AFRL   = 0xE<<0 | 0xE<<4 | 0xE<<8 | 0xE<<16 | 0xE<<20 | 0xE<<24 | 0xE<<28;   //PI0,PI1,PI2,PI4,PI5,PI6,PI7复用功能为AF14
	
	*GPIOE_MODER &=  ~(3<<22);			 /*清空PE11模式寄存器*/
	*GPIOE_MODER |=  1<<22   ;  		/*设置为通用输出模式*/
}
static void Stm32mp157_lcd_controller_init(p_lcd_params plcdparams)
{	
    int bpp_mode;
	 
	Stm32mp157_clk_init();  	/*时钟初始化*/
	 
	Stm32mp157_lcd_io_init();  /*LCD控制器占据的GPIO引脚初始化*/

	/*[11:0]垂直同步信号宽度tvp，[27:16]水平同步信号宽度thp*/
	LCDIF->LTDC_SSCR = (plcdparams->time_seq.tvp << 0) | (plcdparams->time_seq.thp << 16);
	
	/*清空LTDC_BPCR寄存器*/
	LCDIF->LTDC_BPCR = 0 ;
	/*[11:0] VSYNC宽度tvp + 上黑框tvb - 1*/
	LCDIF->LTDC_BPCR |= (plcdparams->time_seq.tvp + plcdparams->time_seq.tvb - 1) << 0 ;
	/*[27:16]HSYNC宽度thp + 左黑框thb - 1*/
	LCDIF->LTDC_BPCR |=	(plcdparams->time_seq.thp + plcdparams->time_seq.thb - 1) << 16;
	
	/*清空LTDC_AWCR寄存器*/
	LCDIF->LTDC_AWCR = 0 ;
	/*[11:0]  VSYNC宽度tvp + 上黑框tvb + 垂直有效高度yres - 1*/
	LCDIF->LTDC_AWCR |= (plcdparams->time_seq.tvp + plcdparams->time_seq.tvb + plcdparams->yres - 1) << 0;
	/*[27:16] HSYNC宽度thp + 左黑框thb +  水平有效高度xres - 1*/ 
	LCDIF->LTDC_AWCR |= (plcdparams->time_seq.thp + plcdparams->time_seq.thb + plcdparams->xres - 1) << 16;
	
	/*清空LTDC_TWCR寄存器*/
	LCDIF->LTDC_TWCR = 0;
	/*[11:0]  VSYNC宽度tvp + 上黑框tvb + 垂直有效高度yres + 下黑框tvf - 1 ， 即垂直方向上的总周期*/
	LCDIF->LTDC_TWCR |= (plcdparams->time_seq.tvp + plcdparams->time_seq.tvb + plcdparams->yres + plcdparams->time_seq.tvf - 1) << 0;
	/*[27:16] HSYNC宽度thp + 左黑框thb + 垂直有效高度xres + 右黑框thf - 1 ， 即水平方向上的总周期*/
	LCDIF->LTDC_TWCR |= (plcdparams->time_seq.thp + plcdparams->time_seq.thb + plcdparams->xres + plcdparams->time_seq.thf - 1) << 16;
		
	/*清空LTDC_GCR寄存器*/
	LCDIF->LTDC_GCR &= ~(0xF << 28);
	/*  1 : DOTCLK下降沿有效 ，根据屏幕配置文件将其设置为1    */
	LCDIF->LTDC_GCR |= plcdparams->pins_pol.vclk  << 28;
	/*  1 : ENABLE信号高电平有效，根据屏幕配置文件将其设置为1 */
	LCDIF->LTDC_GCR |= plcdparams->pins_pol.de    << 29;
	/*  0 : VSYNC低电平有效  ,根据屏幕配置文件将其设置为0     */
	LCDIF->LTDC_GCR |= plcdparams->pins_pol.vsync << 30 ;
	/*  0 : HSYNC低电平有效 , 根据屏幕配置文件将其设置为0     */
	LCDIF->LTDC_GCR |= plcdparams->pins_pol.hsync << 31 ;

	/*layer 1的相关设置如下*/
	LCDIF->LTDC_L1WHPCR |= (plcdparams->time_seq.thp + plcdparams->time_seq.thb + plcdparams->xres - 1) << 16 | (plcdparams->time_seq.thp + plcdparams->time_seq.thb ) ;

	LCDIF->LTDC_L1WVPCR |= (plcdparams->time_seq.tvp + plcdparams->time_seq.tvb + plcdparams->yres - 1) << 16 | (plcdparams->time_seq.tvp + plcdparams->time_seq.tvb ) ;

	LCDIF->LTDC_L1CFBLR = (plcdparams->xres * (plcdparams->bpp>>3) + 7) | (plcdparams->xres * (plcdparams->bpp>>3))<< 16;
	 
	LCDIF->LTDC_L1CFBLNR = plcdparams->yres;/*显存总共的行数*/
	
	/*透明度填充值，当选的bpp格式是ARGB8888，ARGB1555等会使用到，如选的是RGB565,RBG888等者不设置也可以*/
	LCDIF->LTDC_L1CACR = 0xff;

	/*
	 *BC = BF1 x C + BF2 x Cs 
	 *BF1为LTDC_L1BFCR设置的[10:8]值,设置为100：constant alpha即LTDC_L1CACR设置的值0xff，表示完全不透明
	 *BF2为LTDC_L1BFCR设置的[2:0]值，设置为101：constant alpha即LTDC_L1CACR设置的值0xff，表示完全不透明
	 *C为当前层的颜色,
	 *Cs为背景色，不设置，默认值为0，即黑色
	 *LTDC_L1BFCR寄存器也是针对有透明度的像素格式而设置，如用RGB565等也可不设置
	 */
	LCDIF->LTDC_L1BFCR = (4<<8) | (5<<0);
	
	 /*当bpp为16时，数据格式为RGB565 , 当bpp为32时，数据格式为ARGB8888*/
     switch(plcdparams->bpp)
	{
		case 16:{
			bpp_mode = 0x2;break;}
		case 32:{
			bpp_mode = 0x0;break;}
		default:{
			bpp_mode = 0x0;break;}
	}
	 
	 LCDIF->LTDC_L1PFCR  =       0 ;
	 LCDIF->LTDC_L1PFCR |= bpp_mode; /*设置像素格式*/
	  
	 LCDIF->LTDC_L1CFBAR = plcdparams->fb_base; /*设置显存地址*/
	 
	 LCDIF->LTDC_L1CR |= 0x1;/*1 layer 使能*/
	
}
static void Stm32mp157_lcd_controller_enable(void)
{	
	LCDIF->LTDC_SRCR |= 1;         /*加载LAYER的参数*/
	
	LCDIF->LTDC_GCR  |= 1<<0;      /* 使能STM32MP157的LCD控制器 */
	
	*GPIOE_ODR |= 0x1<<11;         /*打开背光*/
}
static void Stm32mp157_lcd_controller_disable(void)
{
	LCDIF->LTDC_GCR &= ~(1<<0); /* 关闭STM32MP157的LCD控制器 */
	
	*GPIOE_ODR &= ~(0x1<<11);      /*关闭背光*/
}

struct lcd_controller Stm32mp157_lcd_controller = {
	.name         ="Stm32mp157",
	.init         = Stm32mp157_lcd_controller_init,
	.enable 	  = Stm32mp157_lcd_controller_enable,
	.disable      = Stm32mp157_lcd_controller_disable,
};

/**********************************************************************
 * 函数名称： Stm32mp157_lcd_controller_add
 * 功能描述： 添加Stm32mp157的lcd控制器
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2020/02/26	     V1.0	  zh(angenao)	      创建
 ***********************************************************************/
void Stm32mp157_lcd_controller_add(void)
{
	register_lcd_controller(&Stm32mp157_lcd_controller);
}
