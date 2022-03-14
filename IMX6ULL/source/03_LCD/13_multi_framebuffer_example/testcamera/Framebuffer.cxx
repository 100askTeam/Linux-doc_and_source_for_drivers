// vi:ts=4:shiftwidth=4

/*
 *
 * Framebuffer.cxx ---- GUI directly based on Linux Frame Buffer
 *
 * Author: wang ming-rong (wmrmail@163.com)
 *
 * Published on 2008-12-1
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <assert.h>

#include "Framebuffer.h"

#define FBIO_WAITFORVSYNC	_IOW('F', 0x20, __u32)

// static member initialize
HANDLE CFramebuffer::m_fd=INVALID_HANDLE_VALUE;
char * CFramebuffer::m_pMem=NULL, * CFramebuffer::m_pVirtualMem[11]={ NULL };
int CFramebuffer::m_nMemSize=0, CFramebuffer::m_nDispMemNo = 0, CFramebuffer::m_nPhysicalMemNum = 0;
int CFramebuffer::m_nPhysicalWidth=0, CFramebuffer::m_nPhysicalHeight=0, CFramebuffer::m_nDepth=0, CFramebuffer::m_nLineLength=0, CFramebuffer::m_nBytesPerPixel=2;
int CFramebuffer::m_nVirtualX=0, CFramebuffer::m_nVirtualY=0, CFramebuffer::m_nVirtualWidth=0, CFramebuffer::m_nVirtualHeight=0, CFramebuffer::m_nVirtualLineLength=0;

struct fb_fix_screeninfo CFramebuffer::m_finfo;
struct fb_var_screeninfo CFramebuffer::m_vinfo;
int CFramebuffer::m_nInstance=0;
BOOL CFramebuffer::m_bFlip = FALSE;

RGB yuv2rgb(BYTE y, BYTE u, BYTE v)
{
	int r, g, b;

	r = ( (y<<8)               + 359*(v-128) ) >> 8;
	g = ( (y<<8) -  88*(u-128) - 183*(v-128) ) >> 8;
	b = ( (y<<8) + 454*(u-128)               ) >> 8;

	if (r < 0)   r = 0;
	else if (r > 255) r = 255;
	if (g < 0)   g = 0;
	else if (g > 255) g = 255;
	if (b < 0)   b = 0;
	else if (b > 255) b = 255;

	return MAKERGB(r,g,b);
}

RGB16 yuv2rgb565(BYTE y, BYTE u, BYTE v)
{
	int r, g, b;

	r = ( (y<<8)               + 359*(v-128) ) >> 8;
	g = ( (y<<8) -  88*(u-128) - 183*(v-128) ) >> 8;
	b = ( (y<<8) + 454*(u-128)               ) >> 8;

	if (r < 0)   r = 0;
	else if (r > 255) r = 255;
	if (g < 0)   g = 0;
	else if (g > 255) g = 255;
	if (b < 0)   b = 0;
	else if (b > 255) b = 255;

	return MAKERGB565(r,g,b);
}

////////////////////////////////////////////////////////////////////////////////////////////
//
// CFramebuffer
//

CFramebuffer::CFramebuffer()
{
	Open();
}

CFramebuffer::~CFramebuffer()
{
	Close();
}

int CFramebuffer::ReOpen(char* pDevName)
{
	Close();
	return Open(pDevName);
}

int CFramebuffer::Open(char* pDevName)
{
	// Open FrameBuffer for reading and writing
	if(pDevName==NULL) pDevName = (char *)"/dev/fb";
	if(m_nInstance++==0) {
		HANDLE fbfd, Err = 0;
		m_nPhysicalMemNum = 0;

		fbfd = open(pDevName, O_RDWR);
		if(fbfd == INVALID_HANDLE_VALUE && strcmp(pDevName, "/dev/fb0")) {
			int fd;
			Err = errno;
			fd = open("/dev/fb0", O_RDWR);
			if(fd != INVALID_HANDLE_VALUE) {
				pDevName = "/dev/fb0";
				fbfd = fd;
				Err = 0;
			} else if(strcmp(pDevName, "/dev/fb/0")) {
				fd = open("/dev/fb/0", O_RDWR);
				if(fd != INVALID_HANDLE_VALUE) {
					pDevName = "/dev/fb/0";
					fbfd = fd;
					Err = 0;
				}
			}
			errno = Err;
		}
		if ( fbfd==INVALID_HANDLE_VALUE ) Err=errno;
		else {	
			// Get fixed screen information
			if ( ioctl(fbfd, FBIOGET_FSCREENINFO, &m_finfo) ) Err=errno;
			else {
				// Get variable screen information
				if ( ioctl(fbfd, FBIOGET_VSCREENINFO, &m_vinfo) )  Err=errno;
				else {
					// Figure out the size of the screen in bytes
					struct fb_var_screeninfo vinfo;
					memcpy(&vinfo, &m_vinfo, sizeof(struct fb_var_screeninfo));
//printf("finfo: line_length=%d, smem_len=%d\r\n", m_finfo.line_length, m_finfo.smem_len); // 960
//printf("vinfo: %d x %d x %d\r\n", m_vinfo.xres, m_vinfo.yres, m_vinfo.bits_per_pixel); // 240 x 320 x 32
					m_nPhysicalWidth=m_vinfo.xres;


					m_nVirtualX=m_nVirtualY=0;
					m_nVirtualWidth=vinfo.xres;

					m_nPhysicalHeight=m_vinfo.yres;
					m_nVirtualHeight=vinfo.yres;
					
					m_nDepth=vinfo.bits_per_pixel;
					m_nBytesPerPixel=(m_nDepth+7)/8;
					m_nVirtualLineLength=m_nLineLength=m_nPhysicalWidth*m_nBytesPerPixel;
					m_nMemSize=m_nLineLength*m_nPhysicalHeight;
					m_nPhysicalMemNum = m_finfo.smem_len / m_nMemSize;
					if(m_vinfo.yres_virtual < (unsigned int)(m_nPhysicalMemNum * m_nPhysicalHeight)) {
						m_vinfo.yres_virtual = m_nPhysicalMemNum * m_nPhysicalHeight;
						ioctl(fbfd, FBIOPUT_VSCREENINFO, &m_vinfo);
					}
					printf("MemLen=%d, frameSize=%d, frames=%d\r\n", m_finfo.smem_len, m_nMemSize, m_nPhysicalMemNum);

					// Map the device to memory
					m_pMem = (char *)mmap(0, m_finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
					if (m_pMem==INVALID_ADDRESS) {
						m_pMem=NULL;
						m_nPhysicalMemNum = 0;
						Err=errno;
					}
					else
					{
						memset(m_pMem, 0x00, m_finfo.smem_len);
						usleep(10*1000);
						m_nDispMemNo = 0;
#if 1
						int i;
						//set the working buffer for the displayed buffer
						for(i = 0; i < m_nPhysicalMemNum; i++) {
							m_vinfo.yoffset = i * m_nPhysicalHeight;
							if(ioctl(fbfd, FBIOPAN_DISPLAY, &m_vinfo) == -1) {
								printf("FBIOPAN_DISPLAY frame %d fail.\r\n", i);
								break;
							}
else printf("FBIOPAN_DISPLAY frame %d OK\r\n", i);
							m_nDispMemNo = i;
						}
						if(i == 0) i = 1;
						m_nPhysicalMemNum = i;
#endif
					}
				}
			}
			if(Err) ::close(fbfd);
		}
		if(Err) return Err;
		m_fd=fbfd;

		m_pVirtualMem[0] = m_pMem; /* true display buffer */
		if(m_nPhysicalMemNum > 1) {
			if(m_nPhysicalMemNum > 10) m_nPhysicalMemNum = 10;
			for(int i = 1; i < m_nPhysicalMemNum; i++) {
				m_pVirtualMem[i] = m_pMem + i * m_nMemSize;
			}
		}
	
  		if(m_pVirtualMem[10] == NULL) {
			m_pVirtualMem[10] = (char*)malloc(m_nMemSize); /* reserved display buffer */
			if(m_pVirtualMem[10]) memset(m_pVirtualMem[10], 0, m_nMemSize);
  		}
 	}
	
	ExtendVirtualMem(3);

	return 0;
}

void CFramebuffer::Close()
{
	if( --m_nInstance == 0 ) {
		if(m_pMem) munmap(m_pMem,m_finfo.smem_len); m_pMem=NULL;
		for(int i = 1; i < 11; i++) {
			if((i >= m_nPhysicalMemNum) && m_pVirtualMem[i]) free(m_pVirtualMem[i]);
			m_pVirtualMem[i]=NULL;
		}
		if(m_fd!=INVALID_HANDLE_VALUE) ::close(m_fd); m_fd=INVALID_HANDLE_VALUE;
	}
}

BOOL CFramebuffer::IsOpen()
{
	return ( m_fd != INVALID_HANDLE_VALUE );
}

void CFramebuffer::ExtendVirtualMem(int nVirtualMemNum)
{
	int i;
	if(m_fd==INVALID_HANDLE_VALUE) return;

	if(nVirtualMemNum>10) nVirtualMemNum = 10;
	for(i=0; i<nVirtualMemNum; i++) {
		if(m_pVirtualMem[i]==NULL) {
			m_pVirtualMem[i]=(char*)malloc(m_nMemSize);
			if(m_pVirtualMem[i]) memset(m_pVirtualMem[i], 0, m_nMemSize);
			else break;
		}
	}
}

int CFramebuffer::PhysicalWidth()
{
	int nWidth;
	if(m_bFlip==FALSE) nWidth=m_nPhysicalWidth;
	else nWidth=m_nPhysicalHeight;
	return nWidth;
}

int CFramebuffer::PhysicalHeight()
{
	int nHeight;
	if(m_bFlip==FALSE) nHeight=m_nPhysicalHeight;
	else nHeight=m_nPhysicalWidth;
	return nHeight;
}

int CFramebuffer::ScreenLeft()
{
	int x;
	if(m_bFlip==FALSE) x = m_nVirtualX;
	else x = m_nVirtualY;
	return x;
}

int CFramebuffer::ScreenTop()
{
	int y;
	if(m_bFlip==FALSE) y = m_nVirtualY;
	else y = m_nVirtualX;
	return y;
}

int CFramebuffer::ScreenWidth()
{
	int w;
	if(m_bFlip==FALSE) w = m_nVirtualWidth;
	else w = m_nVirtualHeight;
	return w;
}

int CFramebuffer::ScreenHeight()
{
	int h;
	if(m_bFlip==FALSE) h = m_nVirtualHeight;
	else h = m_nVirtualWidth;
	return h;
}

int CFramebuffer::Depth()
{
	return m_nDepth;
}

void CFramebuffer::Flip(BOOL bFlip)
{
	m_bFlip = bFlip;
}

BOOL CFramebuffer::GetFlipMode()
{
	return m_bFlip;
}

void CFramebuffer::ClearDevice(RGB bgcolor, int nMemMap)
{
	int nWidth=m_nVirtualWidth, nHeight=m_nVirtualHeight, nLineLen=m_nLineLength;
	char* pDstMem = ((nMemMap>=0 && nMemMap<11) ?m_pVirtualMem[nMemMap]:NULL);
	if(pDstMem==NULL) return;

	pDstMem+=m_nVirtualY*m_nLineLength+m_nVirtualX*m_nBytesPerPixel;

	if(pDstMem) {
		if(m_nDepth==4) bgcolor+=bgcolor<<4;
		if(m_nDepth<=8 || m_nDepth==24 && (((bgcolor & 0xFF0000)>>16)==((bgcolor & 0xFF00)>>8)==(bgcolor & 0x00FF)) ) {
			for(int i=0;i<nHeight;i++) {
				memset(pDstMem, bgcolor & 0xFF, nLineLen);
				pDstMem+=nLineLen;
			}
		} else if(m_nDepth==16) {
			RGB16 bc=RGB_TO_RGB565(bgcolor);
			for(int i=0;i<nHeight;i++) {
				for(int j=0;j<nWidth;j++) {
					*(WORD*)(pDstMem+j*sizeof(WORD))=bc;
				}
				pDstMem+=nLineLen;
			}
		} else {
			BYTE r = bgcolor>>16, g = (bgcolor>>8) & 0xff, b = bgcolor & 0xff;
			for(int i=0;i<nHeight;i++) {
				for(int j=0;j<nWidth;j++) {
					*pDstMem++=b;
					*pDstMem++=g;
					*pDstMem++=r;
					if(m_nDepth==32) *pDstMem++=0; // apha byte
				}
				pDstMem+=nLineLen-nWidth*m_nBytesPerPixel;
			}
		}
	}
}

void CFramebuffer::InvalidateScreen(LPRECTANGLE pRect, int nMemMap)
{
	if(pRect==NULL) {
		int ret = -1;
		if(nMemMap >= 0 && nMemMap < m_nPhysicalMemNum) {
			if(nMemMap != m_nDispMemNo) {				
				//set the working buffer for the displayed buffer
				m_vinfo.yoffset = nMemMap * m_nPhysicalHeight;
				ret = ioctl(m_fd, FBIOPAN_DISPLAY, &m_vinfo);
				if(ret == 0) {
					m_nDispMemNo = nMemMap;
					ioctl(m_fd, FBIO_WAITFORVSYNC, &ret);
				}
			} else ret = 0;
		}
		if(ret) {
			char* pDst = m_pMem + (m_nDispMemNo * m_nMemSize), *pSrc = ((nMemMap>=0 && nMemMap<11) ?m_pVirtualMem[nMemMap]:NULL);
			if(pSrc && pDst && pSrc != pDst) {
				memcpy(pDst, pSrc, m_nPhysicalHeight*m_nLineLength);
			}
		}
	} else InvalidateScreen(pRect->left,pRect->top,pRect->left+pRect->width-1,pRect->top+pRect->height-1, nMemMap);
}

void CFramebuffer::InvalidateScreen(int left, int top, int right, int bottom, int nMemMap)
{
	char* pDst = m_pMem + (m_nDispMemNo * m_nMemSize), *pSrc = ((nMemMap>=0 && nMemMap<11) ?m_pVirtualMem[nMemMap]:NULL);
	if(pSrc && pDst && pSrc != pDst) {
		int i, width, height, widthbytes, nLineLength=m_nLineLength, nMaxWidth=m_nVirtualWidth, nMaxHeight=m_nVirtualHeight;

		if(left>=nMaxWidth || right<0 || left>right || top>=nMaxHeight || bottom<0 || top>bottom) return;

		if(left<0) left=0;
		if(right <= 0 || right>=nMaxWidth) right=nMaxWidth-1;
		if(top<0) top=0;
		if(bottom <= 0 || bottom>=nMaxHeight) bottom=nMaxHeight-1;
		width=right-left+1;
		height=bottom-top+1;
		widthbytes=width*m_nBytesPerPixel;
		pDst += (m_nVirtualY+top)*nLineLength + left*m_nBytesPerPixel;
		pSrc += (m_nVirtualY+top)*nLineLength + left*m_nBytesPerPixel;
		if(width==m_nPhysicalWidth) memcpy(pDst, pSrc, height*nLineLength);
		else
		{
			for(i=0; i<height; i++) {
				memcpy(pDst, pSrc, widthbytes);
				pDst += nLineLength;
				pSrc += nLineLength;
			}
		}
	}
}

char* CFramebuffer::GetVirtualMemory(int nMemMap)
{
	return m_pVirtualMem[nMemMap];
}

