# screen capture lite
<p>Linux/Mac <img src="https://travis-ci.org/smasherprog/screen_capture_lite.svg?branch=master" /></p>
<p>Windows <img src="https://ci.appveyor.com/api/projects/status/6nlqo1csbkgdxorx"/><p>
<h3>Project setup: https://github.com/smasherprog/Projects_Setup</h3>
<p>Cross-platform screen capturing library . . . this is made to be lightweight and fast.
see the Exmaple folder for a demo</p>
<h2>No External Dependencies</h2>
<h4>Platforms supported:</h4>

<ul>
<li>Windows XP and Up</li>
<li>MacOS</li>
<li>Linux</li>
</ul>

<h2>USAGE</h2>
<p>The image format is raw BGRA 32 bits per pixel. Alpha is unused! <p>
https://github.com/smasherprog/screen_capture_lite/blob/master/Example/Screen_Capture_Example.cpp


```c++

auto framgrabber = SL::Screen_Capture::CreateScreeCapture([]() {
    return SL::Screen_Capture::GetMonitors();
  }).onFrameChanged([&](const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Monitor& monitor) {
  
  }).onNewFrame([&](const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Monitor& monitor) {
  
  }).onMouseChanged([&](const SL::Screen_Capture::Image* img, int x, int y) {
  
  }).start_capturing();

framgrabber.setFrameChangeInterval(std::chrono::milliseconds(100));//100 ms
framgrabber.setMouseChangeInterval(std::chrono::milliseconds(100));//100 ms

```
