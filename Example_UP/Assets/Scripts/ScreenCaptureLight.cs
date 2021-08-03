using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using AOT;
using UnityEngine;

namespace SCL {

#region Screen Capture Light API
    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    public struct Point {
        public int x;
        public int y;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    public struct MousePoint {
        public Point Position;
        public Point HotSpot;
    };

    [StructLayout(LayoutKind.Sequential, Size = 4+8+8+128, Pack = 4, CharSet = CharSet.Ansi)]
    public struct Window {
        // not sure wether this type is correct, but uint is _not_ working correctly
        // https://stackoverflow.com/questions/32906774/what-is-equal-to-the-c-size-t-in-c-sharp/32907246
        public UIntPtr Handle; // size_t in c++
        public Point Position;
        public Point Size;
        [MarshalAs (UnmanagedType.ByValTStr, SizeConst=128)]
        public string Name;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    public struct Image {}

    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    public struct ImageBGRA {
        public char B;
        public char G;
        public char R;
        public char A;
    }

    [StructLayout(LayoutKind.Sequential, Size=176, Pack = 4, CharSet = CharSet.Ansi)]
    public struct Monitor {
        public int Id;
        public int Index;
        public int Adapter;
        public int Height;
        public int Width;
        public int OriginalHeight;
        public int OriginalWidth;
        // Offset are the number of pixels that a monitor can be from the origin. For example, users can shuffle their
        // monitors around so this affects their offset.
        public int OffsetX;
        public int OffsetY;
        public int OriginalOffsetX;
        public int OriginalOffsetY;
        // https://www.mono-project.com/docs/advanced/pinvoke/
        [MarshalAs (UnmanagedType.ByValTStr, SizeConst=128)]
        public string Name;
        public float Scaling;
    }
#endregion
 
public class ScreenCaptureLightManager {

#region Screen Capture Light API
    [DllImport("libscreen_capture_lite")]
    private static extern IntPtr C_GetWindows(ref int size);
 
    [DllImport("libscreen_capture_lite")]
    private static extern IntPtr C_GetMonitors(ref int size);

    // https://stackoverflow.com/questions/43848841/pass-large-arrays-of-structs-from-c-sharp-unity-script-to-c-dll-using-gchandle
    [DllImport("libscreen_capture_lite", CallingConvention = CallingConvention.Cdecl)]
    public static extern void C_ICaptureConfiguration (
            [In] IntPtr windowToCapture,
            int numberOfWindows,
            ImageRefWindowRefCallbackType frameChangedCallback, 
            ImageRefWindowRefCallbackType newFrameCallback, 
            ImagePtrMousePointRefCallbackType mouseChangedCallback
        );

    [DllImport("libscreen_capture_lite")]
    public static extern void C_Capture_Start ();

    [DllImport("libscreen_capture_lite")]
    public static extern void C_Capture_Stop ();

    [DllImport("libscreen_capture_lite")]
    public static extern void C_Capture_Pause ();

    [DllImport("libscreen_capture_lite")]
    public static extern void C_Capture_Resume ();

    [DllImport("libscreen_capture_lite")]
    public static extern bool C_Capture_IsPaused ();

    [DllImport("libscreen_capture_lite")]
    public static extern bool C_Capture_ExpectedErrorEvent ();

    [DllImport("libscreen_capture_lite")]
    public static extern void C_Capture_SetFrameChangeInterval (int ms);

    [DllImport("libscreen_capture_lite")]
    public static extern void C_Capture_SetMouseChangeInterval (int ms);
#endregion Screen Capture Light API


#region Screen Capture Light Unity API
    public static Window[] GetWindows () {
        int windowsCount = 0;
        IntPtr windowsArray = C_GetWindows(ref windowsCount);
        Window[] windows = GetNativeArray<Window> (windowsArray, windowsCount);
        return windows;
    }
 
    public static Monitor[] GetMonitors () {
        int monitorCount = 0;
        IntPtr monitorsArray = C_GetMonitors(ref monitorCount);
        Monitor[] monitors = GetNativeArray<Monitor> (monitorsArray, monitorCount);
        return monitors;
    }

    public delegate void ImageRefWindowRefCallbackType(int w, int h, int s, IntPtr array, IntPtr window);
    public delegate void ImagePtrMousePointRefCallbackType(int w, int h, int s, IntPtr array, IntPtr mousePoint);

    static ImageRefWindowRefCallbackType frameChangedCallback;
    static ImageRefWindowRefCallbackType newFameCallback;
    static ImagePtrMousePointRefCallbackType mouseChangedCallback;

    static ImageRefWindowRefCallbackType frameChangeDelegate = null;
    static ImageRefWindowRefCallbackType newFrameDelegate = null;
    static ImagePtrMousePointRefCallbackType mouseChangedDelegate = null;

    // based on https://answers.unity.com/questions/34606/how-do-i-pass-arrays-from-c-to-c-in-unity-if-at-al.html
    private static T[] GetNativeArray<T>(IntPtr array, int length) {
        T[] result = new T[length];
        int size = Marshal.SizeOf (typeof(T));

        if (IntPtr.Size == 4) {
            // 32-bit system
            for (int i = 0; i < result.Length; i++) {
                result [i] = (T)Marshal.PtrToStructure (array, typeof(T));
                array = new IntPtr (array.ToInt32 () + size);
            }
        } 
        else {
            // probably 64-bit system
            for (int i = 0; i < result.Length; i++) {
                result [i] = (T)Marshal.PtrToStructure (array, typeof(T));
                array = new IntPtr (array.ToInt64 () + size);
            }
        }
        return result;
    }

    private static T GetNativeType<T>(IntPtr ptr) {
        return (T)Marshal.PtrToStructure (ptr, typeof(T));
    }

    public static List<WindowCallbackData> onFrameChangedData = new List<WindowCallbackData>();
    public static List<WindowCallbackData> onNewFrameData = new List<WindowCallbackData>();
    public static MousecursorCallbackData onMouseChangedData = new MousecursorCallbackData();

    // unity doesnt allow writing the texture here
    [MonoPInvokeCallback (typeof (ImageRefWindowRefCallbackType))]
    static void OnFrameChanged (int w, int h, int s, IntPtr array, IntPtr windowPtr) {
        Window win = GetNativeType<Window> (windowPtr);

        // Debug.Log($"OnFrameChanged {win.Handle} w:{w} h:{h} s:{s}");
        int index = onFrameChangedData.FindIndex(a => a.window.Handle == win.Handle);
        if (index == -1) {
            // Debug.Log($"OnFrameChanged {index} {win.Handle} Create w:{w} h:{h} s:{s}");
            WindowCallbackData windowCallbackParams = new WindowCallbackData() {width=w, height=h, window=win};
            onFrameChangedData.Add(windowCallbackParams);
            index = onFrameChangedData.FindIndex(a => a.window.Handle == win.Handle);
        }

        if (index != -1) {
            // Debug.Log($"OnFrameChanged Update w:{w} h:{h} s:{s}");
            onFrameChangedData[index].Changed (w, h, s, array, win);
        }
    }

    [MonoPInvokeCallback (typeof (ImageRefWindowRefCallbackType))]
    static void OnNewFrame (int w, int h, int s, IntPtr array, IntPtr windowPtr) {
        Window win = GetNativeType<Window> (windowPtr);
        // Debug.Log($"OnNewFrame {win.Handle} w:{w} h:{h} s:{s}");

        int index = onNewFrameData.FindIndex(a => a.window.Handle == win.Handle);
        if (index == -1) {
            // Debug.Log($"OnNewFrame {index} {win.Handle} Create w:{w} h:{h} s:{s}");
            WindowCallbackData windowCallbackParams = new WindowCallbackData() {width=w, height=h, window=win};
            onNewFrameData.Add(windowCallbackParams);
            index = onNewFrameData.FindIndex(a => a.window.Handle == win.Handle);
        }

        if (index != -1) {
            // Debug.Log($"OnNewFrame Update w:{w} h:{h} s:{s}");
            onNewFrameData[index].Changed (w, h, s, array, win);
        }
    }

    [MonoPInvokeCallback (typeof (ImagePtrMousePointRefCallbackType))]
    static void OnMouseChanged (int w, int h, int s, IntPtr array, IntPtr mposPtr) {
        MousePoint mousePoint = GetNativeType<MousePoint> (mposPtr);

        onMouseChangedData.Changed(w, h, s, array, mousePoint);
    }

    // convert the texture data to textures to use in unity
    // public static void UpdateTextures () {
    //     WindowCallbackParams parms;

    //     // update all textures onFrameChanged
    //     for (int i=0; i<onFrameChangedParams.Count; i++) {
    //         // Debug.Log($"UpdateTextures onFrameChangedParams {i}");
    //         parms = onFrameChangedParams[i];
    //         parms.UpdateTexture();
    //     }

    //     for (int i=0; i<onNewFrameParams.Count; i++) {
    //         parms = onNewFrameParams[i];
    //         parms.UpdateTexture();
    //     }

    //     onMouseChangedParms.UpdateTexture();

    //     if (C_Capture_ExpectedErrorEvent()) {
    //         Debug.LogWarning("Capturing Encountered Expected Error (like window resize)");
    //     }
    // }
#endregion Screen Capture Light Unity API

#region 
    public static void StartCapture (Window[] captureWindows, bool captureNewFrame, bool captureOnFrameChanged, bool captureMouseChanged) {
        if (captureNewFrame) {
            newFrameDelegate = new ImageRefWindowRefCallbackType(OnNewFrame);
        }
        if (captureOnFrameChanged) {
            frameChangeDelegate = new ImageRefWindowRefCallbackType(OnFrameChanged);
        }
        if (captureMouseChanged) {
            mouseChangedDelegate = new ImagePtrMousePointRefCallbackType(OnMouseChanged);
        }

        GCHandle pinnedArray = GCHandle.Alloc(captureWindows, GCHandleType.Pinned);
        IntPtr ptr = pinnedArray.AddrOfPinnedObject();
        // Debug.Log($"- a - Log Messages from the Capture Configuration Setup appear here -");
        C_ICaptureConfiguration( ptr, captureWindows.Length, frameChangeDelegate, newFrameDelegate, mouseChangedDelegate );
        // Debug.Log($"- b -");
        pinnedArray.Free();
    }
#endregion


}
}