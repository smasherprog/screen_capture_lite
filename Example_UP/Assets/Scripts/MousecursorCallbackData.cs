using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;

namespace SCL {
public class MousecursorCallbackData {
    // is set to true when the bytes are modified, can be changed by the user
    public bool changed;
    // can be used to check wether this instance is new (for the user, not internally used)
    public bool isNew = true;
    // size of the required texture to write the bytes into
    public int width;
    public int height;
    public SCL.MousePoint mousePoint;
    public byte[] bytes = new byte[0];
    public int byteCount;

    // public Texture2D texture;
    // FpsCounter fpsCounter = new FpsCounter();

    public void Changed (int w, int h, int s, IntPtr array, SCL.MousePoint inMousePoint) {
        width = w;
        height = h;
        byteCount = s;
        changed = true;
        mousePoint = inMousePoint;
        
        if (bytes.Length != byteCount) {
            bytes = new byte[s];
        }
        Marshal.Copy(array, bytes, 0, bytes.Length);

        // fpsCounter.UpdateFps($"Changed mousePoint");
    }

    // must be called in Unity Update (Main Thread)
    // public void UpdateTexture () {
    //     fpsCounter.PrintLog();

    //     if ((texture == null) || (texture.width != width) || (texture.height != height)) {
    //         string log = $"MousecursorCallbackParams.UpdateTexture: create new Texture w/h: {width}/{height}";
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
    //                     Debug.LogWarning($"MousecursorCallbackParams.UpdateTexture: bytes available:{bytes.Length} do not equal texture size {(texture.width * texture.height * 4)} ({texture.width}*{texture.height}*4)");
    //                 }
    //             }
    //         }
    //         catch (Exception e) {
    //             Debug.LogError($"MousecursorCallbackParams.UpdateTexture: bytes available:{bytes.Length} w:{width} h:{height} s:{byteCount} *:{(width*height)} for texture {texture.width}/{texture.height}\n\n{e.ToString()}");
    //         }
    //     }
    // }
}
}