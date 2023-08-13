using System;
using System.Runtime.InteropServices;
using System.Threading;

namespace SCL
{

    [StructLayout(LayoutKind.Sequential)]
    public class WindowCaptureConfiguration : IDisposable
    {
        public IntPtr Config { get; private set; }

        private WindowCallback _windowCallback;

        private IntPtr _handle;

        private Action<Image, Window> _onNewFrame;

        private Action<Image, Window> _onFrameChanged;

        private Action<Image, MousePoint> _onMouseChanged;

        private bool disposedValue = false;

        private static int WindowSizeHint = 64;

        private static readonly UnmanagedHandles<WindowCaptureConfiguration> UnmanagedHandles = new();

        private static MonitorWindowCallbackWithContext _onCaptureWithContext = OnCapture;
        private static WindowCaptureCallbackWithContext _onNewFrameWithContext = OnNewFrame;
        private static WindowCaptureCallbackWithContext _onFrameChangedWithContext = OnFrameChanged;
        private static MouseCaptureCallbackWithContext _onMouseChangedWithContext = OnMouseChanged;

        public static Window[] GetWindows()
        {
            return Utility.CopyUnmanagedWithHint<Window>(ref WindowSizeHint, NativeFunctions.SCL_GetWindows);
        }

        private static int OnCapture(IntPtr buffer, int buffersize, IntPtr context)
        {
            var conf = UnmanagedHandles.Get(context);
            if (conf == null) throw new InvalidOperationException("Invalid Handle.");
            var windows = conf._windowCallback();
            var count = Math.Min(buffersize, windows.Length);
            var output = new UnmanagedArray<Window>(buffer, buffersize);
            for (var i = 0; i < count; i++) output[i] = windows[i];
            return count;
        }

        private static void OnNewFrame(IntPtr imagePtr, IntPtr windowPtr, IntPtr context)
        {
            var conf = UnmanagedHandles.Get(context);
            if (conf == null) throw new InvalidOperationException("Invalid Handle.");
            var image = Marshal.PtrToStructure<Image>(imagePtr);
            var window = Marshal.PtrToStructure<Window>(windowPtr);
            conf._onNewFrame(image, window);
        }

        private static void OnFrameChanged(IntPtr imagePtr, IntPtr windowPtr, IntPtr context)
        {
            var conf = UnmanagedHandles.Get(context);
            var image = Marshal.PtrToStructure<Image>(imagePtr);
            var window = Marshal.PtrToStructure<Window>(windowPtr);
            conf._onFrameChanged(image, window);
        }

        private static void OnMouseChanged(IntPtr imagePtr, IntPtr mousePointPtr, IntPtr context)
        {
            var conf = UnmanagedHandles.Get(context);
            var image = Marshal.PtrToStructure<Image>(imagePtr);
            var mousePoint = Marshal.PtrToStructure<MousePoint>(mousePointPtr);
            conf._onMouseChanged(image, mousePoint);
        }

        public WindowCaptureConfiguration(WindowCallback callback)
        {
            try
            {
                _windowCallback = callback;
                UnmanagedHandles.Add(this, out _handle);
                Config = NativeFunctions.SCL_CreateWindowCaptureConfigurationWithContext(_onCaptureWithContext, _handle);
            }
            catch
            {
                Dispose();
                throw;
            }
        }

        public WindowCaptureConfiguration OnNewFrame(Action<Image, Window> onNewFrame)
        {

            if (_onNewFrame == null)
            {
                _onNewFrame = onNewFrame;
                NativeFunctions.SCL_WindowOnNewFrameWithContext(Config, _onNewFrameWithContext);
            }
            else
            {
                _onNewFrame += onNewFrame;
            }

            return this;

        }

        public WindowCaptureConfiguration OnFrameChanged(Action<Image, Window> onFrameChanged)
        {

            if (_onFrameChanged == null)
            {
                _onFrameChanged = onFrameChanged;
                NativeFunctions.SCL_WindowOnFrameChangedWithContext(Config, _onFrameChangedWithContext);
            }
            else
            {
                _onFrameChanged += onFrameChanged;
            }

            return this;

        }

        public WindowCaptureConfiguration OnMouseChanged(Action<Image, MousePoint> onMouseChanged)
        {

            if (_onMouseChanged == null)
            {
                _onMouseChanged = onMouseChanged;
                NativeFunctions.SCL_WindowOnMouseChangedWithContext(Config, _onMouseChangedWithContext);
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
                    NativeFunctions.SCL_FreeWindowCaptureConfiguration(Config);
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
                    this._windowCallback = null;
                }

                disposedValue = true;
            }
        }

        ~WindowCaptureConfiguration()
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