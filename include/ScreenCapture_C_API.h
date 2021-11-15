//
// Created by ptwohig https://github.com/ptwohig on 10/17/21.
//

#ifndef SCREEN_CAPTURE_LITE_BUILD_SCREENCAPTURE_C_API_H
#define SCREEN_CAPTURE_LITE_BUILD_SCREENCAPTURE_C_API_H

#if defined(WINDOWS) || defined(WIN32)
#if defined(SC_LITE_DLL) && defined(__cplusplus)
#define SC_LITE_C_EXTERN extern "C" __declspec(dllexport)
#else
#define SC_LITE_C_EXTERN
#endif
#else
#if defined(SC_LITE_DLL) && defined(__cplusplus)
#define SC_LITE_C_EXTERN extern "C"
#else
#define SC_LITE_C_EXTERN
#endif
#endif

#if defined(__cplusplus)

namespace SL::Screen_Capture
{

struct Image;
struct Monitor;
struct Window;
struct MousePoint;

namespace C_API
{

struct IScreenCaptureManagerWrapper;
struct ICaptureConfigurationScreenCaptureCallbackWrapper;
struct ICaptureConfigurationWindowCaptureCallbackWrapper;

} // namespace C_API

}

typedef SL::Screen_Capture::Image* SCL_ImageRef;
typedef SL::Screen_Capture::Window* SCL_WindowRef;
typedef SL::Screen_Capture::Monitor* SCL_MonitorRef;
typedef SL::Screen_Capture::MousePoint* SCL_MousePointRef;

typedef SL::Screen_Capture::Image const* SCL_ImageRefConst;
typedef SL::Screen_Capture::Window const* SCL_WindowRefConst;
typedef SL::Screen_Capture::Monitor const* SCL_MonitorRefConst;
typedef SL::Screen_Capture::MousePoint const* SCL_MousePointRefConst;

typedef SL::Screen_Capture::C_API::IScreenCaptureManagerWrapper* SCL_IScreenCaptureManagerWrapperRef;
typedef SL::Screen_Capture::C_API::ICaptureConfigurationScreenCaptureCallbackWrapper* SCL_ICaptureConfigurationScreenCaptureCallbackWrapperRef;
typedef SL::Screen_Capture::C_API::ICaptureConfigurationWindowCaptureCallbackWrapper* SCL_ICaptureConfigurationWindowCaptureCallbackWrapperRef;

#else

typedef void* SCL_ImageRef;
typedef void* SCL_WindowRef;
typedef void* SCL_MonitorRef;
typedef void* SCL_MousePointRef;

typedef void const* SCL_ImageRefConst;
typedef void const* SCL_WindowRefConst;
typedef void const* SCL_MonitorRefConst;
typedef void const* SCL_MousePointRefConst;

typedef void* SCL_IScreenCaptureManagerWrapperRef;
typedef void* SCL_ICaptureConfigurationScreenCaptureCallbackWrapperRef;
typedef void* SCL_ICaptureConfigurationWindowCaptureCallbackWrapperRef;

#endif

typedef int (*SCL_ScreenCaptureCallback)(SCL_ImageRefConst img, SCL_MonitorRefConst monitor);
typedef int (*SCL_ScreenCaptureCallbackWithContext)(SCL_ImageRefConst img, SCL_MonitorRefConst monitor, void *context);

typedef int (*SCL_MouseCaptureCallback)(SCL_ImageRefConst img, SCL_MousePointRefConst mouse);
typedef int (*SCL_MouseCaptureCallbackWithContext)(SCL_ImageRefConst img, SCL_MousePointRefConst mouse, void *context);

typedef int (*SCL_WindowCaptureCallback)(SCL_ImageRefConst img, SCL_WindowRefConst monitor);
typedef int (*SCL_WindowCaptureCallbackWithContext)(SCL_ImageRefConst img, SCL_WindowRefConst monitor, void *context);

typedef int (*SCL_WindowCallback)(SCL_WindowRef buffer, int buffersize);
typedef int (*SCL_MonitorCallback)(SCL_MonitorRef buffer, int buffersize);

typedef int (*SCL_WindowCallbackWithContext)(SCL_WindowRef buffer, int buffersize, void *context);
typedef int (*SCL_MonitorCallbackWithContext)(SCL_MonitorRef buffer, int buffersize, void *context);

#ifdef __cplusplus
extern "C"
{
#endif

//SCL_GetWindows and SCL_GetMonitors expect a pre allocated buffer with the size as the second input parameter.
//The output of these functions is the actual total number of elements that the library had to return. So, applications should use this value in determininng how to preallocate data.
SC_LITE_C_EXTERN
int SCL_GetWindows(SCL_WindowRef windows, int windows_size);

SC_LITE_C_EXTERN
int SCL_GetMonitors(SCL_MonitorRef monitors, int monitors_size);

SC_LITE_C_EXTERN
int SCL_IsMonitorInsideBounds(SCL_MonitorRef monitors, int monitorsize, SCL_MonitorRef monitor);

SC_LITE_C_EXTERN
void SCL_MonitorOnNewFrame(SCL_ICaptureConfigurationScreenCaptureCallbackWrapperRef ptr, SCL_ScreenCaptureCallback cb);

SC_LITE_C_EXTERN
void SCL_MonitorOnNewFrameWithContext(SCL_ICaptureConfigurationScreenCaptureCallbackWrapperRef ptr, SCL_ScreenCaptureCallbackWithContext cb);

SC_LITE_C_EXTERN
void SCL_MonitorOnFrameChanged(SCL_ICaptureConfigurationScreenCaptureCallbackWrapperRef ptr, SCL_ScreenCaptureCallback cb);

SC_LITE_C_EXTERN
void SCL_MonitorOnFrameChangedWithContext(SCL_ICaptureConfigurationScreenCaptureCallbackWrapperRef ptr, SCL_ScreenCaptureCallbackWithContext cb);

SC_LITE_C_EXTERN
void SCL_MonitorOnMouseChanged(SCL_ICaptureConfigurationScreenCaptureCallbackWrapperRef ptr, SCL_MouseCaptureCallback cb);

SC_LITE_C_EXTERN
void SCL_MonitorOnMouseChangedWithContext(SCL_ICaptureConfigurationScreenCaptureCallbackWrapperRef ptr, SCL_MouseCaptureCallbackWithContext cb);

SC_LITE_C_EXTERN
SCL_IScreenCaptureManagerWrapperRef SCL_MonitorStartCapturing(SCL_ICaptureConfigurationScreenCaptureCallbackWrapperRef ptr);

SC_LITE_C_EXTERN
void SCL_FreeIScreenCaptureManagerWrapper(SCL_IScreenCaptureManagerWrapperRef ptr);

SC_LITE_C_EXTERN
void SCL_SetFrameChangeInterval(SCL_IScreenCaptureManagerWrapperRef ptr, int milliseconds);

SC_LITE_C_EXTERN
void SCL_SetMouseChangeInterval(SCL_IScreenCaptureManagerWrapperRef ptr, int milliseconds);

SC_LITE_C_EXTERN
void SCL_PauseCapturing(SCL_IScreenCaptureManagerWrapperRef ptr);

SC_LITE_C_EXTERN
int SCL_IsPaused(SCL_IScreenCaptureManagerWrapperRef ptr);

SC_LITE_C_EXTERN
void SCL_Resume(SCL_IScreenCaptureManagerWrapperRef ptr);

SC_LITE_C_EXTERN
void SCL_WindowOnNewFrame(SCL_ICaptureConfigurationWindowCaptureCallbackWrapperRef ptr, SCL_WindowCaptureCallback cb);

SC_LITE_C_EXTERN
void SCL_WindowOnNewFrameWithContext(SCL_ICaptureConfigurationWindowCaptureCallbackWrapperRef ptr, SCL_WindowCaptureCallbackWithContext cb);

SC_LITE_C_EXTERN
void SCL_WindowOnFrameChanged(SCL_ICaptureConfigurationWindowCaptureCallbackWrapperRef  ptr, SCL_WindowCaptureCallback cb);

SC_LITE_C_EXTERN
void SCL_WindowOnFrameChangedWithContext(SCL_ICaptureConfigurationWindowCaptureCallbackWrapperRef ptr, SCL_WindowCaptureCallbackWithContext cb);

SC_LITE_C_EXTERN
void SCL_WindowOnMouseChanged(SCL_ICaptureConfigurationWindowCaptureCallbackWrapperRef ptr, SCL_MouseCaptureCallback cb);

SC_LITE_C_EXTERN
void SCL_WindowOnMouseChangedWithContext(SCL_ICaptureConfigurationWindowCaptureCallbackWrapperRef ptr, SCL_MouseCaptureCallbackWithContext cb);

SC_LITE_C_EXTERN
SCL_IScreenCaptureManagerWrapperRef SCL_WindowStartCapturing(SCL_ICaptureConfigurationWindowCaptureCallbackWrapperRef ptr);

SC_LITE_C_EXTERN
unsigned char* SCL_Utility_CopyToContiguous(unsigned char* destination, SCL_ImageRefConst image);

SC_LITE_C_EXTERN
SCL_ICaptureConfigurationScreenCaptureCallbackWrapperRef
SCL_CreateMonitorCaptureConfiguration(SCL_MonitorCallback monitorstocapture);

SC_LITE_C_EXTERN
SCL_ICaptureConfigurationScreenCaptureCallbackWrapperRef
SCL_CreateMonitorCaptureConfigurationWithContext(SCL_MonitorCallbackWithContext monitorstocapture, void *context);

SC_LITE_C_EXTERN
void SCL_FreeMonitorCaptureConfiguration(SCL_ICaptureConfigurationScreenCaptureCallbackWrapperRef ptr);

SC_LITE_C_EXTERN
SCL_ICaptureConfigurationWindowCaptureCallbackWrapperRef
SCL_CreateWindowCaptureConfiguration(SCL_WindowCallback windowstocapture);

SC_LITE_C_EXTERN
SCL_ICaptureConfigurationWindowCaptureCallbackWrapperRef
SCL_CreateWindowCaptureConfigurationWithContext(SCL_WindowCallbackWithContext windowstocapture, void *context);

SC_LITE_C_EXTERN
void SCL_FreeWindowCaptureConfiguration(SCL_ICaptureConfigurationWindowCaptureCallbackWrapperRef ptr);

SC_LITE_C_EXTERN
SCL_ICaptureConfigurationScreenCaptureCallbackWrapperRef
SCL_CreateMonitorCaptureConfigurationWithContext(SCL_MonitorCallbackWithContext monitorstocapture, void *context);

SC_LITE_C_EXTERN
void SCL_FreeMonitorCaptureConfiguration(SCL_ICaptureConfigurationScreenCaptureCallbackWrapperRef ptr);

SC_LITE_C_EXTERN
SCL_ICaptureConfigurationWindowCaptureCallbackWrapperRef
SCL_CreateWindowCaptureConfiguration(SCL_WindowCallback windowstocapture);

SC_LITE_C_EXTERN
SCL_ICaptureConfigurationWindowCaptureCallbackWrapperRef
SCL_CreateWindowCaptureConfigurationWithContext(SCL_WindowCallbackWithContext windowstocapture, void *context);

SC_LITE_C_EXTERN
void SCL_FreeWindowCaptureConfiguration(SCL_ICaptureConfigurationWindowCaptureCallbackWrapperRef ptr);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SCREEN_CAPTURE_LITE_BUILD_SCREENCAPTURE_SCL_H
