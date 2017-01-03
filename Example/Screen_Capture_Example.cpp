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
	auto diffunc = [&](const char* data, const int pixelstride, const SL::Screen_Capture::Monitor& monitor, const SL::Screen_Capture::ImageRect& rect) {

		std::cout << "Diff ScreenIndex " << monitor.Index << " right " << rect.right << ", bottom " << rect.bottom<< std::endl;
		auto r = realcounter.fetch_add(1);
		auto s = std::to_string(r) + std::string(" D") + std::string(".jpg");
	
		/*auto imgdata = std::make_unique<char[]>(pixelstride*(rect.bottom - rect.top) * (rect.right - rect.left));
		if (!tje_encode_to_file(s.c_str(), (rect.right - rect.left), (rect.bottom - rect.top), 4, (const unsigned char*)imgdata.get())) {
			std::cout << "Could not write JPEG\n";
		}*/
	};
	auto wholefunc = [&](const char* data, const int pixelstride, const SL::Screen_Capture::Monitor& monitor) {

		std::cout << "Entire ScreenIndex " << monitor.Index << " Width " << monitor.Width << ", Height " << monitor.Height << std::endl;
		auto r = realcounter.fetch_add(1);
		auto s = std::to_string(r) + std::string(" E") + std::string(".jpg");

		//if (!tje_encode_to_file(s.c_str(), monitor.Width, monitor.Height, 4, (const unsigned char*)data)) {
		//	std::cout << "Could not write JPEG\n";
		//}

	};
	framgrabber.Set_CapturCallback(diffunc);
	framgrabber.Set_CapturCallback(wholefunc);

	framgrabber.StartCapturing(100);

	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	framgrabber.StopCapturing();
	return 0;
}

