using System;

namespace Namazu.SCL
{
    public class ScreenCaptureManager : IDisposable
    {

        public IntPtr Session { get; private set; }

        public Boolean IsPaused
        {
            get => NativeFunctions.SCL_IsPaused(Session) != 0;
            set
            {
                if (IsPaused)
                {
                    if (value) NativeFunctions.SCL_Resume(Session);
                }
                else
                {
                    if (!value) NativeFunctions.SCL_PauseCapturing(Session);
                }
            }
        }

        public ScreenCaptureManager(WindowCaptureConfiguration config)
        {
            Session = NativeFunctions.SCL_WindowStartCapturing(config.Config);
        }

        public ScreenCaptureManager(MonitorCaptureConfiguration config)
        {
            Session = NativeFunctions.SCL_MonitorStartCapturing(config.Config);
        }

        public void Dispose()
        {
            if (Session != IntPtr.Zero) NativeFunctions.SCL_FreeIScreenCaptureManagerWrapper(Session);
            Session = IntPtr.Zero;
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

    }
}