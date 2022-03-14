#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>

static int fd_fb;
static struct fb_fix_screeninfo fix;	/* Current fix */
static struct fb_var_screeninfo var;	/* Current var */
static int screen_size;
static unsigned char *fb_base;
static unsigned int line_width;
static unsigned int pixel_width;

/**********************************************************************
 * 函数名称： lcd_put_pixel
 * 功能描述： 在LCD指定位置上输出指定颜色（描点）
 * 输入参数： x坐标，y坐标，颜色
 * 输出参数： 无
 * 返 回 值： 会
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2020/05/12	     V1.0	  zh(angenao)	      创建
 ***********************************************************************/ 
void lcd_put_pixel(void *fb_base, int x, int y, unsigned int color)
{
	unsigned char *pen_8 = fb_base+y*line_width+x*pixel_width;
	unsigned short *pen_16;	
	unsigned int *pen_32;	

	unsigned int red, green, blue;	

	pen_16 = (unsigned short *)pen_8;
	pen_32 = (unsigned int *)pen_8;

	switch (var.bits_per_pixel)
	{
		case 8:
		{
			*pen_8 = color;
			break;
		}
		case 16:
		{
			/* 565 */
			red   = (color >> 16) & 0xff;
			green = (color >> 8) & 0xff;
			blue  = (color >> 0) & 0xff;
			color = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);
			*pen_16 = color;
			break;
		}
		case 32:
		{
			*pen_32 = color;
			break;
		}
		default:
		{
			printf("can't surport %dbpp\n", var.bits_per_pixel);
			break;
		}
	}
}

void lcd_draw_screen(void *fb_base, unsigned int color)
{
	int x, y;
	for (x = 0; x < var.xres; x++)
		for (y = 0; y < var.yres; y++)
			lcd_put_pixel(fb_base, x, y, color);
}


/* ./multi_framebuffer_test single
 * ./multi_framebuffer_test double
 */
int main(int argc, char **argv)
{
	int i;
	int ret;
	int nBuffers;
	int nNextBuffer = 1;
	char *pNextBuffer;
	unsigned int colors[] = {0x00FF0000, 0x0000FF00, 0x000000FF, 0, 0x00FFFFFF};  /* 0x00RRGGBB */
	struct timespec time;

	time.tv_sec  = 0;
	time.tv_nsec = 100000000;

	if (argc != 2)
	{
		printf("Usage : %s <single|double>\n", argv[0]);
		return -1;
	}
	
	fd_fb = open("/dev/fb0", O_RDWR);
	if (fd_fb < 0)
	{
		printf("can't open /dev/fb0\n");
		return -1;
	}

	if (ioctl(fd_fb, FBIOGET_FSCREENINFO, &fix))
	{
		printf("can't get fix\n");
		return -1;
	}
	
	if (ioctl(fd_fb, FBIOGET_VSCREENINFO, &var))
	{
		printf("can't get var\n");
		return -1;
	}

	line_width  = var.xres * var.bits_per_pixel / 8;
	pixel_width = var.bits_per_pixel / 8;
	screen_size = var.xres * var.yres * var.bits_per_pixel / 8;

	nBuffers = fix.smem_len / screen_size;
	printf("nBuffers = %d\n", nBuffers);
	
	fb_base = (unsigned char *)mmap(NULL , fix.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd_fb, 0);
	if (fb_base == (unsigned char *)-1)
	{
		printf("can't mmap\n");
		return -1;
	}

	if ((argv[1][0] == 's') || (nBuffers == 1))
	{
		while (1)
		{
			/* use single buffer */
			for (i = 0; i < sizeof(colors)/sizeof(colors[0]); i++)
			{
				lcd_draw_screen(fb_base, colors[i]);
				nanosleep(&time, NULL);
			}
		}
	}
	else
	{
		/* use double buffer */
		/* a. enable use multi buffers */
		var.yres_virtual = nBuffers * var.yres;
		ioctl(fd_fb, FBIOPUT_VSCREENINFO, &var);

		while (1)
		{
			for (i = 0; i < sizeof(colors)/sizeof(colors[0]); i++)
			{
				/* get buffer */
				pNextBuffer =  fb_base + nNextBuffer * screen_size;

				/* set buffer */
				lcd_draw_screen(pNextBuffer, colors[i]);

				/* switch buffer */
				var.yoffset = nNextBuffer * var.yres;
				ioctl(fd_fb, FBIOPAN_DISPLAY, &var);

				ret = 0;
				ioctl(fd_fb, FBIO_WAITFORVSYNC, &ret);
				
				nNextBuffer = !nNextBuffer;
				nanosleep(&time, NULL);
			}
		}
		
	}
	
	munmap(fb_base , screen_size);
	close(fd_fb);
	
	return 0;	
}


