#include "windows\ScreenCaptureDX.h"
#include <iostream> 
#include <chrono> 
#include <atomic>

#include <string>
#define TJE_IMPLEMENTATION
#include "tiny_jpeg.h"

int main()
{

	std::atomic<int> realcounter;
	realcounter = 0;
	SL::Screen_Capture::ScreenCaptureDX dx;
	SL::Screen_Capture::ImageCallback func = [&](const SL::Screen_Capture::CapturedImage& img) {
		if (img.ScreenIndex != 1) return;
		std::cout << "Height " << img.Height << ", Width " << img.Width << ", Top " << img.AbsoluteTop << ", Left " << img.AbsoluteLeft << std::endl;
		auto r = realcounter.fetch_add(1);
		std::string s("screen");
		s += std::to_string(img.AbsoluteTop) + std::string(",") + std::to_string(img.AbsoluteLeft) +std::string(" ") + std::to_string(r) + std::string(".jpg");
		//savetodisk(img, s);

		if (!tje_encode_to_file(s.c_str(), img.Width, img.Height, 4, (const unsigned char*)img.Data.get())) {
			std::cout << "Could not write JPEG\n";
		}
	};
	dx.StartProcessing(func);

	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	dx.StopProcessing();
	int k = 0;
	std::cin >> k;
	return 0;
}

