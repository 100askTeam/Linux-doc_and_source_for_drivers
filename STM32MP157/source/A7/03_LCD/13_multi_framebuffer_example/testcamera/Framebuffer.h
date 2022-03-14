// vi:ts=4:shiftwidth=4

/*
 *
 * Framebuffer.h ---- GUI directly based on Linux Frame Buffer
 *
 * Author: wang ming-rong (wmrmail@163.com)
 *
 * Published on 2008-12-1
 *
 */

#ifndef __FRAME_BUFFER_H__
#define __FRAME_BUFFER_H__

#include <linux/fb.h>

#define BOOL int
#define HANDLE int
#define BYTE unsigned char
#define WORD unsigned short
#define LONG long
#define DWORD unsigned long
#define FLOAT float
#define RGB unsigned long
#define RGB16 unsigned short

#define INVALID_VALUE         (-1)
#define INVALID_HANDLE_VALUE ((HANDLE)INVALID_VALUE)
#define INVALID_ADDRESS     ((char*)INVALID_VALUE)

#define TRUE	1
#define FALSE 0

#define D1_WIDTH 	704
#define D1_HEIGHT 	576
#define CIF_WIDTH	(D1_WIDTH / 2)
#define CIF_HEIGHT	(D1_HEIGHT / 2)
#define QCFI_WIDTH	(CIF_WIDTH / 2)
#define QCIF_HEIGHT	(CIF_HEIGHT / 2)
#define VGA_WIDTH	640
#define VGA_HEIGHT	480
#define SIF_WIDTH	(VGA_WIDTH / 2)
#define SIF_HEIGHT	(VGA_HEIGHT / 2)
#define QSIF_WIDTH	(SIF_WIDTH / 2)
#define QSIF_HEIGHT	(SIF_HEIGHT / 2)

// define some color macro
#define BLACK       0x00000000
#define WHITE       0x00FFFFFF
#define GRAY        0x00808080
#define RED         0x00FF0000
#define GREEN       0x0000FF00
#define BLUE        0x000000FF
#define YELLOW      0x00FFFF00
#define PINK        0x00FF00FF
#define QING        0x0000FFFF

#define MAKERGB(r,g,b) (RGB)( ((r)<<16) | ((g)<<8) | (b) )
#define MAKEBGR(r,g,b) (RGB)( ((b)<<16) | ((g)<<8) | (r) )
#define MAKERGB565(r,g,b) (RGB16)( (((r)>>3)<<11) | (((g)>>2)<<5) | ((b)>>3) )
#define MAKEBGR565(r,g,b) (RGB16)( (((b)>>3)<<11) | (((g)>>2)<<5) | ((r)>>3) )
#define RGB_TO_RGB565(c) (RGB16)( ((((c)&0xff0000)>>19)<<11) | ((((c)&0xff00)>>10)<<5) | (((c)&0xff)>>3) )
#define RGB565_TO_RGB(c) (RGB)( (((c)&0x1f)<<3) | ((((c)>>5)&0x3f)<<10) | (((c)>>11)<<19)  | 0x070307)


typedef struct tagPOINT {
	long x, y;
} POINT, * LPPOINT;

typedef struct {
	long left, top, right, bottom;
} RECT, * LPRECT;

typedef struct _RECT {
	long left, top, width, height;
} RECTANGLE, * LPRECTANGLE;

#ifdef WIN32
#pragma pack(1) 
#define PACKED_ALIGN
#else
#define PACKED_ALIGN	__attribute__((packed))
#endif

typedef union {
	RGB rgb;
	struct {
		BYTE b, g, r, t;
	} PACKED_ALIGN val;
} PACKED_ALIGN COLORFMT;

#ifdef WIN32
#pragma pack()
#endif
#undef PACKED_ALIGN

RGB yuv2rgb(BYTE y, BYTE u, BYTE v);
RGB16 yuv2rgb565(BYTE y, BYTE u, BYTE v);

class CFramebuffer {
	static HANDLE m_fd;
	static char *m_pMem, *m_pVirtualMem[11];
	static int m_nDispMemNo, m_nPhysicalMemNum, m_nMemSize;
	static int m_nPhysicalWidth, m_nPhysicalHeight, m_nDepth, m_nLineLength, m_nBytesPerPixel;
	static int m_nVirtualX, m_nVirtualY, m_nVirtualWidth, m_nVirtualHeight, m_nVirtualLineLength;
	static struct fb_fix_screeninfo m_finfo;
	static struct fb_var_screeninfo m_vinfo;
	static BOOL m_bFlip; /* ·­×ª*/
	static int m_nInstance;

	int Open(char* pDevName=NULL);
	void Close();

public:
	CFramebuffer();
	~CFramebuffer();

	BOOL IsOpen();
	int ReOpen(char* pDevName);

	int Depth();
	int PhysicalWidth();
	int PhysicalHeight();
	int ScreenLeft();
	int ScreenTop();
	int ScreenWidth();
	int ScreenHeight();

	void Flip(BOOL bFlip=TRUE);
	BOOL GetFlipMode();

	void ClearDevice(RGB bgcolor=BLACK, int nMemMap = 0);
 
	void ExtendVirtualMem(int nVirtualMemNum=3);

	void InvalidateScreen(LPRECTANGLE pRect = NULL, int nMemMap = 0);
	void InvalidateScreen(int left, int top, int right, int bottom, int nMemMap = 0);
	
	char* GetVirtualMemory(int nMemMap=0);
};

#endif // __FRAME_BUFFER_H__

