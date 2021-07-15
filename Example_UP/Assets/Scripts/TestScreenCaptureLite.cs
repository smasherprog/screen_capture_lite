using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;
using AOT;

// internal class MonoPInvokeCallbackAttribute : Attribute {
//     public MonoPInvokeCallbackAttribute() { }
// }

#if UNITY_EDITOR
using UnityEditor;
[CustomEditor(typeof(TestScreenCaptureLite))]
public class TestScreenCaptureLiteInspector : Editor {
    public override void OnInspectorGUI() {
        base.DrawDefaultInspector();
        
        TestScreenCaptureLite t = (TestScreenCaptureLite)target;
        if (GUILayout.Button("Save FrameChangedTex")) {
            t.SaveFrameChangedTex();
        }
    }    
}
#endif


public class TestScreenCaptureLite : MonoBehaviour {

    [DllImport("libscreen_capture_lite")]
    public static extern int add(int a, int b);
    [DllImport("libscreen_capture_lite")]
    public static extern int subtract(int a, int b);


    [StructLayout(LayoutKind.Sequential)]
    public struct Point {
        public int x;
        public int y;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct MousePoint {
        public Point Position;
        public Point HotSpot;
    };

    [StructLayout(LayoutKind.Sequential)]
    public struct Window {
        // not sure wether this type is correct, but uint is _not_ working correctly
        // https://stackoverflow.com/questions/32906774/what-is-equal-to-the-c-size-t-in-c-sharp/32907246
        public UIntPtr Handle;
        public Point Position;
        public Point Size;
        // https://www.mono-project.com/docs/advanced/pinvoke/
        [MarshalAs (UnmanagedType.ByValTStr, SizeConst=128)]
        public string Name;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Image {}

    [StructLayout(LayoutKind.Sequential)]
    public struct ImageBGRA {
        public char B;
        public char G;
        public char R;
        public char A;
    }

    [StructLayout(LayoutKind.Sequential)]  
    public struct Monitor {  
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
        // https://www.mono-project.com/docs/advanced/pinvoke/
        [MarshalAs (UnmanagedType.ByValTStr, SizeConst=128)]
        public string Name;
        public float Scaling;
    }

    [DllImport("libscreen_capture_lite")]
    private static extern IntPtr C_GetWindows(ref int size);
 
    public Window[] GetWindows () {
        int windowsCount = 0;
        IntPtr windowsArray = C_GetWindows(ref windowsCount);
        Window[] windows = GetNativeArray<Window> (windowsArray, windowsCount);
        return windows;
    }

    [DllImport("libscreen_capture_lite")]
    private static extern IntPtr C_GetMonitors(ref int size);
 
    public Monitor[] GetMonitors () {
        int monitorCount = 0;
        IntPtr monitorsArray = C_GetWindows(ref monitorCount);
        Monitor[] monitors = GetNativeArray<Monitor> (monitorsArray, monitorCount);
        return monitors;
    }

    // public static IntPtr GetFunctionPointerForDelegate<TDelegate> (TDelegate d);

    // Testing Callback Types
    // public delegate void ImageRefWindowRefCallbackType(int size, [In, Out] byte[] imgData);
    // public delegate void ImagePtrMousePointRefCallbackType(int size, [In, Out] byte[] imgData);
    public delegate void ImageRefWindowRefCallbackType(int w, int h, int s, IntPtr array);
    public delegate void ImagePtrMousePointRefCallbackType(int w, int h, int s, IntPtr array);

    // public delegate void ImageRefWindowRefCallbackType(Image image, Window window);
    // public delegate void ImagePtrMousePointRefCallbackType(Image image, MousePoint mousePoint);

    [DllImport("libscreen_capture_lite")]
    public static extern void C_ICaptureConfiguration (
            Window windowToCapture, 
            ImageRefWindowRefCallbackType frameChangedCallback, 
            ImageRefWindowRefCallbackType newFrameCallback, 
            ImagePtrMousePointRefCallbackType mouseChangedCallback);

    [DllImport("libscreen_capture_lite")]
    public static extern void C_Capture_Start ();

    [DllImport("libscreen_capture_lite")]
    public static extern void C_Capture_Stop ();

    ImageRefWindowRefCallbackType frameChangedCallback;
    ImageRefWindowRefCallbackType newFameCallback;
    ImagePtrMousePointRefCallbackType mouseChangedCallback;

    public int windowIdToCapture = 0;

    void Start() {
        // Texture.allowThreadedTextureCreation = true; // useless, only for unity internals

        Window[] windows = GetWindows();
        for (int i=0; i<windows.Length; i++) {
            Window window = windows[i];
            Debug.Log($"Window: {i} handle:{window.Handle} posX:{window.Position.x} posY:{window.Position.y} sizeX:{window.Size.x} sizeY:{window.Size.y} Name:{window.Name}");
        }

        Monitor[] monitors = GetMonitors();
        for (int i=0; i<monitors.Length; i++) {
            Monitor monitor = monitors[i];
            Debug.Log($"Monitors Id:{monitor.Id} Index:{monitor.Index} Adapter:{monitor.Adapter}");
            Debug.Log($"- Name:'{monitor.Name}'");
            Debug.Log($"- Height:{monitor.Height} Width:{monitor.Width} OriginalHeight:{monitor.OriginalHeight} OriginalWidth:{monitor.OriginalWidth} Scaling:{monitor.Scaling}");
            Debug.Log($"- OffsetX:{monitor.OffsetX} OffsetY:{monitor.OffsetY} OriginalOffsetX:{monitor.OriginalOffsetX} OriginalOffsetY:{monitor.OriginalOffsetY}");
        }

        frameChangeDelegate = new ImageRefWindowRefCallbackType(OnFrameChanged);
        newFrameDelegate = new ImageRefWindowRefCallbackType(OnNewFrame);
        mouseChangedDelegate = new ImagePtrMousePointRefCallbackType(OnMouseChanged);

        Debug.Log($"- a -");
        C_ICaptureConfiguration(
            windows[windowIdToCapture], 
            frameChangeDelegate,
            newFrameDelegate,
            mouseChangedDelegate
        );
        Debug.Log($"- b -");
    }

    void OnApplicationQuit() {
        C_Capture_Stop();
    }

    void OnDisable () {
        C_Capture_Stop();
    }

    void OnDestroy () {
        C_Capture_Stop();
    }

    ImageRefWindowRefCallbackType frameChangeDelegate;
    ImageRefWindowRefCallbackType newFrameDelegate;
    ImagePtrMousePointRefCallbackType mouseChangedDelegate;

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
        } else {
            // probably 64-bit system
            for (int i = 0; i < result.Length; i++) {
                result [i] = (T)Marshal.PtrToStructure (array, typeof(T));
                array = new IntPtr (array.ToInt64 () + size);
            }
        }
        return result;
    }

    public Texture2D frameChangedTex;
    public Texture2D newFrameTex;
    public Texture2D mouseChangedTex;
    
    int onFrameChangedWidth;
    int onFrameChangedHeight;
    int onFrameChangedSize;
    // if there is data in the onFrameChangedBytes, the texture should be updated with it
    byte[] onFrameChangedBytes;
    
    int onNewFrameWidth;
    int onNewFrameHeight;
    int onNewFrameSize;
    byte[] onNewFrameBytes;
    
    int onMouseChangedWidth;
    int onMouseChangedHeight;
    int onMouseChangedSize;
    byte[] onMouseChangedBytes;

    // unity doesnt allow writing the texture here
    [MonoPInvokeCallback (typeof (ImageRefWindowRefCallbackType))]
    void OnFrameChanged (int w, int h, int s, IntPtr array) {
        // Debug.Log($"OnFrameChanged w:{w} h:{h} s:{s}   Data: {onFrameChangedBytes[0]:x} {onFrameChangedBytes[1]:x} {onFrameChangedBytes[2]:x} {onFrameChangedBytes[3]:x} {onFrameChangedBytes[4]:x} {onFrameChangedBytes[5]:x} {onFrameChangedBytes[6]:x} {onFrameChangedBytes[7]:x}");
        onFrameChangedWidth = w;
        onFrameChangedHeight = h;
        onFrameChangedSize = s;
        onFrameChangedBytes = GetNativeArray<byte> (array, s);

        if ((w * h * 4) != s) {
            Debug.Log($"OnFrameChanged w:{w} h:{h} *:{(w*h)} s:{s} l:{onFrameChangedBytes.Length} : Invalid size of Data for given Size");
            onFrameChangedBytes = null;
        }
    }

    [MonoPInvokeCallback (typeof (ImageRefWindowRefCallbackType))]
    void OnNewFrame (int w, int h, int s, IntPtr array) {
        onNewFrameWidth = w;
        onNewFrameHeight = h;
        onNewFrameSize = s;
        onNewFrameBytes = GetNativeArray<byte> (array, s);

        if ((w * h * 4) != s) {
            Debug.Log($"OnNewFrame: w:{w} h:{h} *:{(w*h)} s:{s} l:{onNewFrameBytes.Length} : Invalid size of Data for given Size");
            onNewFrameBytes = null;
        }
    }

    [MonoPInvokeCallback (typeof (ImagePtrMousePointRefCallbackType))]
    void OnMouseChanged (int w, int h, int s, IntPtr array) {
        onMouseChangedWidth = w;
        onMouseChangedHeight = h;
        onMouseChangedSize = s;
        onMouseChangedBytes = GetNativeArray<byte> (array, s);

        if ((w * h * 4) != s) {
            Debug.Log($"OnMouseChanged: w:{w} h:{h} *:{(w*h)} s:{s} l:{onMouseChangedBytes.Length} : Invalid size of Data for given Size");
            onMouseChangedBytes = null;
        }
    }

    public void SaveFrameChangedTex () {
        String dirPath = Application.dataPath + "/../" + "frameChangedTex.png";
        byte[] _bytes =frameChangedTex.EncodeToPNG();
        System.IO.File.WriteAllBytes(dirPath, _bytes);
        Debug.Log(_bytes.Length/1024  + "Kb was saved as: " + dirPath);
    }

    void Update () {
        if (onFrameChangedBytes != null) {
            if ((frameChangedTex == null) || (frameChangedTex.width != onFrameChangedWidth) || (frameChangedTex.height != onFrameChangedHeight)) {
                frameChangedTex = new Texture2D(onFrameChangedWidth, onFrameChangedHeight, TextureFormat.RGBA32, false);
            }
            try {
                Debug.Log($"Update.frameChangedTex: {onFrameChangedWidth} {onFrameChangedHeight} {onFrameChangedSize}");
                frameChangedTex.LoadRawTextureData(onFrameChangedBytes);
                onFrameChangedBytes = null;
                frameChangedTex.Apply();
            }
            catch (Exception e) {
                Debug.Log($"Update.frameChangedTex.Exception: bytes:{onFrameChangedBytes.Length} w:{onFrameChangedWidth} h:{onFrameChangedHeight} s:{onFrameChangedSize} *:{(onFrameChangedWidth*onFrameChangedHeight)}");
                Debug.Log($"{e.ToString()}");
            }
        }

        // untested
        if (onNewFrameBytes != null) {
            if ((newFrameTex == null) || (newFrameTex.width != onNewFrameWidth) || (newFrameTex.height != onNewFrameHeight)) {
                newFrameTex = new Texture2D(onNewFrameWidth, onNewFrameHeight, TextureFormat.RGBA32, false);
            }
            try {
                newFrameTex.LoadRawTextureData(onNewFrameBytes);
                onNewFrameBytes = null;
                newFrameTex.Apply();
            }
            catch (Exception e) {
                Debug.Log($"Update.onNewFrameBytes.Exception: bytes:{onNewFrameBytes.Length} w:{onNewFrameWidth} h:{onNewFrameHeight} s:{onNewFrameSize} *:{(onNewFrameWidth*onNewFrameHeight)}");
                Debug.Log($"{e.ToString()}");
            }
        }

        // untested
        if (onMouseChangedBytes != null) {
            if ((mouseChangedTex == null) || (mouseChangedTex.width != onMouseChangedWidth) || (mouseChangedTex.height != onMouseChangedHeight)) {
                mouseChangedTex = new Texture2D(onMouseChangedWidth, onMouseChangedHeight, TextureFormat.RGBA32, false);
            }
            try {
                mouseChangedTex.LoadRawTextureData(onMouseChangedBytes);
                onMouseChangedBytes = null;
                mouseChangedTex.Apply();
            }
            catch (Exception e) {
                Debug.Log($"Update.onMouseChangedBytes.Exception: bytes:{onMouseChangedBytes.Length} w:{onMouseChangedWidth} h:{onMouseChangedHeight} s:{onMouseChangedSize} *:{(onMouseChangedWidth*onMouseChangedHeight)}");
                Debug.Log($"{e.ToString()}");
            }
        }
    }

}
