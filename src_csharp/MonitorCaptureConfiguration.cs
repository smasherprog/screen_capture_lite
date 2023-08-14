using System;
using System.Runtime.InteropServices;
using System.Threading;

namespace SCL
{
    [StructLayout(LayoutKind.Sequential)]
    public class MonitorCaptureConfiguration : IDisposable
    {
        public IntPtr Config { get; private set; }

        private IntPtr _handle;

        private MonitorCallback _monitorCallback;

        private Action<Image, Monitor> _onNewFrame;

        private Action<Image, Monitor> _onFrameChanged;

        private Action<Image, MousePoint> _onMouseChanged;
        private bool disposedValue = false;
        private static int MonitorSizeHint = 8;

        private static MonitorWindowCallbackWithContext _onCaptureWithContext = OnCapture;
        private static ScreenCaptureCallbackWithContext _onNewFrameWithContext = OnNewFrame;
        private static ScreenCaptureCallbackWithContext _onFrameChangedWithContext = OnFrameChanged;
        private static MouseCaptureCallbackWithContext _onMouseChangedWithContext = OnMouseChanged;

        private static readonly UnmanagedHandles<MonitorCaptureConfiguration> UnmanagedHandles = new();

        public static Monitor[] GetMonitors()
        {
            return Utility.CopyUnmanagedWithHint<Monitor>(ref MonitorSizeHint, NativeFunctions.SCL_GetMonitors);
        }

        private static int OnCapture(IntPtr buffer, int buffersize, IntPtr context)
        {
            if (context == IntPtr.Zero) throw new InvalidOperationException("Got null config.");
            var conf = UnmanagedHandles.Get(context);
            var monitors = conf._monitorCallback();
            var count = Math.Min(buffersize, monitors.Length);
            var output = new UnmanagedArray<Monitor>(buffer, buffersize);
            for (var i = 0; i < count; i++) output[i] = monitors[i];
            return count;
        }

        private static void OnNewFrame(IntPtr imagePtr, IntPtr windowPtr, IntPtr context)
        {
            if (context == IntPtr.Zero) throw new InvalidOperationException("Got null config.");
            var conf = UnmanagedHandles.Get(context);
            var image = Marshal.PtrToStructure<Image>(imagePtr);
            var window = Marshal.PtrToStructure<Monitor>(windowPtr);
            conf._onNewFrame(image, window);
        }

        private static void OnFrameChanged(IntPtr imagePtr, IntPtr windowPtr, IntPtr context)
        {
            if (context == IntPtr.Zero) throw new InvalidOperationException("Got null config.");
            var conf = UnmanagedHandles.Get(context);
            var image = Marshal.PtrToStructure<Image>(imagePtr);
            var monitor = Marshal.PtrToStructure<Monitor>(windowPtr);
            conf._onFrameChanged(image, monitor);
        }

        private static void OnMouseChanged(IntPtr imagePtr, IntPtr mousePointPtr, IntPtr context)
        {
            if (context == IntPtr.Zero) throw new InvalidOperationException("Got null config.");
            var image = Marshal.PtrToStructure<Image>(imagePtr);
            var mousePoint = Marshal.PtrToStructure<MousePoint>(mousePointPtr);
            var conf = UnmanagedHandles.Get(context);
            conf._onMouseChanged(image, mousePoint);
        }

        public MonitorCaptureConfiguration(MonitorCallback callback)
        {
            try
            {
                _monitorCallback = callback;
                UnmanagedHandles.Add(this, out _handle);
                Config = NativeFunctions.SCL_CreateMonitorCaptureConfigurationWithContext(_onCaptureWithContext, _handle);
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
                NativeFunctions.SCL_MonitorOnNewFrameWithContext(Config, _onNewFrameWithContext);
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
                NativeFunctions.SCL_MonitorOnFrameChangedWithContext(Config, _onFrameChangedWithContext);
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
                NativeFunctions.SCL_MonitorOnMouseChangedWithContext(Config, _onMouseChangedWithContext);
            }
            else
            {
                _onMouseChanged += onMouseChanged;
            }

            return this;

        }
        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                if (Config != IntPtr.Zero)
                {
                    NativeFunctions.SCL_FreeMonitorCaptureConfiguration(Config);
                    Config = IntPtr.Zero;
                }
                if (_handle != IntPtr.Zero)
                {
                    UnmanagedHandles.Remove(_handle);
                    _handle = IntPtr.Zero;
                }

                if (disposing)
                {
                    this._onFrameChanged = null;
                    this._onMouseChanged = null;
                    this._onNewFrame = null;
                    this._monitorCallback = null;
                }
                disposedValue = true;
            }
        }

        ~MonitorCaptureConfiguration()
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
}