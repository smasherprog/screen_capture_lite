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

public class BasicSample : MonoBehaviour {
    #region unity user interface
    public Text uiText;
    public RawImage uiRawImage;
    #endregion
    
    Screen_Capture.Monitor scMonitor;
    Screen_Capture.MousePoint scMousePoint;
    Screen_Capture.Image scImage;
    byte[] scBytes = new byte[0];

    Texture2D scTexture;
    bool scDataChanged = false;

    Screen_Capture.ScreenCaptureManager screenCaptureManager;

    void Start() {
        Debug.Log("Starting Capture Demo/Test");
        var monitors = Screen_Capture.GetMonitors();
        foreach(var item in monitors) {
            Debug.Log($"{monitors}");
        }

        screenCaptureManager = CreateScreenCaptureManager();
    }

    public Screen_Capture.ScreenCaptureManager CreateScreenCaptureManager() {
        Screen_Capture.CaptureConfigurationData<Screen_Capture.MonitorType> captureConfigurationData = 
            Screen_Capture.CaptureConfiguration.CreateCaptureConfiguration(() => {
                Screen_Capture.Monitor[] mons = Screen_Capture.GetMonitors();
            // return first monitor as array
            return mons.Take(1).ToArray();
        });

        // ACTIVATE ONLY ONE OF THE FOLLOWING
        captureConfigurationData.onNewFrame( ImageMonitorCallback );
        // captureConfigurationData.onFrameChanged( ImageMonitorCallback ); 
        // captureConfigurationData.onMouseChanged( ImageMousepointCallback );

        Screen_Capture.ScreenCaptureManager scm = captureConfigurationData.start_capturing();
        scm.setFrameChangeInterval(100);
        scm.setMouseChangeInterval(100);
        return scm;
    }

    public void ImageMonitorCallback ( Screen_Capture.Image img,  Screen_Capture.Monitor monitor) {
        lock(scBytes) {
            scImage = img;
            scMonitor = monitor;

            // the img.Data cannot be used in Unity
            int byteCount = img.Bounds.width * img.Bounds.height * 4;
            if (scBytes.Length != byteCount) {
                scBytes = new byte[byteCount];
            }

            // copy data from img to bytes, we must use a buffer, unity does not allow a texture to be modified by a thread
            Marshal.Copy(img.Data, scBytes, 0, scBytes.Length);

            scDataChanged = true;
        }
    }

    public void ImageMousepointCallback ( Screen_Capture.Image img, Screen_Capture.MousePoint mousePoint ) {
        lock(scBytes) {
            scImage = img;
            scMousePoint = mousePoint;

            // the img.Data cannot be used in Unity
            int byteCount = img.Bounds.width * img.Bounds.height * 4;
            if (scBytes.Length != byteCount) {
                scBytes = new byte[byteCount];
            }

            // copy data from img to bytes, we must use a buffer, unity does not allow a texture to be modified by a thread
            Marshal.Copy(img.Data, scBytes, 0, scBytes.Length);

            scDataChanged = true;
        }
    }

    void Update () {
        if (scDataChanged) {
            lock(scBytes) {
                scDataChanged = false;
                if (scMonitor != null) {
                    uiText.text = $"{scMonitor.Name} {scMonitor.Id} {scMonitor.Width}/{scMonitor.Height}";
                }
                else if (scMousePoint != null) {
                    uiText.text = $"{scMousePoint.Position.x}/{scMousePoint.Position.y}";
                } 

                if ((scTexture == null) || (scTexture.width != scImage.Bounds.width) || (scTexture.height != scImage.Bounds.height)) {
                    Debug.Log($"{scImage.Bounds.width}/{scImage.Bounds.height} {scImage.BytesToNextRow} {(scImage.Bounds.width*scImage.Bounds.height*4)}=?={scBytes.Length}");
                    scTexture = new Texture2D(scImage.Bounds.width, scImage.Bounds.height, TextureFormat.BGRA32, false);
                    uiRawImage.texture = scTexture;
                    uiRawImage.GetComponent<AspectRatioFitter>().aspectRatio = (float)scTexture.width / scTexture.height;
                }

                // this is not valid from time to time, but i dont know why
                if ((scImage.Bounds.width*scImage.Bounds.height*4)==scBytes.Length) {
                    Debug.Log($"{scImage.Bounds.width}/{scImage.Bounds.height} {(scImage.Bounds.width*scImage.Bounds.height*4)}=?={scBytes.Length}");
                    scTexture.LoadRawTextureData(scBytes);
                    scTexture.Apply();
                }
                else {
                    Debug.LogError($"invalid amout of data: {scImage.Bounds.width}/{scImage.Bounds.height} {(scImage.Bounds.width*scImage.Bounds.height*4)}=?={scBytes.Length}");
                }
            }
        }
    }

    void OnDestroy() {
        if (screenCaptureManager != null) {
            screenCaptureManager.Dispose();
            screenCaptureManager = null;
        }
    }
}
