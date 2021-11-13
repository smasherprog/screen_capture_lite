using System;

namespace SCL
{
    public class ScreenCaptureManager : IDisposable
    {

        public IntPtr Session { get; private set; }

        private IDisposable _configuration;

        private bool disposedValue = false;

        public bool IsPaused
        {
            get => NativeFunctions.SCL_IsPaused(Session) != 0;
            set
            {
                if (value)
                {
                    NativeFunctions.SCL_PauseCapturing(Session);
                }
                else
                {
                    NativeFunctions.SCL_Resume(Session);
                }
            }
        }

        public ScreenCaptureManager(WindowCaptureConfiguration config)
        {
            _configuration = config;
            Session = NativeFunctions.SCL_WindowStartCapturing(config.Config);
        }

        public ScreenCaptureManager(MonitorCaptureConfiguration config)
        {
            _configuration = config;
            Session = NativeFunctions.SCL_MonitorStartCapturing(config.Config);
        }


        public ScreenCaptureManager SetFrameChangeInterval(int milliseconds)
        {
            NativeFunctions.SCL_SetFrameChangeInterval(Session, milliseconds);
            return this;
        }

        public ScreenCaptureManager SetMouseChangeInterval(int milliseconds)
        {
            NativeFunctions.SCL_SetMouseChangeInterval(Session, milliseconds);
            return this;
        }

        public ScreenCaptureManager PauseCapturing()
        {
            NativeFunctions.SCL_PauseCapturing(Session);
            return this;
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {

                if (Session != IntPtr.Zero) NativeFunctions.SCL_FreeIScreenCaptureManagerWrapper(Session);
                Session = IntPtr.Zero;

                if (disposing)
                {
                    _configuration?.Dispose();
                    _configuration = null;
                }

                disposedValue = true;
            }
        }

        // // TODO: override finalizer only if 'Dispose(bool disposing)' has code to free unmanaged resources
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
}