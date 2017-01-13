//
//  Header.h
//  Screen_Capture
//
//  Created by scott lee on 1/11/17.
//
//

#ifndef Header_h
#define Header_h
#ifdef __cplusplus
extern "C" {
#endif
    void SLScreen_Capture_InitMouseCapture();
    CGImageRef SLScreen_Capture_GetCurrentMouseImage();
#ifdef __cplusplus
}
#endif


#endif /* Header_h */
