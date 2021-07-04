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

        public static class NativeFunctions
        { 
            [DllImport("screen_capture_lite_shared")]
            private static extern int GetMonitors(ref IntPtr monitors);
            public static Monitor[] GetMonitors()
            {
                var unmanagedArray = IntPtr.Zero;
                var length = GetMonitors(ref unmanagedArray);
                var size = Marshal.SizeOf(typeof(Monitor));
                var mangagedArray = new Monitor[length]; 

                for (int i = 0; i < length; i++)
                {
                    if (IntPtr.Size == 8)
                    {
                        var ins = new IntPtr(unmanagedArray.ToInt64() + i * size);
                        mangagedArray[i] = Marshal.PtrToStructure<Monitor>(ins);
                    }
                    else
                    {
                        var ins = new IntPtr(unmanagedArray.ToInt32() + i * size);
                        mangagedArray[i] = Marshal.PtrToStructure<Monitor>(ins);
                    }
                }
                return mangagedArray;
            }

            [DllImport("screen_capture_lite_shared")]
            private static extern bool isMonitorInsideBounds(ref Monitor[] monitors, int monitorsize, ref Monitor monitor);
            public static bool isMonitorInsideBounds(Monitor[] monitors, Monitor monitor)
            {
                return true;
            }
        }
    }
}