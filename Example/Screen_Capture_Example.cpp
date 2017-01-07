#include "ScreenCapture.h"
#include <iostream> 
#include <chrono> 
#include <atomic>
#include <thread>
#include <string>
#define TJE_IMPLEMENTATION
#include "tiny_jpeg.h"

int main()
{

	std::atomic<int> realcounter;
	realcounter = 0;
	SL::Screen_Capture::ScreenCaptureManager framgrabber;
	auto diffunc = [&](const char* src, const int pixelstride, const SL::Screen_Capture::Monitor& monitor, const SL::Screen_Capture::ImageRect& rect) {
		if (monitor.Id != 1) return;
	
		auto r = realcounter.fetch_add(1);
		std::cout << " r " << r << "  Diff ScreenId" << monitor.Id << " right " << rect.right << ", bottom " << rect.bottom << " top " << rect.top << ", left " << rect.left << std::endl;
		auto s = std::to_string(r) + std::string(" D") + std::string(".jpg");

		auto imgdata = std::make_unique<char[]>(pixelstride*(rect.bottom - rect.top) * (rect.right - rect.left));
	
		auto height = rect.bottom - rect.top;
		auto width = rect.right - rect.left;
		auto srcrowstride = pixelstride *monitor.Width;
		auto dstrowstride = pixelstride *width;
		auto startdst = imgdata.get();
		auto startsrc = src + (rect.left*pixelstride) + (rect.top*srcrowstride);
		//copy the data out if you need to keep it. The screencapture system owns the data in the src pointer

		for (auto i = 0; i < height; i++) {
			memcpy(startdst + (i* dstrowstride), startsrc + (i* srcrowstride) , dstrowstride);
		}

		/*	if (!tje_encode_to_file(s.c_str(), width, height, 4, (const unsigned char*)imgdata.get())) {
			std::cout << "Could not write JPEG\n";
		}*/
	};
	auto wholefunc = [&](const char* src, const int pixelstride, const SL::Screen_Capture::Monitor& monitor) {

		std::cout << "Entire ScreenId " << monitor.Id << " Width " << monitor.Width << ", Height " << monitor.Height << std::endl;
		auto r = realcounter.fetch_add(1);
		auto s = std::to_string(r) + std::string(" E") + std::string(".jpg");

		/*if (!tje_encode_to_file(s.c_str(), monitor.Width, monitor.Height, 4, (const unsigned char*)src)) {
			std::cout << "Could not write JPEG\n";
		}*/

	};

	auto monitors = SL::Screen_Capture::GetMonitors();

	framgrabber.Set_CaptureMonitors(monitors);//capture all monitors
	framgrabber.Set_CapturCallback(diffunc);
	framgrabber.Set_CapturCallback(wholefunc);

	framgrabber.StartCapturing(100);

	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	framgrabber.StopCapturing();
	return 0;
}

