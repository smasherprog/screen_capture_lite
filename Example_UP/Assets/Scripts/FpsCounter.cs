using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class FpsCounter
{
    int frameCounter = 0;
    System.Diagnostics.Stopwatch stopwatch;
    List<string> debugMessages = new List<string>();

    public void UpdateFps (string log) {
        frameCounter ++;
        if (stopwatch == null) {
            stopwatch = new System.Diagnostics.Stopwatch();
            stopwatch.Start();
        }
        if (stopwatch.Elapsed.Seconds > 0) {
            float fps = frameCounter / (float)stopwatch.Elapsed.TotalSeconds;
            debugMessages.Add($"WindowCallbackParams.FpsCounter: {log} fps {fps}");
            stopwatch.Restart();
            frameCounter = 0;
        }
    }

    public void PrintLog () {
        if (debugMessages.Count > 0) {
            for (int i=0; i<debugMessages.Count; i++ ){
                Debug.Log(debugMessages[i]);
            }
            debugMessages.Clear();
        }
    }
}
