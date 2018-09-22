#ifndef Header_h
#define Header_h

struct SL_MouseCur{
	CGImageRef Image;
	int HotSpotx;
	int HotSpoty;
};

#ifdef __cplusplus
extern "C" {
#endif
    void SLScreen_Capture_InitMouseCapture();
    struct SL_MouseCur SLScreen_Capture_GetCurrentMouseImage();
#ifdef __cplusplus
}
#endif


#endif 
