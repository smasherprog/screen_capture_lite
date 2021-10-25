using System;
using System.Runtime.InteropServices;

namespace SL
{
    public static class Screen_Capture
    {
        public static CaptureConfigurationData<MonitorType> onNewFrame(this CaptureConfigurationData<MonitorType> captureConfiguration, ScreenCaptureCallback cb)
        {
            captureConfiguration.MonitorImpl_.OnNewFrame = cb;
            NativeFunctions.MonitoronNewFrame(captureConfiguration.CaptureConfigurationPtr, cb);
            return captureConfiguration;
        }

        public static CaptureConfigurationData<MonitorType> onFrameChanged(this CaptureConfigurationData<MonitorType> captureConfiguration, ScreenCaptureCallback cb)
        {
            captureConfiguration.MonitorImpl_.OnFrameChanged = cb;
            NativeFunctions.MonitoronFrameChanged(captureConfiguration.CaptureConfigurationPtr, cb);
            return captureConfiguration;
        }
        public static CaptureConfigurationData<MonitorType> onMouseChanged(this CaptureConfigurationData<MonitorType> captureConfiguration, MouseCaptureCallback cb)
        {
            captureConfiguration.MonitorImpl_.OnMouseChanged = cb;
            NativeFunctions.MonitoronMouseChanged(captureConfiguration.CaptureConfigurationPtr, cb);
            return captureConfiguration;
        }

        public static ScreenCaptureManager start_capturing(this CaptureConfigurationData<MonitorType> captureConfiguration)
        {
            var r = new ScreenCaptureManager(NativeFunctions.Monitorstart_capturing(captureConfiguration.CaptureConfigurationPtr), captureConfiguration.MonitorImpl_);
            captureConfiguration.CaptureConfigurationPtr = IntPtr.Zero;
            return r;
        }

        public static CaptureConfigurationData<WindowType> onNewFrame(this CaptureConfigurationData<WindowType> captureConfiguration, WindowCaptureCallback cb)
        {
            captureConfiguration.WindowImpl_.OnNewFrame = cb;
            NativeFunctions.WindowonNewFrame(captureConfiguration.CaptureConfigurationPtr, cb);
            return captureConfiguration;
        }

        public static CaptureConfigurationData<WindowType> onFrameChanged(this CaptureConfigurationData<WindowType> captureConfiguration, WindowCaptureCallback cb)
        {
            captureConfiguration.WindowImpl_.OnFrameChanged = cb;
            NativeFunctions.WindowonFrameChanged(captureConfiguration.CaptureConfigurationPtr, cb);
            return captureConfiguration;
        }
        public static CaptureConfigurationData<WindowType> onMouseChanged(this CaptureConfigurationData<WindowType> captureConfiguration, MouseCaptureCallback cb)
        {
            captureConfiguration.WindowImpl_.OnMouseChanged = cb;
            NativeFunctions.WindowonMouseChanged(captureConfiguration.CaptureConfigurationPtr, cb);
            return captureConfiguration;
        }
        public static ScreenCaptureManager start_capturing(this CaptureConfigurationData<WindowType> captureConfiguration)
        {
            var r = new ScreenCaptureManager(NativeFunctions.Windowstart_capturing(captureConfiguration.CaptureConfigurationPtr), captureConfiguration.WindowImpl_);
            captureConfiguration.CaptureConfigurationPtr = IntPtr.Zero;
            return r;
        }

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
        public delegate void MouseCaptureCallback(Image img, MousePoint mousePoint);
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void ScreenCaptureCallback(Image img, Monitor monitor);
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void WindowCaptureCallback(Image img, Window window);
        public class MonitorType
        {
        }
        public class WindowType
        {
        }
        public class ScreenCaptureManager : IDisposable
        {
            private CaptureData<NativeFunctions.MonitorWindowCallback, ScreenCaptureCallback, MouseCaptureCallback> ScreenCaptureData;
            private CaptureData<NativeFunctions.MonitorWindowCallback, WindowCaptureCallback, MouseCaptureCallback> WindowCaptureData;
            private bool disposedValue;
            private IntPtr ScreenCapturePtr = IntPtr.Zero;

            public ScreenCaptureManager(IntPtr p, CaptureData<NativeFunctions.MonitorWindowCallback, ScreenCaptureCallback, MouseCaptureCallback> d)
            {
                ScreenCapturePtr = p;
                ScreenCaptureData = d;
            }
            public ScreenCaptureManager(IntPtr p, CaptureData<NativeFunctions.MonitorWindowCallback, WindowCaptureCallback, MouseCaptureCallback> d)
            {
                ScreenCapturePtr = p;
                WindowCaptureData = d;
            }

            public void pause()
            {
                NativeFunctions.pausecapturing(ScreenCapturePtr);
            }

            public void resume()
            {
                NativeFunctions.resume(ScreenCapturePtr);
            }

            public bool isPaused()
            {
                return NativeFunctions.isPaused(ScreenCapturePtr);
            }

            public void setFrameChangeInterval(int milliseconds)
            {
                NativeFunctions.setFrameChangeInterval(ScreenCapturePtr, milliseconds);
            }
            public void setMouseChangeInterval(int milliseconds)
            {
                NativeFunctions.setMouseChangeInterval(ScreenCapturePtr, milliseconds);
            }

            protected virtual void Dispose(bool disposing)
            {
                if (!disposedValue)
                {
                    NativeFunctions.FreeIScreenCaptureManagerWrapper(ScreenCapturePtr);
                    ScreenCapturePtr = IntPtr.Zero;
                    disposedValue = true;
                }
            }

            ~ScreenCaptureManager()
            {
                // Do not change this code. Put cleanup code in 'Dispose(bool disposing)' method
                Dispose(disposing: false);
            }

            public void Dispose()
            {
                // Do not change this code. Put cleanup code in 'Dispose(bool disposing)' method
                Dispose(disposing: true);
                GC.SuppressFinalize(this);
            }
        }

        public class CaptureData<MonitorCallback, ScreenCaptureCallback, MouseCaptureCallback>
        {
            public MonitorCallback getThingsToWatch;
            public ScreenCaptureCallback OnFrameChanged;
            public MouseCaptureCallback OnMouseChanged;
            public ScreenCaptureCallback OnNewFrame;
            public int FrameTimerInMS = 100;
        };

        public class CaptureConfigurationData<T> : IDisposable
        {
            public CaptureData<NativeFunctions.MonitorWindowCallback, ScreenCaptureCallback, MouseCaptureCallback> MonitorImpl_;
            public CaptureData<NativeFunctions.MonitorWindowCallback, WindowCaptureCallback, MouseCaptureCallback> WindowImpl_;
            private bool disposedValue;
            public IntPtr CaptureConfigurationPtr = IntPtr.Zero;

            private void FreeUnmanagedMemory()
            {
                if (MonitorImpl_ != null)
                {
                    NativeFunctions.FreeMonitorCaptureConfiguration(CaptureConfigurationPtr);
                }
                if (WindowImpl_ != null)
                {
                    NativeFunctions.FreeWindowCaptureConfiguration(CaptureConfigurationPtr);
                }
                CaptureConfigurationPtr = IntPtr.Zero;
            }

            protected virtual void Dispose(bool disposing)
            {
                if (!disposedValue)
                {
                    if (disposing)
                    {
                        // TODO: dispose managed state (managed objects)
                    }

                    FreeUnmanagedMemory();
                    disposedValue = true;
                }
            }

            ~CaptureConfigurationData()
            {
                // Do not change this code. Put cleanup code in 'Dispose(bool disposing)' method
                Dispose(disposing: false);
            }

            public void Dispose()
            {
                // Do not change this code. Put cleanup code in 'Dispose(bool disposing)' method
                Dispose(disposing: true);
                GC.SuppressFinalize(this);
            }
        }

        public static class CaptureConfiguration
        {
            public static CaptureConfigurationData<MonitorType> CreateCaptureConfiguration(MonitorCallback monitorstocapture)
            {
                NativeFunctions.MonitorWindowCallback newcb = (IntPtr buffer, int buffersize) =>
                {
                    var m = monitorstocapture();
                    var monitorstocopy = m.Length > buffersize ? buffersize : m.Length;
                    for (var i = 0; i < monitorstocopy; i++)
                    {
                        Marshal.StructureToPtr(m[i], buffer, false);
                        var size = Marshal.SizeOf(typeof(Monitor));
                        buffer = IntPtr.Add(buffer, size);
                    }
                    return m.Length;
                };
                return new CaptureConfigurationData<MonitorType>
                {
                    MonitorImpl_ = new CaptureData<NativeFunctions.MonitorWindowCallback, ScreenCaptureCallback, MouseCaptureCallback>
                    {
                        getThingsToWatch = newcb
                    },
                    CaptureConfigurationPtr = NativeFunctions.CreateMonitorCaptureConfiguration(newcb)
                };
            }
            public static CaptureConfigurationData<WindowType> CreateCaptureConfiguration(WindowCallback windowsstocapture)
            {
                NativeFunctions.MonitorWindowCallback newcb = (IntPtr buffer, int buffersize) =>
                {
                    var m = windowsstocapture();
                    var monitorstocopy = m.Length > buffersize ? buffersize : m.Length;
                    for (var i = 0; i < monitorstocopy; i++)
                    {
                        Marshal.StructureToPtr(m[i], buffer, false);
                        var size = Marshal.SizeOf(typeof(Window));
                        buffer = IntPtr.Add(buffer, size);
                    }
                    return m.Length;
                };
                return new CaptureConfigurationData<WindowType>
                {
                    WindowImpl_ = new CaptureData<NativeFunctions.MonitorWindowCallback, WindowCaptureCallback, MouseCaptureCallback>
                    {
                        getThingsToWatch = newcb
                    },
                    CaptureConfigurationPtr = NativeFunctions.CreateWindowCaptureConfiguration(newcb)
                };
            }
        }

        private static int GetMonitorsAllocationSize = 16;
        public static Monitor[] GetMonitors()
        {
            return GetThings<Monitor>(ref GetMonitorsAllocationSize, NativeFunctions.GetMonitors);
        }

        private static int GetWindowsAllocationSize = 16;
        public static Window[] GetWindows()
        {
            return GetThings<Window>(ref GetWindowsAllocationSize, NativeFunctions.GetWindows);
        }

        private static T[] GetThings<T>(ref int guesssize, NativeFunctions.MonitorWindowCallback callback)
        {
            var size = Marshal.SizeOf(typeof(T));
            var unmanagedArray = Marshal.AllocHGlobal(guesssize * size);
            var sizeneeded = callback(unmanagedArray, guesssize);
            if (guesssize < sizeneeded)
            {
                guesssize = sizeneeded;
                Marshal.FreeHGlobal(unmanagedArray);
                unmanagedArray = Marshal.AllocHGlobal(sizeneeded * size);
                sizeneeded = callback(unmanagedArray, sizeneeded);
            }

            var copyunmanagedArray = unmanagedArray;
            var mangagedArray = new T[sizeneeded];
            for (int i = 0; i < sizeneeded; i++)
            {
                mangagedArray[i] = Marshal.PtrToStructure<T>(unmanagedArray);
                unmanagedArray = IntPtr.Add(unmanagedArray, size);
            }
            Marshal.FreeHGlobal(copyunmanagedArray);
            return mangagedArray;
        }

        public static class NativeFunctions
        { 
            [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
            public delegate int MonitorWindowCallback(IntPtr buffer, int buffersize);

            [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
            public static extern int SCL_GetMonitors(IntPtr buffer, int buffer_size);
            [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
            public static extern int SCL_GetWindows(IntPtr buffer, int buffer_size);
            [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr SCL_CreateWindowCaptureConfiguration(MonitorWindowCallback callback);
            [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr SCL_CreateMonitorCaptureConfiguration(MonitorWindowCallback callback);
            [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
            public static extern void SCL_FreeMonitorCaptureConfiguration(IntPtr ptr);
            [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
            public static extern void SCL_FreeWindowCaptureConfiguration(IntPtr ptr);
            [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
            public static extern void SCL_PauseCapturing(IntPtr ptr);
            [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
            [return: MarshalAs(UnmanagedType.I1)]
            public static extern bool SCL_IsPaused(IntPtr ptr);
            [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
            public static extern bool SCL_Resume(IntPtr ptr);
            [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
            public static extern void SCL_SetFrameChangeInterval(IntPtr ptr, int milliseconds);
            [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
            public static extern void SCL_SetMouseChangeInterval(IntPtr ptr, int milliseconds);

            [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
            public static extern void SCL_MonitorOnNewFrame(IntPtr ptr, ScreenCaptureCallback monitorCallback);
            [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
            public static extern void SCL_MonitorOnFrameChanged(IntPtr ptr, ScreenCaptureCallback monitorCallback);
            [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
            public static extern void SCL_MonitorOnMouseChanged(IntPtr ptr, MouseCaptureCallback monitorCallback);
            [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
            //this function will free the input pointer calling FreeCaptureConfiguration internally since its no longer needed. This makes it easier to reason about the c# code.
            public static extern IntPtr SCL_MonitorStartCapturing(IntPtr ptr);

            [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
            public static extern void SCL_WindowOnNewFrame(IntPtr ptr, WindowCaptureCallback monitorCallback);
            [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
            public static extern void SCL_WindowOnFrameChanged(IntPtr ptr, WindowCaptureCallback monitorCallback);
            [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
            public static extern void SCL_WindowOnMouseChanged(IntPtr ptr, MouseCaptureCallback monitorCallback);
            [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
            //this function will free the input pointer calling FreeCaptureConfiguration internally since its no longer needed. This makes it easier to reason about the c# code.
            public static extern IntPtr SCL_WindowStartCapturing(IntPtr ptr);
            [DllImport("screen_capture_lite_shared", CallingConvention = CallingConvention.Cdecl)]
            public static extern void SCL_FreeIScreenCaptureManagerWrapper(IntPtr ptr);
        }
    }
}