# Screen_Capture
Cross-platform screen capturing library. No extra dependencies ... this is made to be lightweight and fast.
see the Exmaple folder for a demo

Platforms supported:
<ul>
<li>Windows 7 SP1 and Up</li>
<li>MacOS: Done</li>
<li>Linux: In Progress</li>
</ul>


<h2>USAGE</h2>
```
SL::Screen_Capture::ScreenCaptureManager framgrabber;
auto monitors = SL::Screen_Capture::GetMonitors();
framgrabber.Set_CaptureMonitors(monitors);

framgrabber.Set_CaptureDifCallback(diffunc);
framgrabber.Set_CaptureEntireCallback(wholefunc);
framgrabber.StartCapturing(100);


```
