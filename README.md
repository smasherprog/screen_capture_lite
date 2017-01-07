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

SL::Screen_Capture::ScreenCaptureManager framgrabber;<br/>
auto monitors = SL::Screen_Capture::GetMonitors();<br/>
framgrabber.Set_CaptureMonitors(monitors);<br/>

framgrabber.Set_CaptureDifCallback(diffunc);<br/>
framgrabber.Set_CaptureEntireCallback(wholefunc);<br/>
framgrabber.StartCapturing(100);<br/>
