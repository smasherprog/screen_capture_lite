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
    public static extern int C_GetWindows_Count();

    [DllImport("libscreen_capture_lite")]
    public static extern void C_GetWindows_Index_Out(int index, out Window window);

    [DllImport("libscreen_capture_lite")]
    public static extern void C_GetWindows(IntPtr windows);

    [DllImport("libscreen_capture_lite")]
    public static extern int C_GetMonitors_Count();

    [DllImport("libscreen_capture_lite")]
    public static extern void C_GetMonitors_Index_Out(int index, out Monitor monitor);

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
        Texture.allowThreadedTextureCreation = true;

        int wCount = C_GetWindows_Count();
        Debug.Log($"number of windows: {wCount}");
        for (int i=0; i<wCount; i++) {
            Window window = new Window();
            C_GetWindows_Index_Out(i, out window);
            Debug.Log($"Window: {i} handle:{window.Handle} posX:{window.Position.x} posY:{window.Position.y} sizeX:{window.Size.x} sizeY:{window.Size.y} Name:{window.Name}");
        }

        // int mCount = C_GetMonitors_Count();
        // Debug.Log($"number of monitors: {mCount}");
        // for (int i=0; i<mCount; i++) {
        //     Monitor monitor = new Monitor();
        //     C_GetMonitors_Index_Out(i, out monitor);
        //     Debug.Log($"Monitors Id:{monitor.Id} Index:{monitor.Index} Adapter:{monitor.Adapter}");
        //     Debug.Log($"- Name:'{monitor.Name}'");
        //     Debug.Log($"- Height:{monitor.Height} Width:{monitor.Width} OriginalHeight:{monitor.OriginalHeight} OriginalWidth:{monitor.OriginalWidth} Scaling:{monitor.Scaling}");
        //     Debug.Log($"- OffsetX:{monitor.OffsetX} OffsetY:{monitor.OffsetY} OriginalOffsetX:{monitor.OriginalOffsetX} OriginalOffsetY:{monitor.OriginalOffsetY}");
        // }

        Window windowToCapture = new Window();
        C_GetWindows_Index_Out(windowIdToCapture, out windowToCapture);

        frameChangeDelegate = new ImageRefWindowRefCallbackType(OnFrameChanged);
        newFrameDelegate = new ImageRefWindowRefCallbackType(OnNewFrame);
        mouseChangedDelegate = new ImagePtrMousePointRefCallbackType(OnMouseChanged);

        Debug.Log($"- a -");
        C_ICaptureConfiguration(
            windowToCapture, 
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
        if ((w * h * 4) != s) {
            Debug.Log($"OnFrameChanged w:{w} h:{h} s:{s} : Invalid size of Data for given Size");
        }

        // Debug.Log($"OnFrameChanged w:{w} h:{h} s:{s}   Data: {onFrameChangedBytes[0]:x} {onFrameChangedBytes[1]:x} {onFrameChangedBytes[2]:x} {onFrameChangedBytes[3]:x} {onFrameChangedBytes[4]:x} {onFrameChangedBytes[5]:x} {onFrameChangedBytes[6]:x} {onFrameChangedBytes[7]:x}");
        onFrameChangedWidth = w;
        onFrameChangedHeight = h;
        onFrameChangedSize = s;
        onFrameChangedBytes = GetNativeArray<byte> (array, s);
    }

    [MonoPInvokeCallback (typeof (ImageRefWindowRefCallbackType))]
    void OnNewFrame (int w, int h, int s, IntPtr array) {
        if ((w * h * 4) != s) {
            Debug.Log($"OnNewFrame {w} {h} {s} : Invalid size of Data for given Size");
        }
        
        onNewFrameBytes = GetNativeArray<byte> (array, s);
        onNewFrameWidth = w;
        onNewFrameHeight = h;
        onNewFrameSize = s;
    }

    [MonoPInvokeCallback (typeof (ImagePtrMousePointRefCallbackType))]
    void OnMouseChanged (int w, int h, int s, IntPtr array) {
        if ((w * h * 4) != s) {
            Debug.Log($"OnMouseChanged {w} {h} {s} : Invalid size of Data for given Size");
            s = onMouseChangedSize = w * h * 4;
        }

        onMouseChangedBytes = GetNativeArray<byte> (array, s);
        onMouseChangedWidth = w;
        onMouseChangedHeight = h;
        onMouseChangedSize = s;
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
                Debug.Log($"Update.frameChangedTex.Exception: bytes:{onFrameChangedBytes.Length} w:{onFrameChangedWidth} h:{onFrameChangedHeight}  s:{onFrameChangedSize}");
                Debug.Log($"{e.ToString()}");
            }
        }



        // untested
        // if ((newFrameTex == null) || (newFrameTex.width != onNewFrameWidth) || (newFrameTex.height != onNewFrameHeight)) {
        //     newFrameTex = new Texture2D(onNewFrameWidth, onNewFrameHeight, TextureFormat.RGBA32, false);
        // }
        // if (onNewFrameBytes != null) {
        //     try {
        //         newFrameTex.LoadRawTextureData(onNewFrameBytes);
        //         onNewFrameBytes = null; 
        //     }
        //     catch (Exception e) {
        //         Debug.Log($"bytes:{onNewFrameBytes.Length} s:{onNewFrameSize} w:{onNewFrameWidth} h:{onNewFrameHeight}");
        //         Debug.Log($"{e.ToString()}");
        //     }
        // }

        // untested
        // if ((mouseChangedTex == null) || (mouseChangedTex.width != onMouseChangedWidth) || (mouseChangedTex.height != onMouseChangedHeight)) {
        //     mouseChangedTex = new Texture2D(onMouseChangedWidth, onMouseChangedHeight, TextureFormat.RGBA32, false);
        // }
        // if (onMouseChangedBytes != null) {
        //     try {
        //         mouseChangedTex.LoadRawTextureData(onMouseChangedBytes);
        //         onMouseChangedBytes = null;
        //     }
        //     catch (Exception e) {
        //         Debug.Log($"bytes:{onMouseChangedBytes.Length} s:{onMouseChangedSize} w:{onMouseChangedWidth} h:{onMouseChangedHeight}");
        //         Debug.Log($"{e.ToString()}");
        //     }
        // }
    }

}
