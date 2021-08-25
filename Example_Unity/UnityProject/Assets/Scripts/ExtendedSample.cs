using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;
using AOT;
using UnityEngine.UI;
using UnityEngine.Events;
using SL;
using System.Linq;
using System.Threading;

#if UNITY_EDITOR
using UnityEditor;
[CustomEditor(typeof(ExtendedSample))]
public class ExtendedSampleInspector : Editor {
    public override void OnInspectorGUI() {
        base.DrawDefaultInspector();
        
        ExtendedSample t = (ExtendedSample)target;
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


public class ExtendedSample : MonoBehaviour {

#region Screen Capture Light Unity Testing
    public List<int> captureWindowIds = new List<int>();
    // do we activate the callbacks
    public bool captureOnFrameChanged = true;
    public bool captureNewFrame = false;
    public bool captureMouseChanged = false;


    [HideInInspector]
    public Texture2D[] frameChangedTex;
    [HideInInspector]
    public Texture2D[] newFrameTex;
    [HideInInspector]
    public Texture2D mouseChangedTex;

    public RawImage rawImage1;
    public RawImage rawImage2;
    public RawImage rawImage3;
    public RawImage rawImage4;
    
    public Text topText;
    public Text text1;
    public Text text2;
    public Text text3;
    public Text text4;
    
    public Dropdown dropdown1;
    public Dropdown dropdown2;

    private static void WriteLine( Screen_Capture.Monitor p)
    {
        Debug.Log($"Id = {p.Id} Index = {p.Index} Height = {p.Height} Width = {p.Width} OffsetX = {p.OffsetX} OffsetY= {p.OffsetY} Name= {p.Name}");
    }
    private static void WriteLine( Screen_Capture.Window p)
    {
        Debug.Log($"Name = {p.Name} Postion.x = {p.Position.x} Postion.y = {p.Position.y} Size.x = {p.Size.x} Size.y = {p.Size.y}");
    }
    private static void WriteLine( Screen_Capture.Image p)
    {
        Debug.Log($"BytesToNextRow = {p.BytesToNextRow} isContiguous = {p.isContiguous} Bounds.bottom = {p.Bounds.bottom} Bounds.left = {p.Bounds.left} Bounds.right = {p.Bounds.right} Bounds.top = {p.Bounds.top}");
    }
    static int onNewFramecounter = 0;
    static DateTime onNewFramestart = DateTime.Now;

    Screen_Capture.Image onNewFrameScreenImage;
    public byte[] onNewFrameScreenBytes = new byte[0];
    public byte[] reversedOnNewFrameScreenBytes = new byte[0];

    public Texture2D onNewFrameScreenTexture;
    public bool onNewFrameChanged = false;
    // Screen_Capture.Image onFrameChangedScreenImage;
    // Texture2D onFrameChangedScreenTexture;
    // Screen_Capture.Image onMouseChangedScreenImage;
    // Texture2D onMouseChangedScreenTexture;


    public void MonitorOnNewFrame ( Screen_Capture.Image img,  Screen_Capture.Monitor monitor) {
        try {
            if (framgrabber == null)
                return;
            onNewFrameChanged = true;

            onNewFrameScreenImage = img;

            // the data buffer cannot be used, we need to copy it right now
            int byteCount = img.Bounds.width * img.Bounds.height * 4;
            if (reversedOnNewFrameScreenBytes.Length != byteCount) {
                reversedOnNewFrameScreenBytes = new byte[byteCount];
            }
            
            // image is upside down
            Marshal.Copy(img.Data, reversedOnNewFrameScreenBytes, 0, reversedOnNewFrameScreenBytes.Length);
        }
        catch (Exception e) {
            Debug.LogWarning("can happen when stopping unity while the screen capture thread has not stopped in time");
        }
    }

    public Screen_Capture.ScreenCaptureManager createframegrabber() {
        onNewFramecounter = 0;
        onNewFramestart = DateTime.Now;
        Screen_Capture.CaptureConfigurationData<Screen_Capture.MonitorType> captureConfigurationData = 
            Screen_Capture.CaptureConfiguration.CreateCaptureConfiguration(() => {
                Screen_Capture.Monitor[] mons = Screen_Capture.GetMonitors();
                Debug.Log("Library is requesting the list of monitors to capture!");
                for (int i = 0; i < mons.Length; ++i) {
                    WriteLine( mons[i]);
                }
            return mons;
        });

        captureConfigurationData.onNewFrame( MonitorOnNewFrame );

        // captureConfigurationData.onFrameChanged(( Screen_Capture.Image img,  Screen_Capture.Monitor monitor) => {
        //     //var newBitmap = new Bitmap(img.Bounds.right - img.Bounds.left, img.Bounds.bottom - img.Bounds.top, img.BytesToNextRow, System.Drawing.Imaging.PixelFormat.Format32bppRgb, img.Data);
        //     //newBitmap.Save($"{onNewFramecounter++}onFrameChanged.jpg", System.Drawing.Imaging.ImageFormat.Jpeg);
        //     //WriteLine( img);
        //     //WriteLine( monitor); 
        //     // onFrameChangedScreenImage = img;
        // });
        // captureConfigurationData.onMouseChanged((Screen_Capture.Image img, Screen_Capture.MousePoint mousePoint) => { 
        //     // if (img != null) {
        //     //     // var newBitmap = new Bitmap(img.Bounds.right - img.Bounds.left, img.Bounds.bottom - img.Bounds.top, img.BytesToNextRow, System.Drawing.Imaging.PixelFormat.Format32bppArgb, img.Data);
        //     //     // newBitmap.Save($"{onNewFramecounter++}NewMouseImage.jpg", System.Drawing.Imaging.ImageFormat.Png);
        //     //     WriteLine(img);
        //     //     Debug.Log("New Mouse image");
        //     //     onMouseChangedScreenImage = img;
        //     // }
        //     // else { 
        //     //     Debug.Log("Mouse Moved"); 
        //     // }
        // });

        Screen_Capture.ScreenCaptureManager framgrabber = captureConfigurationData.start_capturing();

        framgrabber.setFrameChangeInterval(100);
        framgrabber.setMouseChangeInterval(100);

        return framgrabber;
    }

    // Screen_Capture.Image onNewFrameWindowImage;
    // Texture2D onNewFrameWindowTexture;
    // Screen_Capture.Image onFrameChangedWindowImage;
    // Texture2D onFrameChangedWindowTexture;
    // Screen_Capture.Image onMouseChangedWindowImage;
    // Texture2D onMouseChangedWindowTexture;

    // public void OnNewFrameWindowCallback ( Screen_Capture.Image img,  Screen_Capture.Window monitor) {
    //     WriteLine( img);
    //     WriteLine( monitor); 
    //     //var newBitmap = new Bitmap(img.Bounds.right - img.Bounds.left, img.Bounds.bottom - img.Bounds.top, img.BytesToNextRow, System.Drawing.Imaging.PixelFormat.Format32bppRgb, img.Data);
    //     //newBitmap.Save($"{onNewFramecounter++}onNewFrame.jpg", System.Drawing.Imaging.ImageFormat.Jpeg);
    //     // if ((DateTime.Now - onNewFramestart).TotalMilliseconds > 1000) {
    //     //     Debug.Log("onNewFrame fps" + onNewFramecounter);
    //     //     onNewFramestart = DateTime.Now;
    //     //     onNewFramecounter = 0;
    //     // }
    //     // Interlocked.Increment(ref onNewFramecounter);
    //     // onNewFrameWindowImage = img;
    // }

    // public Screen_Capture.ScreenCaptureManager createwindowgrabber()
    // {
    //     onNewFramecounter = 0;
    //     onNewFramestart = DateTime.Now;
    //     var captureConfigurationData = Screen_Capture.CaptureConfiguration.CreateCaptureConfiguration(() => {
    //         var windows = Screen_Capture.GetWindows();
    //         Debug.Log("Library is requesting the list of windows to capture!");
    //         for (int i = 0; i < windows.Length; ++i) {
    //             WriteLine(windows[i]);
    //         }
    //         return windows.Where(a => a.Name.ToLower().Contains("unity")).ToArray();
    //     });
    //     captureConfigurationData.onNewFrame(( Screen_Capture.Image img,  Screen_Capture.Window monitor) => {
    //             // WriteLine( img);
    //             // WriteLine( monitor); 
    //             //var newBitmap = new Bitmap(img.Bounds.right - img.Bounds.left, img.Bounds.bottom - img.Bounds.top, img.BytesToNextRow, System.Drawing.Imaging.PixelFormat.Format32bppRgb, img.Data);
    //             //newBitmap.Save($"{onNewFramecounter++}onNewFrame.jpg", System.Drawing.Imaging.ImageFormat.Jpeg);
    //             // if ((DateTime.Now - onNewFramestart).TotalMilliseconds > 1000) {
    //             //     Debug.Log("onNewFrame fps" + onNewFramecounter);
    //             //     onNewFramestart = DateTime.Now;
    //             //     onNewFramecounter = 0;
    //             // }
    //             // Interlocked.Increment(ref onNewFramecounter);
    //             // onNewFrameImage = img;
    //         }
    //     );
    //     captureConfigurationData.onFrameChanged(( Screen_Capture.Image img,  Screen_Capture.Window monitor) => {
    //         //var newBitmap = new Bitmap(img.Bounds.right - img.Bounds.left, img.Bounds.bottom - img.Bounds.top, img.BytesToNextRow, System.Drawing.Imaging.PixelFormat.Format32bppRgb, img.Data);
    //         //newBitmap.Save($"{onNewFramecounter++}onFrameChanged.jpg", System.Drawing.Imaging.ImageFormat.Jpeg);
    //         //WriteLine( img);
    //         //WriteLine( monitor); 
    //         // onFrameChangedWindowImage = img;
    //     });
    //     captureConfigurationData.onMouseChanged(( Screen_Capture.Image img,  Screen_Capture.MousePoint mousePoint) => {
    //         // if (img != null) {
    //         //     Debug.Log("onNewFrame fps" + onNewFramecounter);
    //         // }
    //         // else {
    //         //     Debug.Log("onNewFrame fps" + onNewFramecounter);
    //         // }
    //         // onMouseChangedWindowImage = img;
    //     });
        
    //     var framgrabber = captureConfigurationData.start_capturing();
    //     framgrabber.setFrameChangeInterval(100);
    //     framgrabber.setMouseChangeInterval(100);
    //     return framgrabber;
    // }

    void Update () {
        if (onNewFrameChanged) {
            onNewFrameChanged = false;
            if ((onNewFrameScreenImage != null) && (onNewFrameScreenImage.Data != null) && (reversedOnNewFrameScreenBytes != null) && (reversedOnNewFrameScreenBytes.Length > 0)) {
                Debug.Log($"{onNewFrameScreenImage.Bounds.width}/{onNewFrameScreenImage.Bounds.height} {reversedOnNewFrameScreenBytes.Length}");
                if ((onNewFrameScreenTexture == null) || 
                    (onNewFrameScreenTexture.width != onNewFrameScreenImage.Bounds.width) || 
                    (onNewFrameScreenTexture.height != onNewFrameScreenImage.Bounds.height)) {
                    onNewFrameScreenTexture = new Texture2D(onNewFrameScreenImage.Bounds.width, onNewFrameScreenImage.Bounds.height, TextureFormat.BGRA32, false);
                    int byteCount = onNewFrameScreenImage.Bounds.width * onNewFrameScreenImage.Bounds.height * 4;
                    onNewFrameScreenBytes = new byte[byteCount];
                    rawImage4.texture = onNewFrameScreenTexture;
                    rawImage4.GetComponent<AspectRatioFitter>().aspectRatio = (float)onNewFrameScreenTexture.width / onNewFrameScreenTexture.height;
                }

                // turn image around
                int stride = onNewFrameScreenImage.BytesToNextRow;
                for (var line = 0; line < onNewFrameScreenImage.Bounds.height; line++) {
                    int src = onNewFrameScreenBytes.Length - ((line + 1) * stride);
                    int target = line * stride;
                    Array.Copy(reversedOnNewFrameScreenBytes, src, onNewFrameScreenBytes, target, stride);
                }

                onNewFrameScreenTexture.LoadRawTextureData(onNewFrameScreenBytes);
                onNewFrameScreenTexture.Apply();
            } 
            else {
                Debug.Log($"onNewFrameImage is null");
            }
        }
    }

    Thread invertBufferThread;
    bool invertBufferIsStopped = false;
    void InvertBuffer () {
        while (!invertBufferIsStopped) {

        }
    }

    Screen_Capture.ScreenCaptureManager framgrabber;

    void Start() {

        invertBufferThread = new Thread(InvertBuffer);
        invertBufferThread.Start();

        Debug.Log("Starting Capture Demo/Test");
        var monitors = Screen_Capture.GetMonitors();
        foreach(var item in monitors) {
            WriteLine(item); 
        }

        var windows = Screen_Capture.GetWindows();
        foreach (var item in windows) {
            WriteLine(item);
        }

        framgrabber = createframegrabber();

        // Debug.Log("Running window capturing for 10 seconds");
        // using (var framgrabber = createwindowgrabber())
        // {
        //     System.Threading.Thread.Sleep(10 * 1000);
        // }

        // using (var framgrabber = createframegrabber())
        // {
        //     Debug.Log("Running display capturing for 1 seconds");
        //     System.Threading.Thread.Sleep(1 * 1000);
        //     Debug.Log("Pausing for 10 seconds.");
        //     framgrabber.pause();
        //     var counti = 0;
        //     while (counti++ < 10)
        //     {
        //         Debug.Assert(framgrabber.isPaused());
        //         Console.Write(" . ");
        //         System.Threading.Thread.Sleep(1 * 1000);
        //     }

        //     framgrabber.resume();
        //     Debug.Log("Testing changing the interval during runtime for race conditions ");

        //     var start = DateTime.Now;
        //     while ((DateTime.Now - start).TotalSeconds < 10)
        //     {
        //         for (var t = 0; t < 100; t++)
        //         {
        //             framgrabber.setFrameChangeInterval(100);
        //             framgrabber.setMouseChangeInterval(100);
        //         }
        //     }

        //     Debug.Log("Changing the cpature rate to 1 second");
        //     framgrabber.setFrameChangeInterval(1000);
        //     System.Threading.Thread.Sleep(1 * 1000);
        // }
    }

    void OnApplicationQuit() {
        if (framgrabber != null) {
            Debug.Log("OnApplicationQuit");
            framgrabber.stop();
            framgrabber = null;
        }
    }

   // write the texture 
    public void SaveTex (Texture2D tex, string name) {
        String dirPath = Application.dataPath + "/../" + name + ".png";
        byte[] _bytes = tex.EncodeToPNG();
        System.IO.File.WriteAllBytes(dirPath, _bytes);
        Debug.Log(_bytes.Length/1024  + "Kb was saved as: " + dirPath);
    }

    List<Texture2D> onFrameChangedDataTextures = new List<Texture2D>();
    List<Texture2D> onNewFrameDataTextures =new List<Texture2D>();

    //     // update the displayed image according to the selected item in the dropdown
    //     if (dropdown.options.Count > 0) {
    //         // when the first texture is available (count > 0 && not interactable yet)
    //         if (!dropdown.interactable) {
    //             dropdown.interactable = true;
    //             // have to change to select when it previously was empty
    //             dropdown.value = 1;
    //             dropdown.value = 0;
    //         }
    //         // apply the currently selected texture in the dropdown
    //         WindowCallbackData data = dataList[dropdown.value];
    //         Texture2D tex = textureList[dropdown.value];
    //         rawImage.texture = tex;
    //         rawImage.GetComponent<AspectRatioFitter>().aspectRatio = tex.width / tex.height;
    //         text.text = $"onFrameChangedParams pos: {data.window.Position.x}/{data.window.Position.y} size: {data.window.Size.x}/{data.window.Size.y}";
    //     }
    // }

    // void Update () {
    //     HandleDataSet (ref ScreenCaptureLightManager.onFrameChangedData, ref onFrameChangedDataTextures, ref dropdown1, ref rawImage1, ref text1);
    //     HandleDataSet (ref ScreenCaptureLightManager.onNewFrameData, ref onNewFrameDataTextures, ref dropdown2, ref rawImage2, ref text2);
    //     // mouse handling is missing
    //     // rawImage3.texture = ScreenCaptureLightManager.onMouseChangedData.texture;
    //     // text3.text = $"pos: {ScreenCaptureLightManager.onMouseChangedData.mousePoint.Position.x}/{ScreenCaptureLightManager.onMouseChangedData.mousePoint.Position.y} hotspot: {ScreenCaptureLightManager.onMouseChangedData.mousePoint.HotSpot.x}/{ScreenCaptureLightManager.onMouseChangedData.mousePoint.HotSpot.y}";
    // }

#endregion
}
