#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "font.h"

#define OLED_SET_XY	  		99
#define OLED_SET_XY_WRITE_DATA  100
#define OLED_SET_XY_WRITE_DATAS 101
#define OLED_SET_DATAS	  		102  /* 102为低8位, 高16位用来表示长度 */

int fd;

/**********************************************************************
	  * 函数名称： OLED_DIsp_Char
	  * 功能描述：在某个位置显示字符 1-9
	  * 输入参数：@ x ：要显示的column address
		 			@y :要显示的page address
		 			@c ：要显示的字符的ascii码
	  * 输出参数： 无
	  * 返 回 值： 
	  * 修改日期		版本号	  修改人 	   修改内容
	  * -----------------------------------------------
	  * 2020/03/15		  V1.0	   芯晓		   创建
***********************************************************************/
void OLED_DIsp_Char(int x, int y, unsigned char c)
{
	int i = 0;
	/* 得到字模 */
	const unsigned char *dots = oled_asc2_8x16[c - ' '];
	char pos[2];
#if 0
	/* 发给OLED */
	OLED_DIsp_Set_Pos(x, y);
	/* 发出8字节数据 */
	for (i = 0; i < 8; i++)
		oled_write_cmd_data(dots[i], OLED_DATA);
#endif
	pos[0] = x;
	pos[1] = y;

	ioctl(fd, OLED_SET_XY, pos);
	ioctl(fd, OLED_SET_DATAS | (8<<8), dots);

#if 0
	OLED_DIsp_Set_Pos(x, y+1);
	/* 发出8字节数据 */
	for (i = 0; i < 8; i++)
		oled_write_cmd_data(dots[i+8], OLED_DATA);
#endif	
	pos[0] = x;
	pos[1] = y+1;

	ioctl(fd, OLED_SET_XY, pos);
	ioctl(fd, OLED_SET_DATAS | (8<<8), &dots[8]);

}



/**********************************************************************
	 * 函数名称： OLED_DIsp_String
	 * 功能描述： 在指定位置显示字符串
	 * 输入参数：@ x ：要显示的column address
		 			@y :要显示的page address
		 			@str ：要显示的字符串
	 * 输出参数： 无
	 * 返 回 值： 无
	 * 修改日期 	   版本号	 修改人		  修改内容
	 * -----------------------------------------------
	 * 2020/03/15		 V1.0	  芯晓		  创建
***********************************************************************/
void OLED_DIsp_String(int x, int y, char *str)
{
	unsigned char j=0;
	while (str[j])
	{		
		OLED_DIsp_Char(x, y, str[j]);//显示单个字符
		x += 8;
		if(x > 127)
		{
			x = 0;
			y += 2;
		}//移动显示位置
		j++;
	}
}

/**********************************************************************
	 * 函数名称： OLED_DIsp_CHinese
	 * 功能描述：在指定位置显示汉字
	 * 输入参数：@ x ：要显示的column address
		 			@y :要显示的page address
		 			@chr ：要显示的汉字，三个汉字“百问网”中选择一个
	 * 输出参数： 无
	 * 返 回 值： 无
	 * 修改日期 	   版本号	 修改人		  修改内容
	 * -----------------------------------------------
	 * 2020/03/15		 V1.0	  芯晓		  创建
 ***********************************************************************/
void OLED_DIsp_CHinese(unsigned char x,unsigned char y,unsigned char no)
{      			    
	unsigned char t, adder=0;
	char pos[2];

	pos[0] = x;
	pos[1] = y;
	
	ioctl(fd, OLED_SET_XY, pos);
	
    for(t=0;t<16;t++)
	{//显示上半截字符	
		//oled_write_cmd_data(hz_1616[no][t*2],OLED_DATA);
		ioctl(fd, OLED_SET_DATAS | (1<<8), &hz_1616[no][t*2]);
		
		adder+=1;
    }	
	
	pos[0] = x;
	pos[1] = y+1;
	
	ioctl(fd, OLED_SET_XY, pos);
	
    for(t=0;t<16;t++)
	{//显示下半截字符
		//oled_write_cmd_data(hz_1616[no][t*2+1],OLED_DATA);
		ioctl(fd, OLED_SET_DATAS | (1<<8), &hz_1616[no][t*2+1]);
		
		adder+=1;
    }					
}

/**********************************************************************
	 * 函数名称： OLED_DIsp_Test
	 * 功能描述： 整个屏幕显示测试
	 * 输入参数：无
	 * 输出参数： 无
	 * 返 回 值： 无
	 * 修改日期 	   版本号	 修改人		  修改内容
	 * -----------------------------------------------
	 * 2020/03/15		 V1.0	  芯晓		  创建
 ***********************************************************************/
void OLED_DIsp_Test(void)
{ 	
	int i;
	
	OLED_DIsp_String(0, 0, "wiki.100ask.net");
	OLED_DIsp_String(0, 2, "book.100ask.net");
	OLED_DIsp_String(0, 4, "bbs.100ask.net");
	
	for(i = 0; i < 3; i++)
	{   //显示汉字 百问网
		OLED_DIsp_CHinese(32+i*16, 6, i);
	}
} 


/*
 * oled_test /dev/myoled
 */

int main(int argc, char **argv)
{
	int buf[2];

	if (argc != 2)
	{
		printf("Usage: %s <dev>\n", argv[0]);
		return -1;
	}
	
	fd = open(argv[1], O_RDWR);
	if (fd < 0)
	{
		printf(" can not open %s\n", argv[1]);
		return -1;
	}

	OLED_DIsp_Test();
	
	return 0;
}


