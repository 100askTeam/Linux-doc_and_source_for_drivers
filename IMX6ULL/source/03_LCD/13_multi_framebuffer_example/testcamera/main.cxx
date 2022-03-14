
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "video.h"

int main(int argc, char* argv[])
{
	int nFrameNo = 0, nFrameNum = 0, bFlip = FALSE;
	unsigned char *pFrame, *pScreen[4] =  { NULL };
	
	if(argc > 1) nFrameNum = atoi(argv[1]);
	if (nFrameNum > 10) nFrameNum = 10;
	printf("nFrameNum = %d\n", nFrameNum);

	CFramebuffer fb;
	if(fb.ScreenHeight() == 272) {
		bFlip = TRUE;
		fb.Flip(TRUE);
	}
	if(fb.IsOpen()) {
		printf("LCD: %dx%dx%d\r\n", fb.ScreenWidth(), fb.ScreenHeight(), fb.Depth());
		
		fb.ExtendVirtualMem(nFrameNum);
		for(nFrameNo = 0; nFrameNo < nFrameNum; nFrameNo++) {
			printf("GetVirtualMemory %d, %d\n", nFrameNo, nFrameNum);
			pScreen[nFrameNo] = (unsigned char*)fb.GetVirtualMemory(nFrameNo);
			printf("ClearDevice %d\n", nFrameNo);
			fb.ClearDevice(BLACK, nFrameNo);
			printf("InvalidateScreen %d\n", nFrameNo);
			fb.InvalidateScreen(NULL, nFrameNo);
		}
		nFrameNo = 0;
	} else printf("Framebuffer not support !\r\n");

	int nFps, nScreenOffset = (bFlip?((fb.ScreenWidth() - 240) / 2 * fb.ScreenHeight() + (fb.ScreenHeight() - 320) / 2):((fb.ScreenHeight() - 240) / 2 * fb.ScreenWidth() + (fb.ScreenWidth() - 320) / 2))*2;

	HANDLE hVideo = VideoOpen(0);
	if(hVideo != INVALID_HANDLE_VALUE) {
		time_t tCur, tLast = time(NULL), tFps = tLast;
		nFps = 0;
		while(1) {
			//tCur = time(NULL);
			//if((tCur - tLast) > 30) break;
			//else {
			{
				pFrame = (unsigned char*)VideoGetSifRgbFrame(hVideo, !bFlip, pScreen[nFrameNo] + nScreenOffset, bFlip?fb.ScreenHeight():fb.ScreenWidth(), fb.Depth());
				if(pFrame) {
					nFps++;
					fb.InvalidateScreen(NULL, nFrameNo);
					nFrameNo = (nFrameNo + 1) % nFrameNum;
				} else printf("NULL frame\r\n");
			}

			if(tCur != tFps) {
				printf("%d fps\r\n", nFps);
				tFps = tCur;
				nFps = 0;
			}

			usleep(10*1000);
		}
		VideoClose(hVideo);
	}

	return 0;
}


