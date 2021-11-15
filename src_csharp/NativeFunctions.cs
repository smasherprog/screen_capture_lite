using System;
using System.Runtime.InteropServices;

namespace SCL
{

    public static class NativeFunctions
    {
            
        [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SCL_GetMonitors(IntPtr buffer, int buffer_size);
        
        [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SCL_GetWindows(IntPtr buffer, int buffer_size);
        
        [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr SCL_CreateWindowCaptureConfiguration(BufferCallback callback);
        
        [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr SCL_CreateWindowCaptureConfigurationWithContext(MonitorWindowCallbackWithContext callback, IntPtr context);
        
        [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr SCL_CreateMonitorCaptureConfiguration(BufferCallback callback);
        
        [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr SCL_CreateMonitorCaptureConfigurationWithContext(MonitorWindowCallbackWithContext callback, IntPtr context);
        
        [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SCL_FreeMonitorCaptureConfiguration(IntPtr ptr);
        
        [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SCL_FreeWindowCaptureConfiguration(IntPtr ptr);
        
        [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SCL_PauseCapturing(IntPtr ptr);
        
        [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SCL_IsPaused(IntPtr ptr);
        
        [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool SCL_Resume(IntPtr ptr);
        
        [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SCL_SetFrameChangeInterval(IntPtr ptr, int milliseconds);
        
        [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SCL_SetMouseChangeInterval(IntPtr ptr, int milliseconds);

        [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SCL_MonitorOnNewFrame(IntPtr ptr, ScreenCaptureCallback monitorCallback);

        [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SCL_MonitorOnNewFrameWithContext(IntPtr ptr, ScreenCaptureCallbackWithContext monitorCallback);

        [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SCL_MonitorOnFrameChanged(IntPtr ptr, ScreenCaptureCallback monitorCallback);

        [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SCL_MonitorOnFrameChangedWithContext(IntPtr ptr, ScreenCaptureCallbackWithContext monitorCallback);

        [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SCL_MonitorOnMouseChanged(IntPtr ptr, MouseCaptureCallback monitorCallback);

        [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SCL_MonitorOnMouseChangedWithContext(IntPtr ptr, MouseCaptureCallbackWithContext monitorCallback);

        [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr SCL_MonitorStartCapturing(IntPtr ptr);

        [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SCL_WindowOnNewFrame(IntPtr ptr, WindowCaptureCallback monitorCallback);

        [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SCL_WindowOnNewFrameWithContext(IntPtr ptr, WindowCaptureCallbackWithContext monitorCallback);

        [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SCL_WindowOnFrameChanged(IntPtr ptr, WindowCaptureCallback monitorCallback);

        [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SCL_WindowOnFrameChangedWithContext(IntPtr ptr, WindowCaptureCallbackWithContext monitorCallback);
        
        [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SCL_WindowOnMouseChanged(IntPtr ptr, MouseCaptureCallback monitorCallback);
        
        [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SCL_WindowOnMouseChangedWithContext(IntPtr ptr, MouseCaptureCallbackWithContext monitorCallback);
        
        [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr SCL_WindowStartCapturing(IntPtr ptr);
        
        [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SCL_FreeIScreenCaptureManagerWrapper(IntPtr ptr);

        [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr SCL_Utility_CopyToContiguous(IntPtr destination, IntPtr image);

    }

}
