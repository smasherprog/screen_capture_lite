using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;
using AOT;
using UnityEngine.UI;
using UnityEngine.Events;

#if UNITY_EDITOR
using UnityEditor;
[CustomEditor(typeof(TestScreenCaptureLite))]
public class TestScreenCaptureLiteInspector : Editor {
    public override void OnInspectorGUI() {
        base.DrawDefaultInspector();
        
        TestScreenCaptureLite t = (TestScreenCaptureLite)target;
        if (GUILayout.Button("Save frameChangedTex")) {
            for (int i=0; i<t.frameChangedTex.Length; i++) {
                t.SaveTex(t.frameChangedTex[i], $"frameChangedTex_{i}_");
            }
        }
        if (GUILayout.Button("Save newFrameTex")) {
            for (int i=0; i<t.newFrameTex.Length; i++) {
                t.SaveTex(t.newFrameTex[i], $"newFrameTex_{i}_");
            }
        }
        if (GUILayout.Button("Save mouseChangedTex")) {
            t.SaveTex(t.mouseChangedTex, "mouseChangedTex");
        }
    }    
}
#endif


public class TestScreenCaptureLite : MonoBehaviour {


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

    [StructLayout(LayoutKind.Sequential, Size = 4+8+8+128, Pack = 4)]
    public struct Window {
        // not sure wether this type is correct, but uint is _not_ working correctly
        // https://stackoverflow.com/questions/32906774/what-is-equal-to-the-c-size-t-in-c-sharp/32907246
        public UIntPtr Handle; // size_t in c++
        public Point Position;
        public Point Size;
        // https://www.mono-project.com/docs/advanced/pinvoke/
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

    // maybe check: https://www.mono-project.com/docs/advanced/pinvoke/
    [StructLayout(LayoutKind.Sequential, Size=176, Pack = 4)] //, Size=180 , Pack = 8 CharSet.Ansi or CharSet.Unicode or CharSet.Auto
    public struct Monitor {
        // https://www.mono-project.com/docs/advanced/pinvoke/
        [MarshalAs (UnmanagedType.ByValTStr, SizeConst=128)] // works, but not correct output
        // [MarshalAs (UnmanagedType.LPWStr, SizeConst=128)] // BPTStr crashes
        public string Name;
        public int Id;
        public int Index;
        public int Adapter;
        public int Height;
        public int Width;
        public int OriginalHeight;
        public int OriginalWidth;
        // Offsint are the number of pixels that a monitor can be from the origin. For example, users can shuffle their
        // moniints around so this affects their offset.
        public int OffsetX;
        public int OffsetY;
        public int OriginalOffsetX;
        public int OriginalOffsetY;
        // // https://www.mono-project.com/docs/advanced/pinvoke/
        // [MarshalAs (UnmanagedType.ByValTStr, SizeConst=128)] // works, but not correct output
        // // [MarshalAs (UnmanagedType.LPWStr, SizeConst=128)] // BPTStr crashes
        // public string Name;
        public float Scaling;
    }



    // [StructLayout(LayoutKind.Explicit, Size=188, Pack = 4 )] //, Size=180  CharSet.Ansi or CharSet.Unicode or CharSet.Auto
    // public struct Monitor {
    //     [FieldOffset(0)] public int Id;
    //     [FieldOffset(2)] public int Index;
    //     [FieldOffset(4)] public int Adapter;
    //     [FieldOffset(6)] public int Height;
    //     [FieldOffset(8)] public int Width;
    //     [FieldOffset(10)] public int OriginalHeight;
    //     [FieldOffset(12)] public int OriginalWidth;
    //     // Offsint are the number of pixels that a monitor can be from the origin. For example, users can shuffle their
    //     // moniints around so this affects their offset.
    //     [FieldOffset(14)] public int OffsetX;
    //     [FieldOffset(16)] public int OffsetY;
    //     [FieldOffset(18)] public int OriginalOffsetX;
    //     [FieldOffset(20)] public int OriginalOffsetY;
    //     // https://www.mono-project.com/docs/advanced/pinvoke/
    //     [MarshalAs (UnmanagedType.ByValTStr, SizeConst=128)]
    //     // [MarshalAs (UnmanagedType.LPTStr, SizeConst=128)]
    //     [FieldOffset(20)] public string Name;
    //     [FieldOffset(176)] public float Scaling;
    // }

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
        Debug.Log($"detected size {size}");

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

    // do we activate the callbacks
    public bool getOnFrameChanged = true;
    public bool getNewFrame = false;
    public bool getMouseChanged = false;

    List<WindowCallbackParams> onFrameChangedParams = new List<WindowCallbackParams>();
    List<WindowCallbackParams> onNewFrameParams = new List<WindowCallbackParams>();
    
    bool onMouseChanged = false;
    int onMouseChangedWidth;
    int onMouseChangedHeight;
    int onMouseChangedSize;
    byte[] onMouseChangedBytes = new byte[0];
    MousePoint[] onMouseChangedMousePoints;

    // unity doesnt allow writing the texture here
    [MonoPInvokeCallback (typeof (ImageRefWindowRefCallbackType))]
    void OnFrameChanged (int w, int h, int s, IntPtr array, IntPtr windowPtr) {
        Window win = GetNativeType<Window> (windowPtr);

        // Debug.Log($"OnFrameChanged {win.Handle} w:{w} h:{h} s:{s}");

        int index = onFrameChangedParams.FindIndex(a => a.window.Handle == win.Handle);
        if (index == -1) {
            // Debug.Log($"OnFrameChanged {index} {win.Handle} Create w:{w} h:{h} s:{s}");
            WindowCallbackParams windowCallbackParams = new WindowCallbackParams() {width=w, height=h, window=win};
            onFrameChangedParams.Add(windowCallbackParams);
            index = onFrameChangedParams.FindIndex(a => a.window.Handle == win.Handle);

            Dropdown.OptionData optionData = new Dropdown.OptionData();
            optionData.text = ((int)win.Handle).ToString();
            dropdown1.options.Add(optionData);
        }

        if (index != -1) {
            // Debug.Log($"OnFrameChanged Update w:{w} h:{h} s:{s}");
            onFrameChangedParams[index].Changed (w, h, s, array, win);
        }
    }

    [MonoPInvokeCallback (typeof (ImageRefWindowRefCallbackType))]
    void OnNewFrame (int w, int h, int s, IntPtr array, IntPtr windowPtr) {
        Window win = GetNativeType<Window> (windowPtr);

        // Debug.Log($"OnNewFrame {win.Handle} w:{w} h:{h} s:{s}");

        int index = onNewFrameParams.FindIndex(a => a.window.Handle == win.Handle);
        if (index == -1) {
            // Debug.Log($"OnNewFrame {index} {win.Handle} Create w:{w} h:{h} s:{s}");
            WindowCallbackParams windowCallbackParams = new WindowCallbackParams() {width=w, height=h, window=win};
            onNewFrameParams.Add(windowCallbackParams);
            index = onNewFrameParams.FindIndex(a => a.window.Handle == win.Handle);
        }

        if (index != -1) {
            // Debug.Log($"OnNewFrame Update w:{w} h:{h} s:{s}");
            onNewFrameParams[index].Changed (w, h, s, array, win);
        }
    }

    [MonoPInvokeCallback (typeof (ImagePtrMousePointRefCallbackType))]
    void OnMouseChanged (int w, int h, int s, IntPtr array, IntPtr mposPtr) {
        // onMouseChangedWidth = w;
        // onMouseChangedHeight = h;
        // onMouseChangedSize = s;
        // if (onMouseChangedBytes.Length != s) {
        //     onMouseChangedBytes = new byte[s];
        // }
        // onMouseChanged = true;
        // MousePoint onMouseChangedMousePoint = GetNativeType<MousePoint> (mposPtr);

        // Marshal.Copy(array, onMouseChangedBytes, 0, onMouseChangedBytes.Length);

        // // FpsCounter("mouseChanged", ref mouseChangedCounter, ref mouseChangedStopwatch);

        // if ((w * h * 4) != s) {
        //     debugMessages.Add($"OnMouseChanged {mouseChangedCounter}: w:{w} h:{h} *:{(w*h)} s:{s} l:{onMouseChangedBytes.Length} : Invalid size of Data for given Size");
        //     onMouseChangedBytes = null;
        // }
    }

    // convert the texture data to textures to use in unity
    public void UpdateTextures () {
        WindowCallbackParams parms;

        for (int i=0; i<onFrameChangedParams.Count; i++) {
            // Debug.Log($"UpdateTextures onFrameChangedParams {i}");
            parms = onFrameChangedParams[i];
            parms.UpdateTexture();
        }

        if (dropdown1.options.Count > 0) {
            parms = onFrameChangedParams[dropdown1.value];
            rawImage1.texture = parms.texture;
            text1.text = $"onFrameChangedParams pos: {parms.window.Position.x}/{parms.window.Position.y} size: {parms.window.Size.x}/{parms.window.Size.y}";
        }

        for (int i=0; i<onNewFrameParams.Count; i++) {
            // Debug.Log($"UpdateTextures onNewFrameParams {i}");
            parms = onNewFrameParams[i];
            parms.UpdateTexture();
            rawImage2.texture = parms.texture;
            text2.text = $"onNewFrameParams pos: {parms.window.Position.x}/{parms.window.Position.y} size: {parms.window.Size.x}/{parms.window.Size.y}";
        }

        if (dropdown2.options.Count > 0) {
            parms = onNewFrameParams[dropdown2.value];
            rawImage2.texture = parms.texture;
            text2.text = $"onNewFrameParams pos: {parms.window.Position.x}/{parms.window.Position.y} size: {parms.window.Size.x}/{parms.window.Size.y}";
        }

        // if (onMouseChanged) {
        //     onMouseChanged = false;
        //     if ((mouseChangedTex == null) || (mouseChangedTex.width != onMouseChangedWidth) || (mouseChangedTex.height != onMouseChangedHeight)) {
        //         Debug.Log($"Update.mouseChanged: new Texture {onMouseChangedWidth} {onMouseChangedHeight}");
        //         mouseChangedTex = new Texture2D(onMouseChangedWidth, onMouseChangedHeight, TextureFormat.RGBA32, false);
        //         rawImage3.texture = mouseChangedTex;
        //     }
        //     try {
        //         // Debug.Log($"Update.onMouseChanged: {onMouseChangedWidth} {onMouseChangedHeight} {onMouseChangedSize}");
        //         text3.text = $"pos: {onMouseChangedMousePoint.Position.x}/{onMouseChangedMousePoint.Position.y} hotspot: {onMouseChangedMousePoint.HotSpot.x}/{onMouseChangedMousePoint.HotSpot.y}";
        //         mouseChangedTex.LoadRawTextureData(onMouseChangedBytes);
        //         // onMouseChangedBytes = null;
        //         mouseChangedTex.Apply();
        //     }
        //     catch (Exception e) {
        //         Debug.LogError($"Update.onMouseChanged.Exception: bytes:{onMouseChangedBytes.Length} w:{onMouseChangedWidth} h:{onMouseChangedHeight} s:{onMouseChangedSize} *:{(onMouseChangedWidth*onMouseChangedHeight)}");
        //         Debug.LogError($"{e.ToString()}");
        //     }
        // }

        if (C_Capture_ExpectedErrorEvent()) {
            Debug.LogWarning("Capturing Encountered Expected Error (like window resize)");
        }
    }
#endregion Screen Capture Light Unity API


#region Screen Capture Light Unity Testing
    // public int windowIdToCapture = 0;
    public List<int> captureWindowIds = new List<int>();

    [HideInInspector]
    public Texture2D[] frameChangedTex;
    [HideInInspector]
    public Texture2D[] newFrameTex;
    [HideInInspector]
    public Texture2D mouseChangedTex;

    public RawImage rawImage1;
    public RawImage rawImage2;
    public RawImage rawImage3;
    
    public Text topText;
    public Text text1;
    public Text text2;
    public Text text3;
    
    public Dropdown dropdown1;
    // Dropdown.OptionData dropdown1OptionData;
    public Dropdown dropdown2;
    Dropdown.OptionData dropdown2OptionData;
    public Dropdown dropdown3;
    Dropdown.OptionData dropdown3OptionData;


    void Start() {
        Window[] systemWindows = GetWindows();
        Window[] captureWindows = new Window[captureWindowIds.Count];
        for (int i=0; i<systemWindows.Length; i++) {
            // Debug.Log($"Window: {i} handle:{systemWindows[i].Handle} posX:{systemWindows[i].Position.x} posY:{systemWindows[i].Position.y} sizeX:{systemWindows[i].Size.x} sizeY:{systemWindows[i].Size.y} Name:'{systemWindows[i].Name}'");
            if (captureWindowIds.Contains(i)) {
                Debug.Log($"Window: CAPTURE {i} handle:{systemWindows[i].Handle} posX:{systemWindows[i].Position.x} posY:{systemWindows[i].Position.y} sizeX:{systemWindows[i].Size.x} sizeY:{systemWindows[i].Size.y} Name:'{systemWindows[i].Name}'");
                captureWindows[captureWindowIds.IndexOf(i)] = systemWindows[i];
            }
        }

        // monitors dont work at all yet, values are wrong?
        Monitor[] monitors = GetMonitors();
        for (int i=0; i<monitors.Length; i++) {
            Monitor monitor = monitors[i];
            string log = "";
            log += $"Monitors {i} Id:{monitor.Id} Index:{monitor.Index} Adapter:{monitor.Adapter}";
            log += $"- Name:'{monitor.Name}' Scaling:{monitor.Scaling}";
            // Debug.Log($"- MonitorSize: {monitor.Height}/{monitor.Width} MonitorOriginalSize: {monitor.OriginalHeight}/{monitor.OriginalWidth}");
            // Debug.Log($"- Offset: {monitor.OffsetX}/{monitor.OffsetY} MonitorOriginalOffset:{monitor.OriginalOffsetX}/{monitor.OriginalOffsetY}");
            Debug.Log(log);
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

        // IntPtr pnt = Marshal.AllocHGlobal(Marshal.SizeOf(captureWindows));
        // Marshal.StructureToPtr(captureWindows, pnt, false);
        // Debug.Log($"- a -");
        // C_ICaptureConfiguration( pnt, captureWindows.Length, frameChangeDelegate, newFrameDelegate, mouseChangedDelegate );
        // Debug.Log($"- b -");
        // Marshal.FreeHGlobal(pnt);

        GCHandle pinnedArray = GCHandle.Alloc(captureWindows, GCHandleType.Pinned);
        IntPtr ptr = pinnedArray.AddrOfPinnedObject();
        Debug.Log($"- a -");
        C_ICaptureConfiguration( ptr, captureWindows.Length, frameChangeDelegate, newFrameDelegate, mouseChangedDelegate );
        Debug.Log($"- b -");
        pinnedArray.Free();

        topText.text = "";
        foreach (Window win in captureWindows) {
            topText.text += $"grabbing window: {win.Name} {win.Position.x}/{win.Position.y}";
        }

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
