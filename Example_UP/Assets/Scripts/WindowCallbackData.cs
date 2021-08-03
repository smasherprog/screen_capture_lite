using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;

namespace SCL {
public class WindowCallbackData {
    // is set to true when the bytes are modified, can be changed by the user
    public bool changed;
    // can be used to check wether this instance is new (for the user, not internally used)
    public bool isNew = true;
    // size of the required texture to write the bytes into
    public int width;
    public int height;
    public SCL.Window window;
    public byte[] bytes = new byte[0];
    public int byteCount;

    // FpsCounter fpsCounter = new FpsCounter();

    public void Changed (int w, int h, int s, IntPtr array, SCL.Window inWindow) {
        width = w;
        height = h;
        byteCount = s;
        changed = true;
        window = inWindow;
        
        if (bytes.Length != byteCount) {
            bytes = new byte[s];
        }
        Marshal.Copy(array, bytes, 0, bytes.Length);

        // fpsCounter.UpdateFps($"Changed {(int)window.Handle}");
    }

    // // must be called in Unity Update (Main Thread)
    // public void UpdateTexture () {
    //     fpsCounter.PrintLog();

    //     if ((int)window.Handle == 0) {
    //         Debug.Log($"WindowCallbackParams.UpdateTexture: Window Handle is {window.Handle}");
    //         return;
    //     }

    //     if ((texture == null) || (texture.width != width) || (texture.height != height)) {
    //         string log = $"WindowCallbackParams.UpdateTexture: create new Texture {window.Handle} w/h: {width}/{height} win: {window.Size.x}/{window.Size.y}";
    //         if (texture != null) {
    //             log += $"(old: {texture.width} {texture.height})";
    //         }
    //         Debug.Log(log);
    //         texture = new Texture2D(width, height, TextureFormat.RGBA32, false);
    //     }
    //     if (changed) {
    //         changed = false;
    //         try {
    //             // maybe bytes must be thread locked here?
    //             lock (bytes) {
    //                 if (bytes.Length == (texture.width * texture.height * 4)) {
    //                     texture.LoadRawTextureData(bytes);
    //                     texture.Apply();
    //                 }
    //                 else {
    //                     Debug.LogWarning($"WindowCallbackParams.UpdateTexture: bytes available:{bytes.Length} do not equal texture size {(texture.width * texture.height * 4)} ({texture.width}*{texture.height}*4)");
    //                 }
    //             }
    //         }
    //         catch (Exception e) {
    //             Debug.LogError($"WindowCallbackParams.UpdateTexture: bytes available:{bytes.Length} w:{width} h:{height} s:{byteCount} *:{(width*height)} for texture {texture.width}/{texture.height}\n\n{e.ToString()}");
    //         }
    //     }
    // }
}
}