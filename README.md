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
https://github.com/smasherprog/Screen_Capture/tree/master/Example
```
#include "ScreenCapture.h"
auto diffunc = [](const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Monitor& monitor) {
	auto imgdata = std::make_unique<char[]>(RowStride(img)*Height(img));
	auto startdst = imgdata.get();
	auto startsrc = StartSrc(img);
	if (RowPadding(img) == 0) {//if there is no row padding, just do a single memcpy call instead of multple
		memcpy(startdst, startsrc, RowStride(img)*Height(img));
	}else {
		for (auto i = 0; i < Height(img); i++) {
			memcpy(startdst, startsrc, RowStride(img));
			startdst += RowStride(img);//advance to the next row
			startsrc += RowStride(img) + RowPadding(img);//advance to the next row
		}
	}
    //imgdata now contains the image
};
auto wholefunc = [](const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Monitor& monitor) {
	auto imgdata = std::make_unique<char[]>(RowStride(img)*Height(img));
	auto startdst = imgdata.get();
	auto startsrc = StartSrc(img);
	if (RowPadding(img) == 0) {//if there is no row padding, just do a single memcpy call instead of multple
		memcpy(startdst, startsrc, RowStride(img)*Height(img));
	}else {
		for (auto i = 0; i < Height(img); i++) {
			memcpy(startdst, startsrc, RowStride(img));
			startdst += RowStride(img);//advance to the next row
			startsrc += RowStride(img) + RowPadding(img);//advance to the next row
		}
	}
    //imgdata now contains the image
};
  
SL::Screen_Capture::ScreenCaptureManager framgrabber;
auto monitors = SL::Screen_Capture::GetMonitors();
framgrabber.Set_CaptureMonitors(monitors);

framgrabber.Set_CaptureDifCallback(diffunc);
framgrabber.Set_CaptureEntireCallback(wholefunc);
framgrabber.StartCapturing(100);


```
