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
	auto func = [&](const SL::Screen_Capture::CapturedImage& img, const SL::Screen_Capture::Monitor& monitor) {
	
		std::cout << "ScreenIndex " << monitor.Index<<" Height " << img.Height << ", Width " << img.Width << ", OffsetY " << img.OffsetY << ", Offsetx " << img.Offsetx << std::endl;
		auto r = realcounter.fetch_add(1);
		std::string s;
		s += std::to_string(r) + std::string(" ") + std::to_string(img.OffsetY) + std::string(",") + std::to_string(img.Offsetx) +std::string(" ") +  std::string(".jpg");

		/*if (!tje_encode_to_file(s.c_str(), img.Width, img.Height, 4, (const unsigned char*)img.Data.get())) {
			std::cout << "Could not write JPEG\n";
		}*/

	};
	framgrabber.StartCapturing(func, 100);

	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	framgrabber.StopCapturing();
	return 0;
}

