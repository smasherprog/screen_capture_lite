using System;
// using System.Diagnostics;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;
using AOT;
using UnityEngine.UI;

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
        if (GUILayout.Button("Save frameChangedTex")) {
            t.SaveTex(t.frameChangedTex, "frameChangedTex");
        }
        if (GUILayout.Button("Save newFrameTex")) {
            t.SaveTex(t.newFrameTex, "newFrameTex");
        }
        if (GUILayout.Button("Save mouseChangedTex")) {
            t.SaveTex(t.mouseChangedTex, "mouseChangedTex");
        }
    }    
}
#endif


public class TestScreenCaptureLite : MonoBehaviour {


#region Screen Capture Light API
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
 
    [DllImport("libscreen_capture_lite")]
    private static extern IntPtr C_GetMonitors(ref int size);

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
    public Window[] GetWindows () {
        int windowsCount = 0;
        IntPtr windowsArray = C_GetWindows(ref windowsCount);
        Window[] windows = GetNativeArray<Window> (windowsArray, windowsCount);
        return windows;
    }
 
    public Monitor[] GetMonitors () {
        int monitorCount = 0;
        IntPtr monitorsArray = C_GetWindows(ref monitorCount);
        Monitor[] monitors = GetNativeArray<Monitor> (monitorsArray, monitorCount);
        return monitors;
    }

    public delegate void ImageRefWindowRefCallbackType(int w, int h, int s, IntPtr array, IntPtr window);
    public delegate void ImagePtrMousePointRefCallbackType(int w, int h, int s, IntPtr array, IntPtr mousePoint);

    ImageRefWindowRefCallbackType frameChangedCallback;
    ImageRefWindowRefCallbackType newFameCallback;
    ImagePtrMousePointRefCallbackType mouseChangedCallback;

    ImageRefWindowRefCallbackType frameChangeDelegate = null;
    ImageRefWindowRefCallbackType newFrameDelegate = null;
    ImagePtrMousePointRefCallbackType mouseChangedDelegate = null;

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

    private static T GetNativeType<T>(IntPtr ptr) {
        return (T)Marshal.PtrToStructure (ptr, typeof(T));
    }

    public bool getOnFrameChanged = true;
    bool onFrameChanged = false;
    int onFrameChangedWidth;
    int onFrameChangedHeight;
    int onFrameChangedSize;
    // if there is data in the onFrameChangedBytes, the texture should be updated with it
    byte[] onFrameChangedBytes = new byte[0];
    Window onFrameChangedWindow;
    
    public bool getNewFrame = false;
    bool onNewFrame = false;
    int onNewFrameWidth;
    int onNewFrameHeight;
    int onNewFrameSize;
    byte[] onNewFrameBytes = new byte[0];
    Window onNewFrameWindow;
    
    public bool getMouseChanged = false;
    bool onMouseChanged = false;
    int onMouseChangedWidth;
    int onMouseChangedHeight;
    int onMouseChangedSize;
    byte[] onMouseChangedBytes = new byte[0];
    MousePoint onMouseChangedMousePoint;

    List<string> debugMessages = new List<string>();

    int frameChangedCounter;
    int newFrameCounter;
    int mouseChangedCounter;
    System.Diagnostics.Stopwatch frameChangedStopwatch;
    System.Diagnostics.Stopwatch newFrameStopwatch;
    System.Diagnostics.Stopwatch mouseChangedStopwatch;

    void FpsCounter (string log, ref int counter, ref System.Diagnostics.Stopwatch lastTimer) {
        counter ++;
        if (lastTimer == null) {
            lastTimer = new System.Diagnostics.Stopwatch();
            lastTimer.Start();
        }
        if (lastTimer.Elapsed.Seconds > 0) {
            float fps = counter / (float)lastTimer.Elapsed.TotalSeconds;
            debugMessages.Add($"{log} fps {fps}");
            lastTimer.Restart();
            counter = 0;
        }
    }

    // unity doesnt allow writing the texture here
    [MonoPInvokeCallback (typeof (ImageRefWindowRefCallbackType))]
    void OnFrameChanged (int w, int h, int s, IntPtr array, IntPtr windowPtr) {
        // Debug.Log($"OnFrameChanged w:{w} h:{h} s:{s}   Data: {onFrameChangedBytes[0]:x} {onFrameChangedBytes[1]:x} {onFrameChangedBytes[2]:x} {onFrameChangedBytes[3]:x} {onFrameChangedBytes[4]:x} {onFrameChangedBytes[5]:x} {onFrameChangedBytes[6]:x} {onFrameChangedBytes[7]:x}");
        onFrameChangedWidth = w;
        onFrameChangedHeight = h;
        onFrameChangedSize = s;
        if (onFrameChangedBytes.Length != s) {
            onFrameChangedBytes = new byte[s];
        }
        Marshal.Copy(array, onFrameChangedBytes, 0, onFrameChangedBytes.Length);
        onFrameChanged = true;
        // onFrameChangedBytes = GetNativeArray<byte> (array, s);
        onFrameChangedWindow = GetNativeType<Window> (windowPtr);

        FpsCounter("frameChanged", ref frameChangedCounter, ref frameChangedStopwatch);

        // debugMessages.Add($"OnFrameChanged {frameChangedCounter}: w:{w} h:{h} *:{(w*h)} s:{s} l:{onFrameChangedBytes.Length}");

        if ((w != onFrameChangedWindow.Size.x) || (h != onFrameChangedWindow.Size.y)) {
            debugMessages.Add($"OnFrameChanged {frameChangedCounter}: img:{w}/{h} win:{onFrameChangedWindow.Size.x}/{onFrameChangedWindow.Size.y} differ");
        }

        if ((w * h * 4) != s) {
            debugMessages.Add($"OnFrameChanged {frameChangedCounter}: w:{w} h:{h} *:{(w*h)} s:{s} l:{onFrameChangedBytes.Length} : Invalid size of Data for given Size");
            onFrameChangedBytes = null;
        }
    }

    [MonoPInvokeCallback (typeof (ImageRefWindowRefCallbackType))]
    void OnNewFrame (int w, int h, int s, IntPtr array, IntPtr windowPtr) {
        onNewFrameWidth = w;
        onNewFrameHeight = h;
        onNewFrameSize = s;
        if (onNewFrameBytes.Length != s) {
            onNewFrameBytes = new byte[s];
        }
        Marshal.Copy(array, onNewFrameBytes, 0, onNewFrameBytes.Length);
        // onNewFrameBytes = GetNativeArray<byte> (array, s);
        onNewFrameWindow = GetNativeType<Window> (windowPtr);
        onNewFrame = true;

        FpsCounter("newFrame", ref newFrameCounter, ref newFrameStopwatch);

        // debugMessages.Add($"OnNewFrame {newFrameCounter}: w:{w} h:{h} *:{(w*h)} s:{s} l:{onNewFrameBytes.Length}");

        if ((w != onNewFrameWindow.Size.x) || (h != onNewFrameWindow.Size.y)) {
            debugMessages.Add($"OnNewFrame {newFrameCounter}: img:{w}/{h} win:{onNewFrameWindow.Size.x}/{onNewFrameWindow.Size.y} differ");
        }

        if ((w * h * 4) != s) {
            debugMessages.Add($"OnNewFrame {newFrameCounter}: w:{w} h:{h} *:{(w*h)} s:{s} l:{onNewFrameBytes.Length} : Invalid size of Data for given Size");
            onNewFrameBytes = null;
        }
    }

    [MonoPInvokeCallback (typeof (ImagePtrMousePointRefCallbackType))]
    void OnMouseChanged (int w, int h, int s, IntPtr array, IntPtr mposPtr) {
        onMouseChangedWidth = w;
        onMouseChangedHeight = h;
        onMouseChangedSize = s;
        // onMouseChangedBytes = GetNativeArray<byte> (array, s);
        if (onMouseChangedBytes.Length != s) {
            onMouseChangedBytes = new byte[s];
        }
        Marshal.Copy(array, onMouseChangedBytes, 0, onMouseChangedBytes.Length);
        onMouseChanged = true;
        onMouseChangedMousePoint = GetNativeType<MousePoint> (mposPtr);

        // FpsCounter("mouseChanged", ref mouseChangedCounter, ref mouseChangedStopwatch);

        if ((w * h * 4) != s) {
            debugMessages.Add($"OnMouseChanged {mouseChangedCounter}: w:{w} h:{h} *:{(w*h)} s:{s} l:{onMouseChangedBytes.Length} : Invalid size of Data for given Size");
            onMouseChangedBytes = null;
        }
    }

    // convert the texture data to textures to use in unity
    public void UpdateTextures () {
        if (debugMessages.Count > 0) {
            for (int i=0; i<debugMessages.Count; i++ ){
                Debug.Log(debugMessages[i]);
            }
            debugMessages.Clear();
        }
        if (onFrameChanged) {
            onFrameChanged = false;
            if ((frameChangedTex == null) || (frameChangedTex.width != onFrameChangedWidth) || (frameChangedTex.height != onFrameChangedHeight)) {
                Debug.Log($"Update.frameChangedTex: new Texture {onFrameChangedWindow.Size.x} {onFrameChangedWindow.Size.y} {onFrameChangedWidth} {onFrameChangedHeight}");
                frameChangedTex = new Texture2D(onFrameChangedWidth, onFrameChangedHeight, TextureFormat.RGBA32, false);
                rawImage1.texture = frameChangedTex;
            }
            try {
                // Debug.Log($"Update.frameChangedTex: {onFrameChangedWidth} {onFrameChangedHeight} {onFrameChangedSize}");
                // Debug.Log($"Update.frameChange: {onFrameChangedWindow.Position.x} {onFrameChangedWindow.Position.y}");
                text1.text = $"pos: {onFrameChangedWindow.Position.x}/{onFrameChangedWindow.Position.y} size: {onFrameChangedWindow.Size.x}/{onFrameChangedWindow.Size.y}";
                frameChangedTex.LoadRawTextureData(onFrameChangedBytes);
                // onFrameChangedBytes = null;
                frameChangedTex.Apply();
            }
            catch (Exception e) {
                Debug.LogError($"Update.frameChanged.Exception: bytes:{onFrameChangedBytes.Length} w:{onFrameChangedWidth} h:{onFrameChangedHeight} s:{onFrameChangedSize} *:{(onFrameChangedWidth*onFrameChangedHeight)}");
                Debug.LogError($"{e.ToString()}");
            }
        }
        if (onNewFrame) {
            onNewFrame = false;
            if ((newFrameTex == null) || (newFrameTex.width != onNewFrameWidth) || (newFrameTex.height != onNewFrameHeight)) {
                Debug.Log($"Update.newFrameTex: new Texture {onNewFrameWindow.Size.x} {onNewFrameWindow.Size.y} {onNewFrameWidth} {onNewFrameHeight}");
                newFrameTex = new Texture2D(onNewFrameWidth, onNewFrameHeight, TextureFormat.RGBA32, false);
                rawImage2.texture = newFrameTex;
            }
            try {
                // Debug.Log($"Update.newFrameTex: {onNewFrameWidth} {onNewFrameHeight} {onNewFrameSize}");
                // Debug.Log($"Update.newFrame: {onNewFrameWindow.Position.x} {onNewFrameWindow.Position.y}");
                text2.text = $"pos: {onNewFrameWindow.Position.x}/{onNewFrameWindow.Position.y} size: {onNewFrameWindow.Size.x}/{onNewFrameWindow.Size.y}";
                newFrameTex.LoadRawTextureData(onNewFrameBytes);
                // onNewFrameBytes = null;
                newFrameTex.Apply();
            }
            catch (Exception e) {
                Debug.LogError($"Update.onNewFrame.Exception: bytes:{onNewFrameBytes.Length} w:{onNewFrameWidth} h:{onNewFrameHeight} s:{onNewFrameSize} *:{(onNewFrameWidth*onNewFrameHeight)}");
                Debug.LogError($"{e.ToString()}");
            }
        }
        if (onMouseChanged) {
            onMouseChanged = false;
            if ((mouseChangedTex == null) || (mouseChangedTex.width != onMouseChangedWidth) || (mouseChangedTex.height != onMouseChangedHeight)) {
                Debug.Log($"Update.mouseChanged: new Texture {onMouseChangedWidth} {onMouseChangedHeight}");
                mouseChangedTex = new Texture2D(onMouseChangedWidth, onMouseChangedHeight, TextureFormat.RGBA32, false);
                rawImage3.texture = mouseChangedTex;
            }
            try {
                // Debug.Log($"Update.onMouseChanged: {onMouseChangedWidth} {onMouseChangedHeight} {onMouseChangedSize}");
                text3.text = $"pos: {onMouseChangedMousePoint.Position.x}/{onMouseChangedMousePoint.Position.y} hotspot: {onMouseChangedMousePoint.HotSpot.x}/{onMouseChangedMousePoint.HotSpot.y}";
                mouseChangedTex.LoadRawTextureData(onMouseChangedBytes);
                // onMouseChangedBytes = null;
                mouseChangedTex.Apply();
            }
            catch (Exception e) {
                Debug.LogError($"Update.onMouseChanged.Exception: bytes:{onMouseChangedBytes.Length} w:{onMouseChangedWidth} h:{onMouseChangedHeight} s:{onMouseChangedSize} *:{(onMouseChangedWidth*onMouseChangedHeight)}");
                Debug.LogError($"{e.ToString()}");
            }
        }

        if (C_Capture_ExpectedErrorEvent()) {
            Debug.LogWarning("Capturing Encountered Expected Error (like window resize)");
        }
    }
#endregion Screen Capture Light Unity API


#region Screen Capture Light Unity Testing
    public int windowIdToCapture = 0;

    [HideInInspector]
    public Texture2D frameChangedTex;
    [HideInInspector]
    public Texture2D newFrameTex;
    [HideInInspector]
    public Texture2D mouseChangedTex;

    public RawImage rawImage1;
    public RawImage rawImage2;
    public RawImage rawImage3;
    
    public Text topText;
    public Text text1;
    public Text text2;
    public Text text3;
    

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

        if (getOnFrameChanged) {
            frameChangeDelegate = new ImageRefWindowRefCallbackType(OnFrameChanged);
        }
        if (getNewFrame) {
            newFrameDelegate = new ImageRefWindowRefCallbackType(OnNewFrame);
        }
        if (getMouseChanged) {
            mouseChangedDelegate = new ImagePtrMousePointRefCallbackType(OnMouseChanged);
        }

        Debug.Log($"- a -");
        C_ICaptureConfiguration( windows[windowIdToCapture], frameChangeDelegate, newFrameDelegate, mouseChangedDelegate );
        Debug.Log($"- b -");

        topText.text = $"grabbing window: {windows[windowIdToCapture].Name} {windows[windowIdToCapture].Position.x} {windows[windowIdToCapture].Position.y}";

        C_Capture_SetMouseChangeInterval(1000/24);
        C_Capture_SetFrameChangeInterval(1000/24);
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

   // write the texture 
    public void SaveTex (Texture2D tex, string name) {
        String dirPath = Application.dataPath + "/../" + name + ".png";
        byte[] _bytes = tex.EncodeToPNG();
        System.IO.File.WriteAllBytes(dirPath, _bytes);
        Debug.Log(_bytes.Length/1024  + "Kb was saved as: " + dirPath);
    }

    void Update () {
        // read the data received by the callbacks and store them into the textures
        UpdateTextures ();
    }

#endregion
}
