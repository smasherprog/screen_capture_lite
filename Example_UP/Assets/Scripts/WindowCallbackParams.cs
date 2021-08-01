using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;

public class WindowCallbackParams {
    public int width;
    public int height;
    public TestScreenCaptureLite.Window window;
    public bool changed;
    public int byteCount;
    public byte[] bytes = new byte[0];
    public Texture2D texture;

    int frameCounter = 0;
    System.Diagnostics.Stopwatch stopwatch;

    List<string> debugMessages = new List<string>();
    void FpsCounter (string log) {
        frameCounter ++;
        if (stopwatch == null) {
            stopwatch = new System.Diagnostics.Stopwatch();
            stopwatch.Start();
        }
        if (stopwatch.Elapsed.Seconds > 0) {
            float fps = frameCounter / (float)stopwatch.Elapsed.TotalSeconds;
            debugMessages.Add($"{log} fps {fps}");
            stopwatch.Restart();
            frameCounter = 0;
        }
    }

    public void Changed (int w, int h, int s, IntPtr array, TestScreenCaptureLite.Window inWindow) {
        debugMessages.Add($"Changed w:{w} h:{h} s:{s}");

        width = w;
        height = h;
        byteCount = s;
        changed = true;
        window = inWindow;
        
        if (bytes.Length != byteCount) {
            bytes = new byte[s];
        }
        Marshal.Copy(array, bytes, 0, bytes.Length);

        FpsCounter("Changed");

        if ((w != window.Size.x) || (h != window.Size.y)) {
            debugMessages.Add($"OnFrameChanged {frameCounter}: img:{w}/{h} win:{window.Size.x}/{window.Size.y} differ");
        }

        if ((w * h * 4) != s) {
            debugMessages.Add($"OnFrameChanged {frameCounter}: w:{w} h:{h} *:{(w*h)} s:{s} l:{bytes.Length} : Invalid size of Data for given Size");
            bytes = null;
        }
    }

    // must be called in Unity Update (Main Thread)
    public void UpdateTexture () {
        if (debugMessages.Count > 0) {
            for (int i=0; i<debugMessages.Count; i++ ){
                Debug.Log(debugMessages[i]);
            }
            debugMessages.Clear();
        }

        if ((int)window.Handle == 0) {
            Debug.Log($"Window Handle is {window.Handle}");
            return;
        }

        if ((texture == null) || (texture.width != width) || (texture.height != height)) {
            string log = $"Update.frameChangedTex: create new Texture {window.Handle} w/h: {width}/{height} win: {window.Size.x}/{window.Size.y}";
            if (texture != null) {
                log += $"(old: {texture.width} {texture.height})";
            }
            Debug.Log(log);
            texture = new Texture2D(width, height, TextureFormat.RGBA32, false);
        }
        if (changed) {
            changed = false;
            try {
                // maybe bytes must be thread locked here?
                lock (bytes) {
                    if (bytes.Length == (texture.width * texture.height * 4)) {
                        texture.LoadRawTextureData(bytes);
                        texture.Apply();
                    }
                    else {
                        Debug.LogWarning($"Update.frameChanged.Exception: bytes available:{bytes.Length} do not equal texture size {(texture.width * texture.height * 4)} ({texture.width}*{texture.height}*4)");
                    }
                }
            }
            catch (Exception e) {
                Debug.LogError($"Update.frameChanged.Exception: bytes available:{bytes.Length} w:{width} h:{height} s:{byteCount} *:{(width*height)} for texture {texture.width}/{texture.height}\n\n{e.ToString()}");
            }
        }
    }
}
