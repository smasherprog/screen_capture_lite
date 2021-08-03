using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;
using AOT;
using UnityEngine.UI;
using UnityEngine.Events;
using SCL;

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
    
    public Text topText;
    public Text text1;
    public Text text2;
    public Text text3;
    
    public Dropdown dropdown1;
    public Dropdown dropdown2;

    ScreenCaptureLightManager screenCaptureLightManager;

    void Start() {
        Window[] systemWindows = ScreenCaptureLightManager.GetWindows();
        Window[] captureWindows = new Window[captureWindowIds.Count];
        for (int i=0; i<systemWindows.Length; i++) {
            Debug.Log($"Window: {i} handle:{systemWindows[i].Handle} posX:{systemWindows[i].Position.x} posY:{systemWindows[i].Position.y} sizeX:{systemWindows[i].Size.x} sizeY:{systemWindows[i].Size.y} Name:'{systemWindows[i].Name}'");
            if (captureWindowIds.Contains(i)) {
                captureWindows[captureWindowIds.IndexOf(i)] = systemWindows[i];
                Debug.Log($"Window: CAPTURE {systemWindows[i].Handle}");
            }
        }

        // monitors dont work at all yet, values are wrong?
        Monitor[] monitors = ScreenCaptureLightManager.GetMonitors();
        for (int i=0; i<monitors.Length; i++) {
            Monitor monitor = monitors[i];
            string log = "";
            log += $"Monitors: {i} Id:{monitor.Id} Index:{monitor.Index} Adapter:{monitor.Adapter} \n";
            log += $"MonitorSize: {monitor.Height}/{monitor.Width} MonitorOriginalSize: {monitor.OriginalHeight}/{monitor.OriginalWidth} \n";
            log += $"Offset: {monitor.OffsetX}/{monitor.OffsetY} MonitorOriginalOffset:{monitor.OriginalOffsetX}/{monitor.OriginalOffsetY} \n";
            log += $"Name:'{monitor.Name}' Scaling:{monitor.Scaling} ";
            Debug.Log(log);
        }

        ScreenCaptureLightManager.StartCapture(captureWindows, captureNewFrame, captureOnFrameChanged, captureMouseChanged);

        topText.text = "";
        foreach (Window win in captureWindows) {
            topText.text += $"grabbing window: {win.Name} {win.Position.x}/{win.Position.y}";
        }

        ScreenCaptureLightManager.C_Capture_SetFrameChangeInterval(1000/24);
        ScreenCaptureLightManager.C_Capture_SetMouseChangeInterval(1000/24);
    }

    void OnApplicationQuit() {
        ScreenCaptureLightManager.C_Capture_Stop();
    }

    void OnDisable () {
        ScreenCaptureLightManager.C_Capture_Stop();
    }

    void OnDestroy () {
        ScreenCaptureLightManager.C_Capture_Stop();
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

    void HandleDataSet (ref List<WindowCallbackData> dataList, ref List<Texture2D> textureList, ref Dropdown dropdown, ref RawImage rawImage, ref Text text) {
        // handle newly created data sets, create dropdown entry and texture
        for (int i=0; i<dataList.Count; i++) {
            WindowCallbackData data = dataList[i];
            if (data.isNew) {
                data.isNew = false;
                Dropdown.OptionData optionData = new Dropdown.OptionData();
                optionData.text = ((int)data.window.Handle).ToString();
                dropdown.options.Add (optionData);
                textureList.Add (new Texture2D(data.width, data.height, TextureFormat.RGBA32, false));
            }
        }

        // handle changed data sets, update texture size and texture content
        for (int i=0; i<dataList.Count; i++) {
            WindowCallbackData data = dataList[i];
            if (data.changed) {
                data.changed = false;
                Texture2D tex = textureList[i];
                // lock the data bytes from being edited
                // still the same size?
                if ((tex == null) || (tex.width != data.width) || (tex.height != data.height)) {
                    string log = $"WindowCallbackParams.UpdateTexture: create new Texture {data.window.Handle} w/h: {data.width}/{data.height} win: {data.window.Size.x}/{data.window.Size.y}";
                    if (tex != null) {
                        log += $"(old: {tex.width} {tex.height})";
                    }
                    Debug.Log(log);
                    tex = new Texture2D(data.width, data.height, TextureFormat.RGBA32, false);
                }
                try {
                    // maybe bytes must be thread locked here?
                    lock (data.bytes) {     
                        if (data.bytes.Length == (tex.width * tex.height * 4)) {
                            tex.LoadRawTextureData(data.bytes);
                            tex.Apply();
                        }
                        else {
                            Debug.LogWarning($"WindowCallbackParams.UpdateTexture: bytes available:{data.bytes.Length} do not equal texture size {(tex.width * tex.height * 4)} ({tex.width}*{tex.height}*4)");
                        }
                    }
                }
                catch (Exception e) {
                    Debug.LogError($"WindowCallbackParams.UpdateTexture: bytes available:{data.bytes.Length} w:{data.width} h:{data.height} s:{data.byteCount} *:{(data.width*data.height)} for texture {tex.width}/{tex.height}\n\n{e.ToString()}");
                }
                textureList[i] = tex;
            }
        }

        // update the displayed image according to the selected item in the dropdown
        if (dropdown.options.Count > 0) {
            // when the first texture is available (count > 0 && not interactable yet)
            if (!dropdown.interactable) {
                dropdown.interactable = true;
                // have to change to select when it previously was empty
                dropdown.value = 1;
                dropdown.value = 0;
            }
            // apply the currently selected texture in the dropdown
            WindowCallbackData data = dataList[dropdown.value];
            Texture2D tex = textureList[dropdown.value];
            rawImage.texture = tex;
            rawImage.GetComponent<AspectRatioFitter>().aspectRatio = tex.width / tex.height;
            text.text = $"onFrameChangedParams pos: {data.window.Position.x}/{data.window.Position.y} size: {data.window.Size.x}/{data.window.Size.y}";
        }
    }

    void Update () {
        HandleDataSet (ref ScreenCaptureLightManager.onFrameChangedData, ref onFrameChangedDataTextures, ref dropdown1, ref rawImage1, ref text1);
        HandleDataSet (ref ScreenCaptureLightManager.onNewFrameData, ref onNewFrameDataTextures, ref dropdown2, ref rawImage2, ref text2);
        // mouse handling is missing
        // rawImage3.texture = ScreenCaptureLightManager.onMouseChangedData.texture;
        // text3.text = $"pos: {ScreenCaptureLightManager.onMouseChangedData.mousePoint.Position.x}/{ScreenCaptureLightManager.onMouseChangedData.mousePoint.Position.y} hotspot: {ScreenCaptureLightManager.onMouseChangedData.mousePoint.HotSpot.x}/{ScreenCaptureLightManager.onMouseChangedData.mousePoint.HotSpot.y}";
    }

#endregion
}
