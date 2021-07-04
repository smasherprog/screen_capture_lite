using System;
using System.Runtime.InteropServices;

namespace SL
{
    namespace Screen_Capture
    {
        [StructLayout(LayoutKind.Sequential)]
        public struct Point
        {
            public int x;
            public int y;
        }
        [StructLayout(LayoutKind.Sequential)]
        public struct MousePoint
        {
            public Point Position;
            public Point HotSpot;
        };

        [StructLayout(LayoutKind.Sequential)]
        public struct Window
        {
            public IntPtr Handle;
            public Point Position;
            public Point Size;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
            public string Name;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct Monitor
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
        public struct ImageBGRA
        {
            public char B;
            public char G;
            public char R;
            public char A;
        }

        public static class FreeFunctions
        { 
            [DllImport("screen_capture_lite_shared")]
            public static extern void GetMonitors(Monitor[] monitors, ref int monitorslen);
        }
    }
}