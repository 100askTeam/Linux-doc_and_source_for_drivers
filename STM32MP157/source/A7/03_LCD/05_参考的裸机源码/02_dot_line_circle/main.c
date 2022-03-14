#include "lcd_manager.h"
#include "geometry.h"
#include "framebuffer.h"


void lcd_test(void)
{
	unsigned int fb_base ;
	int xres, yres, bpp;
	int x, y;
	unsigned short *p;
	unsigned int *p2;
	
	/* 使能LCD */
	lcd_enable();

	/* 获得LCD的参数: fb_base, xres, yres, bpp */
	get_lcd_params(&fb_base, &xres, &yres, &bpp);

	/* 往framebuffer中写数据 */
	if (bpp == 16)
	{
		/* 让LCD输出整屏的红色 */

		/* ARGB565 */
		/*red*/
		p = (unsigned short *)fb_base;
		for (x = 0; x < xres; x++)
			for (y = 0; y < yres; y++)
				*p++ = 0xF800;

		/* green */
		p = (unsigned short *)fb_base;
		for (x = 0; x < xres; x++)
			for (y = 0; y < yres; y++)
				*p++ = 0x7E0;

		/* blue */
		p = (unsigned short *)fb_base;
		for (x = 0; x < xres; x++)
			for (y = 0; y < yres; y++)
				*p++ = 0x1f;
		
		/* black */
		p = (unsigned short *)fb_base;
		for (x = 0; x < xres; x++)
			for (y = 0; y < yres; y++)
				*p++ = 0;	
	}
	else if (bpp == 32)
	{
		/* 让LCD输出整屏的红色 */

		/* ARGB8888 */
		/*red*/
		p2 = (unsigned int *)fb_base;
		for (x = 0; x < xres; x++)
			for (y = 0; y < yres; y++)
				*p2++ = 0xff0000;

		/*green*/ 
		p2 = (unsigned int *)fb_base;
		for (x = 0; x < xres; x++)
			for (y = 0; y < yres; y++)
				*p2++ = 0x00ff00;

		/*blue*/ 
		p2 = (unsigned int *)fb_base;
		for (x = 0; x < xres; x++)
			for (y = 0; y < yres; y++)
				*p2++ = 0x0000ff;
		
		/* black */
		p2 = (unsigned int *)fb_base;
		for (x = 0; x < xres; x++)
			for (y = 0; y < yres; y++)
				*p2++ = 0;
	}

	/*************画点，画线，画圆实验************/
	
	fb_get_lcd_params();
	
	delay(100000);
     /* 画线 */
	draw_line(0, 0, xres - 1, 0, 0xff0000);
	delay(100000);
	draw_line(xres - 1, 0, xres - 1, yres - 1, 0xffff00);
	delay(100000);
	draw_line(0, yres - 1, xres - 1, yres - 1, 0xff00aa);
	delay(100000);
	draw_line(0, 0, 0, yres - 1, 0xff00ef);
	delay(100000);
	draw_line(0, 0, xres - 1, yres - 1, 0xff4500);
	delay(100000);
	draw_line(xres - 1, 0, 0, yres - 1, 0xff0780);
	delay(100000);
     
	 /* 画圆 */
	draw_circle(xres/2, yres/2, yres/4, 0xff00);
	
}



int main(void)
{
	/*初始化LCD相关*/
	lcd_init();

	/*LCD测试*/		
	lcd_test();

}

