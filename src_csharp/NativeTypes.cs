using System;
using System.Runtime.InteropServices;

namespace SCL
{
    
    [StructLayout(LayoutKind.Sequential)]
    public class Point
    {
        public int x;
        public int y;
    }
    
    [StructLayout(LayoutKind.Sequential)]
    public class MousePoint
    {
        public Point Position;
        public Point HotSpot;
    };

    [StructLayout(LayoutKind.Sequential)]
    public class Window
    {
        public IntPtr Handle;
        public Point Position;
        public Point Size;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string Name;
    }

    [StructLayout(LayoutKind.Sequential)]
    public class Monitor
    {
        public int Id;
        public int Index;
        public int Adapter;
        public int Height;
        public int Width;
        public int OriginalHeight;
        public int OriginalWidth;
        // Offsets are the number of pixels that a monitor can be from the origin. For example, users can shuffle their
        // monitors around so this affects their offset.
        public int OffsetX;
        public int OffsetY;
        public int OriginalOffsetX;
        public int OriginalOffsetY;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string Name;
        public float Scaling;
    }
    
    [StructLayout(LayoutKind.Sequential)]
    public class ImageRect
    {
        public int left;
        public int top;
        public int right;
        public int bottom;
    }
    
    [StructLayout(LayoutKind.Sequential)]
    public class Image
    {
        public ImageRect Bounds;
        public int BytesToNextRow;
        [MarshalAs(UnmanagedType.I1)]
        public bool isContiguous;
        // alpha is always unused and might contain garbage
        public IntPtr Data;
    }
        
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate Window[] WindowCallback();

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate Monitor[] MonitorCallback();

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void MouseCaptureCallback(IntPtr img, IntPtr mousePoint);
        
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void MouseCaptureCallbackWithContext(IntPtr intPtr, IntPtr mousePoint, IntPtr context);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void ScreenCaptureCallback(IntPtr img, IntPtr monitor);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void ScreenCaptureCallbackWithContext(IntPtr img, IntPtr monitor, IntPtr context);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void WindowCaptureCallback(IntPtr img, IntPtr window);
        
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void WindowCaptureCallbackWithContext(IntPtr img, IntPtr window, IntPtr context);
    
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate int BufferCallback(IntPtr buffer, int buffersize);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate int MonitorWindowCallbackWithContext(IntPtr buffer, int buffersize, IntPtr context);


}