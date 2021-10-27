using System;
using System.Runtime.InteropServices;
using System.Threading;

namespace Namazu.SCL
{
    public class MonitorCaptureConfiguration : IDisposable
    {

        public IntPtr Config { get; private set; }

        private GCHandle _this;

        private readonly MonitorCallback _monitorCallback;

        private Action<Image, Monitor> _onNewFrame;
        
        private Action<Image, Monitor> _onFrameChanged;

        private Action<Image, MousePoint> _onMouseChanged;

        private static readonly ThreadLocal<int> MonitorSizeHint = new ThreadLocal<int>(() => 4);

        public static Monitor[] GetWindows()
        {
            return Utility.CopyUnmanagedWithHint<Monitor>(MonitorSizeHint, NativeFunctions.SCL_GetMonitors);
        }

        private static int OnCapture(IntPtr buffer, int buffersize, IntPtr context)
        {

            var hnd = GCHandle.FromIntPtr(context);

            try
            {
                var conf = (MonitorCaptureConfiguration) hnd.Target;
                var monitors = conf._monitorCallback();
                var count = Math.Min(buffersize, monitors.Length);
                var output = new UnmanagedArray<Monitor>(buffer, buffersize);
                for (var i = 0; i < count; i++) output[i] = monitors[i];
                return count;
            }
            finally
            {
                hnd.Free();
            }

        }

        private static void OnNewFrame(IntPtr imagePtr, IntPtr windowPtr, IntPtr context)
        {

            var hnd = GCHandle.FromIntPtr(context);

            try
            {
                var conf = (MonitorCaptureConfiguration) hnd.Target;
                var image = Marshal.PtrToStructure<Image>(imagePtr);
                var window = Marshal.PtrToStructure<Monitor>(windowPtr);
                conf._onNewFrame(image, window);
            }
            finally
            {
                hnd.Free();
            }

        }

        private static void OnFrameChanged(IntPtr imagePtr, IntPtr windowPtr, IntPtr context)
        {
            
            var hnd = GCHandle.FromIntPtr(context);

            try
            {
                var conf = (MonitorCaptureConfiguration) hnd.Target;
                var image = Marshal.PtrToStructure<Image>(imagePtr);
                var monitor = Marshal.PtrToStructure<Monitor>(windowPtr);
                conf._onFrameChanged(image, monitor);
            }
            finally
            {
                hnd.Free();
            }

        }

        private static void OnMouseChanged(IntPtr imagePtr, IntPtr mousePointPtr, IntPtr context)
        {
            
            var hnd = GCHandle.FromIntPtr(context);

            try
            {
                var conf = (MonitorCaptureConfiguration) hnd.Target;
                var image = Marshal.PtrToStructure<Image>(imagePtr);
                var mousePoint = Marshal.PtrToStructure<MousePoint>(mousePointPtr);
                conf._onMouseChanged(image, mousePoint);
            }
            finally
            {
                hnd.Free();
            }

        }

        public MonitorCaptureConfiguration(MonitorCallback callback)
        {
            try
            {
                _monitorCallback = callback;
                _this = GCHandle.Alloc(this, GCHandleType.Pinned);
                Config = NativeFunctions.SCL_CreateMonitorCaptureConfigurationWithContext(OnCapture, _this.AddrOfPinnedObject());
            }
            catch
            {
                Dispose();
                throw;
            }
        }

        public MonitorCaptureConfiguration OnNewFrame(Action<Image, Monitor> onNewFrame)
        {
            
            if (_onNewFrame == null)
            {
                _onNewFrame = onNewFrame;
                NativeFunctions.SCL_MonitorOnNewFrameWithContext(Config, OnNewFrame);
            }
            else
            {
                _onNewFrame += onNewFrame;
            }

            return this;
            
        }
        
        public MonitorCaptureConfiguration OnFrameChanged(Action<Image, Monitor> onFrameChanged)
        {

            if (_onFrameChanged == null)
            {
                _onFrameChanged = onFrameChanged;
                NativeFunctions.SCL_MonitorOnFrameChangedWithContext(Config, OnFrameChanged);
            }
            else
            {
                _onFrameChanged += onFrameChanged;
            }

            return this;
            
        }

        public MonitorCaptureConfiguration OnMouseChanged(Action<Image, MousePoint> onMouseChanged)
        {
            
            if (_onMouseChanged == null)
            {
                _onMouseChanged = onMouseChanged;
                NativeFunctions.SCL_MonitorOnMouseChangedWithContext(Config, OnMouseChanged);
            }
            else
            {
                _onMouseChanged += onMouseChanged;
            }

            return this;

        }

        public void Dispose()
        {

            if (Config != IntPtr.Zero)
            {
                NativeFunctions.SCL_FreeMonitorCaptureConfiguration(Config);
                Config = IntPtr.Zero;
            }

            if (_this.IsAllocated)
            {
                _this.Free();
            }

        }

    }
    
}