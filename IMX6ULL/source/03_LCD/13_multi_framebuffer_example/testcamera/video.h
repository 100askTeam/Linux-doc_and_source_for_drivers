/*
 * video.h
 */

#ifndef __VIDEO__H
#define __VIDEO__H

#include "Framebuffer.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Open and initialize video capture device */
HANDLE VideoOpen(int nCameraIndex);

/* Close device and release memory */
void VideoClose(HANDLE hVideo);

void* VideoGetSifRgbFrame(HANDLE hVideo, BOOL bFlip = FALSE, void* pFrameBuffer=NULL, int nWidth = 0, int nBps = 32);
void* VideoGetD1YFrame(HANDLE hVideo, BOOL bFlip = FALSE, void* pFrameBuffer=NULL);

void VideoSetBright(HANDLE hVideo, int nVal);

void VideoSetContrast(HANDLE hVideo, int nVal);

void VideoSetHue(HANDLE hVideo, int nVal);

void VideoSetSaturation(HANDLE hVideo, int nVal);

void VideoSetExposure(HANDLE hVideo, int nVal);

int VideoGetBright(HANDLE hVideo);

int VideoGetContrast(HANDLE hVideo);

int VideoGetHue(HANDLE hVideo);

int VideoGetSaturation(HANDLE hVideo);

int VideoGetExposure(HANDLE hVideo);

#ifdef __cplusplus
}
#endif

#endif /* __VIDEO_H__ */

