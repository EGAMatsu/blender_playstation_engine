#ifndef DEFFBMM
#define DEFFBMM


#define MAXFBA   64

#define FB_CLEAR 0
#define FB_ADD   1
#define FB_ALLOC 2

struct FrameBufferAlloc{
	char iu, name[15];
	int  sx, sy, ex, ey;
};

#endif /*  DEFFBMM  */
