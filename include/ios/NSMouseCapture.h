#ifndef Header_h
#define Header_h

struct SL_MouseCur{
	CGImageRef Image;
	NSPoint HotSpot;
}

#ifdef __cplusplus
extern "C" {
#endif
    void SLScreen_Capture_InitMouseCapture();
    SL_MouseCur SLScreen_Capture_GetCurrentMouseImage();
#ifdef __cplusplus
}
#endif


#endif 
