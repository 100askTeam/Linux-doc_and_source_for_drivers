/* video.cxx */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/videodev2.h>

#include "video.h"

#define VIDEO_DBG

#ifdef VIDEO_DBG
#define DBG(fmt, args...) printf(fmt, ##args)
#define ERR(fmt, args...) printf(fmt, ##args)
#else
#define DBG(fmt, args...)
#define ERR(fmt, args...)
#endif

#define V4L2_DEVICE_YUV 	"/dev/video0"
#define V4L2_DEVICE_RGB 	"/dev/video1"

/* Triple buffering for the capture driver */
#define NUM_CAPTURE_BUFS         4

/* Describes a capture frame buffer */
typedef struct CaptureBuffer {
	void         *start;
	size_t        length;
} CaptureBuffer;

typedef struct VIDEO
{
	int nCameraIndex;
	int hDevice;
	int vwidth, vheight;
	struct v4l2_capability cap;
	CaptureBuffer* capBufs;
	int numCapBufs;
	struct v4l2_buffer v4l2buf;
} VIDEO, *LPVIDEO;

int xioctl(int fd, int cmd, void* arg)
{
	int r;
	do {
		r = ioctl (fd, cmd, arg);
	} while (-1 == r && EINTR == errno);
	return r;
}

/******************************************************************************
 * initCaptureDevice
 ******************************************************************************/
static int initCaptureDevice(int nCameraIndex, LPVIDEO pVideo)
{
	struct v4l2_requestbuffers req;
	struct v4l2_format fmt;

	int i, fd = INVALID_HANDLE_VALUE;
	const char* pDeviceName = (nCameraIndex==0?V4L2_DEVICE_YUV:V4L2_DEVICE_RGB);
   
	/* Open video capture device */
	fd = open(pDeviceName, O_RDWR | O_NONBLOCK);
	if (fd == INVALID_HANDLE_VALUE) {
		ERR("Cannot open %s (%s)\r\n", pDeviceName, strerror(errno));
		return INVALID_HANDLE_VALUE;
	}

	if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &pVideo->cap)) {
		if (EINVAL == errno) {
			printf("%s is not V4L2 device\n", pDeviceName);
			close(fd);
			return INVALID_HANDLE_VALUE;
		} else {
			printf("VIDIOC_QUERYCAP fail.\r\n");
			memset(&pVideo->cap, 0, sizeof(pVideo->cap));
		}
	} else if (!(pVideo->cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		printf("%s is not capture device\r\n", pDeviceName);
	} else if (!(pVideo->cap.capabilities & V4L2_CAP_STREAMING)) {
		printf("%s doesn't support streaming i/o\r\n", pDeviceName);
	}


	memset(&req, 0, sizeof(req));
	req.count = 4;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	pVideo->vwidth = 640;
	pVideo->vheight = 480;
	if(pVideo->cap.capabilities) {
#if 0
		struct v4l2_cropcap cropcap;
		struct v4l2_crop crop;
		memset(&cropcap, 0, sizeof(cropcap));
		cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap)) {
			crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			crop.c = cropcap.defrect;	/* reset to default */
			if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop)) {
				switch(errno) {
				case EINVAL:
					/* Cropping not supported */
					break;
				default:
					/* Errors ignored */
					break;
				}
			}
		}
#endif

		struct v4l2_fmtdesc fmtDesc;
		memset(&fmtDesc, 0, sizeof(fmtDesc));
		fmtDesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (-1 == xioctl(fd, VIDIOC_ENUM_FMT, &fmtDesc))
			printf("VIDIOC_ENUM_FMT fail\r\n");

		memset(&fmt, 0, sizeof(fmt));
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		fmt.fmt.pix.width = pVideo->vwidth;
		fmt.fmt.pix.height = pVideo->vheight;
		fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
		fmt.fmt.pix.field = V4L2_FIELD_ANY;//V4L2_FIELD_INTERLACED;
		//fmt.fmt.pix.priv = 0x50020000;
		if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
			printf("VIDIOC_S_FMT fail\r\n");
#if 1	
		struct v4l2_streamparm fps;
		memset(&fps, 0, sizeof(struct v4l2_streamparm));
		fps.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		fps.parm.capture.timeperframe.numerator = 1;
		fps.parm.capture.timeperframe.denominator = 25;
		if(-1 == ioctl(fd, VIDIOC_S_PARM, &fps))
    			printf("VIDIOC_S_PARM frames fail.\r\n");
#endif
		if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
			if (EINVAL == errno) {
				printf("%s does not support memory mapping\r\n", pDeviceName);
			} else {
				printf("VIDIOC_REQBUFS fail.\r\n");
			}
		}

		if (req.count < 2) {
			req.count = 3;
			printf("Insufficient buffer memory on %s\n", pDeviceName);
		}
	}

	pVideo->capBufs = (CaptureBuffer*)malloc (req.count *sizeof(CaptureBuffer));
	if (pVideo->capBufs == NULL) {
		ERR("Malloc buffer %s out of memory\r\n", pDeviceName);
		close(fd);
		return INVALID_HANDLE_VALUE;
	}
	for(i = 0; i < (int)req.count; i++) {
		struct v4l2_buffer buf;
		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		if(pVideo->cap.capabilities) xioctl(fd, VIDIOC_QUERYBUF, &buf);
		if(buf.length == 0) {
			buf.length = pVideo->vwidth * pVideo->vheight * 2;
			buf.m.offset = 0;
		}

		pVideo->capBufs[i].length = buf.length;
		pVideo->capBufs[i].start = mmap (NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
		if (MAP_FAILED == pVideo->capBufs[i].start)
		{
			ERR("Map buffer %d  size %d for device %s fail (%s)\r\n", i, buf.length, pDeviceName, strerror(errno));
			break;
		}
	}
	req.count = i;

	if(i == 0) {
		close(fd);
		fd = INVALID_HANDLE_VALUE;
	} else if(pVideo->cap.capabilities) {
		for (i=0; i < (int)req.count; i++) {
			struct v4l2_buffer buf;
			memset(&buf, 0, sizeof(buf));
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
			buf.index = i;
			if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
				printf("VIDIOC_QBUF %d fail\r\n", i);
		}

		enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
			printf("VIDIOC_STREAMON fail.\r\n");
	}
	pVideo->numCapBufs = i;

	return fd;
}

/******************************************************************************
 * cleanupCaptureDevice
 ******************************************************************************/
static void cleanupCaptureDevice(LPVIDEO pVideo)
{
	/* Shut off the video capture */
	if(pVideo) {
		if(pVideo->cap.capabilities) {
			enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			if (-1 == xioctl(pVideo->hDevice, VIDIOC_STREAMOFF, &type))
				printf("VIDIOC_STREAMOFF fail.\r\n");
		}
		if (close(pVideo->hDevice) == -1) {
			ERR("Failed to close capture device (%s)\r\n", strerror(errno));
		}

		if(pVideo->capBufs) {
			for (int i = 0; i < pVideo->numCapBufs; i++) {
				if (munmap((char*)pVideo->capBufs[i].start, pVideo->capBufs[i].length) == -1) {
					ERR("Failed to unmap capture buffer %d\n", i);
				}
			}
			free(pVideo->capBufs);
		}
		usleep(40*1000);
	}
}

/*****************************************************************************************/
/* Application interface */
/*****************************************************************************************/

HANDLE VideoOpen(int nCameraIndex)
{
	/* Initialize the video capture device */
	LPVIDEO pVideo = (LPVIDEO)malloc(sizeof(VIDEO));
	if(pVideo==NULL) return INVALID_HANDLE_VALUE;
	memset(pVideo, 0, sizeof(VIDEO));
	pVideo->nCameraIndex = nCameraIndex;
	pVideo->hDevice = initCaptureDevice(nCameraIndex, pVideo);
	if(pVideo->hDevice == INVALID_HANDLE_VALUE) {
		free(pVideo);
		DBG("Open VIDEO capture device fail\r\n");
		return INVALID_HANDLE_VALUE;
	}
	return (HANDLE)pVideo;
}

void* VideoGetD1YFrame(HANDLE hVideo, BOOL bFlip, void* pFrameBuffer)
{
	static int D1BUF[2][D1_WIDTH*D1_HEIGHT];
	LPVIDEO pVideo = (LPVIDEO)hVideo;
	if(hVideo==INVALID_HANDLE_VALUE) return NULL;
	int i, j;
	if(pFrameBuffer==NULL) pFrameBuffer = (void*)D1BUF[pVideo->nCameraIndex>0];

	fd_set r;
	struct timeval tv;
	FD_ZERO(&r);
	FD_SET(pVideo->hDevice, &r);
	tv.tv_sec = 0;
	tv.tv_usec = 100*1000;
	select(pVideo->hDevice + 1, &r, NULL, NULL, &tv);
	
	/* Get a frame buffer with captured data */
	struct v4l2_buffer buf;
 	memset(&buf, 0, sizeof(buf));
	if(pVideo->numCapBufs) {
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		if (xioctl(pVideo->hDevice, VIDIOC_DQBUF, &buf) == -1 || (int)buf.index >= pVideo->numCapBufs) {
			if(errno != EAGAIN) ERR("VIDIOC_DQBUF failed (%s) !\r\n", strerror(errno));
			pFrameBuffer = NULL;
		}
	} else pFrameBuffer = NULL;

	if(pFrameBuffer) {
		char *pY = (char*)pFrameBuffer, *pYuv = (char*)pVideo->capBufs[buf.index].start;
		if(bFlip) pYuv += (pVideo->vwidth * pVideo->vheight - 1) * 2;
		else pYuv += pVideo->vwidth * (pVideo->vheight - 1) * 2;
		for(j = 0; j < pVideo->vheight; j++) {
			for(i = 0; i < pVideo->vwidth; i++) {			
				*pY = *pYuv;
				if(bFlip) {
					pYuv -= 2;
					pY += pVideo->vheight;
				} else {
					pYuv += 2;
					pY++;
				}
			}
			if(bFlip) pY -= pVideo->vwidth * pVideo->vheight - 1;
			else pYuv -= pVideo->vwidth * pVideo->vheight * 2 * 2;
		}
		if(pVideo->numCapBufs) xioctl(pVideo->hDevice, VIDIOC_QBUF, &buf);
	}

	return pFrameBuffer;
}

void* VideoGetSifRgbFrame(HANDLE hVideo, BOOL bFlip, void* pFrameBuffer, int nWidth, int nBps)
{
	static int D1BUF[2][D1_WIDTH*D1_HEIGHT];
	LPVIDEO pVideo = (LPVIDEO)hVideo;
	if(hVideo==INVALID_HANDLE_VALUE) return NULL;
	int i, j;
	if(pFrameBuffer==NULL) pFrameBuffer = (void*)D1BUF[pVideo->nCameraIndex>0];
	if(nWidth == 0) nWidth = 320;

	fd_set r;
	struct timeval tv;
	FD_ZERO(&r);
	FD_SET(pVideo->hDevice, &r);
	tv.tv_sec = 0;
	tv.tv_usec = 100*1000;
	select(pVideo->hDevice + 1, &r, NULL, NULL, &tv);
	
	/* Get a frame buffer with captured data */
	struct v4l2_buffer buf;
	memset(&buf, 0, sizeof(buf));
	if(pVideo->numCapBufs) {
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		if (xioctl(pVideo->hDevice, VIDIOC_DQBUF, &buf) == -1 || (int)buf.index >= pVideo->numCapBufs) {
			if (errno != EAGAIN) ERR("VIDIOC_DQBUF failed (%s)\r\n", strerror(errno));
			pFrameBuffer = NULL;
		}
	} else pFrameBuffer = NULL;

	if(pFrameBuffer) {
		if(nBps == 32) {
			unsigned char* puyvy = (unsigned char*)pVideo->capBufs[buf.index].start; 
			RGB* pRGB = (RGB*)pFrameBuffer;
			for(j = 0; j < 240; j++) {
				for(i = 0; i < 320; i++) {
					*pRGB = yuv2rgb(*puyvy, *(puyvy + 1), *(puyvy + 3));
					if(bFlip) pRGB += 240;
					else pRGB++;
					puyvy += 4;
				}
				puyvy += pVideo->vwidth * 2;
				pRGB += nWidth - 320;
				if(bFlip) pRGB -= 320 * 240 - 1;
			}
		} else {
			unsigned char* puyvy = (unsigned char*)pVideo->capBufs[buf.index].start; 
			RGB16* pRGB = (RGB16*)pFrameBuffer;
			if(bFlip) puyvy += (pVideo->vwidth * (pVideo->vheight - 1) - 1) * 2;
			else puyvy += pVideo->vwidth * (pVideo->vheight - 2) * 2;
			for(j = 0; j < 240; j++) {
				for(i = 0; i < 320; i++) {
					*pRGB = yuv2rgb565(*puyvy, *(puyvy+1), *(puyvy + 3));
					if(bFlip) {
						puyvy -= 4;
						pRGB += 240;
					} else {
						puyvy += 4;
						pRGB++;
					}
				}
				if(bFlip) {
					puyvy -= pVideo->vwidth * 2;
					pRGB -= nWidth * 240 - 1;
				} else {
					puyvy -= pVideo->vwidth * 3 * 2;
					pRGB += nWidth - 320;
				}
			}
		}

		if(pVideo->numCapBufs) xioctl(pVideo->hDevice, VIDIOC_QBUF, &buf);
	}
	
	return pFrameBuffer;
}

void VideoClose(HANDLE hVideo)
{
	LPVIDEO pVideo = (LPVIDEO)hVideo;
	if(hVideo != INVALID_HANDLE_VALUE) {
		cleanupCaptureDevice(pVideo);
		free(pVideo);
	}
}

void VideoSetBright(HANDLE hVideo, int nVal)
{
	int ret=-1;
	LPVIDEO pVideo = (LPVIDEO)hVideo;
	if(hVideo!=INVALID_HANDLE_VALUE) {
		struct v4l2_control ctrl;
		ctrl.id=V4L2_CID_BRIGHTNESS;
		ctrl.value=nVal;
		ret = ioctl(pVideo->hDevice, VIDIOC_S_CTRL, &ctrl);
	}
}

int VideoGetBright(HANDLE hVideo)
{
	int ret = -1;
	LPVIDEO pVideo = (LPVIDEO)hVideo;
	if(hVideo!=INVALID_HANDLE_VALUE) {
		struct v4l2_control ctrl;
		ctrl.id=V4L2_CID_BRIGHTNESS;
		if( ioctl(pVideo->hDevice, VIDIOC_G_CTRL, &ctrl) != -1 )
			ret = ctrl.value;
	}
	return ret;
}

void VideoSetContrast(HANDLE hVideo, int nVal)
{
	int ret = -1;
	LPVIDEO pVideo = (LPVIDEO)hVideo;
	if(hVideo!=INVALID_HANDLE_VALUE) {
		struct v4l2_control ctrl;
		ctrl.id=V4L2_CID_CONTRAST;
		ctrl.value=nVal;
		ret = ioctl(pVideo->hDevice, VIDIOC_S_CTRL, &ctrl);
	}
}

int VideoGetContrast(HANDLE hVideo)
{
	int ret = -1;
	LPVIDEO pVideo = (LPVIDEO)hVideo;
	if(hVideo!=INVALID_HANDLE_VALUE) {
		struct v4l2_control ctrl;
		ctrl.id=V4L2_CID_CONTRAST;
		if( ioctl(pVideo->hDevice, VIDIOC_G_CTRL, &ctrl) != -1 )
			ret = ctrl.value;
	}
	return ret;
}

void VideoSetHue(HANDLE hVideo, int nVal)
{
	int ret = -1;
	LPVIDEO pVideo = (LPVIDEO)hVideo;
	if(hVideo!=INVALID_HANDLE_VALUE) {
		struct v4l2_control ctrl;
		ctrl.id=V4L2_CID_HUE;
		ctrl.value=nVal;
		ret = ioctl(pVideo->hDevice, VIDIOC_S_CTRL, &ctrl);
	}
}

int VideoGetHue(HANDLE hVideo)
{
	int ret = -1;
	LPVIDEO pVideo = (LPVIDEO)hVideo;
	if(hVideo!=INVALID_HANDLE_VALUE) {
		struct v4l2_control ctrl;
		ctrl.id=V4L2_CID_HUE;	
		if( ioctl(pVideo->hDevice, VIDIOC_G_CTRL, &ctrl) != -1 )
			ret = ctrl.value;
	}
	return ret;
}

void VideoSetSaturation(HANDLE hVideo, int nVal)
{
	int ret = -1;
	LPVIDEO pVideo = (LPVIDEO)hVideo;
	if(hVideo!=INVALID_HANDLE_VALUE) {
		struct v4l2_control ctrl;
		ctrl.id=V4L2_CID_SATURATION;
		ctrl.value=nVal;
		ret = ioctl(pVideo->hDevice, VIDIOC_S_CTRL, &ctrl);
	}
}

int VideoGetSaturation(HANDLE hVideo)
{
	int ret = -1;
	LPVIDEO pVideo = (LPVIDEO)hVideo;
	if(hVideo!=INVALID_HANDLE_VALUE) {
		struct v4l2_control ctrl;
		ctrl.id=V4L2_CID_SATURATION;
		if( ioctl(pVideo->hDevice, VIDIOC_G_CTRL, &ctrl) != -1 )
			ret = ctrl.value;
	}
	return ret;
}

void VideoSetExposure(HANDLE hVideo, int nVal)
{
	LPVIDEO pVideo = (LPVIDEO)hVideo;
	if(hVideo != INVALID_HANDLE_VALUE) {
		if(pVideo->cap.capabilities) { // 标准驱动
			struct v4l2_control ctrl;
			ctrl.id=V4L2_CID_EXPOSURE;
			ctrl.value=nVal;
			ioctl(pVideo->hDevice, VIDIOC_S_CTRL, &ctrl);
		} else { // 汉王驱动
			ioctl(pVideo->hDevice, 300, &nVal);
		}
	}
}

int VideoGetExposure(HANDLE hVideo)
{
	int ret = -1;
	LPVIDEO pVideo = (LPVIDEO)hVideo;
	if(hVideo!=INVALID_HANDLE_VALUE) {
		struct v4l2_control ctrl;
		ctrl.id=V4L2_CID_EXPOSURE;
		if( ioctl(pVideo->hDevice, VIDIOC_G_CTRL, &ctrl) != -1 )
			ret = ctrl.value;
	}
	return ret;
}

