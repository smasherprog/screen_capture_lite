#include "ScreenCapture.h"
#include <iostream> 
#include <chrono> 
#include <atomic>
#include <thread>
#include <string>
#include <vector>
#define TJE_IMPLEMENTATION
#include "tiny_jpeg.h"

int main()
{

	std::atomic<int> realcounter = 0;
	SL::Screen_Capture::ScreenCaptureManager framgrabber;
	auto monitors = SL::Screen_Capture::GetMonitors();
	std::vector<std::unique_ptr<char[]>> images;//this is the actual backing of the image
	std::vector<std::shared_ptr<SL::Screen_Capture::Image>> imagewrapper;//this is a wrapper for convience. This is not required, I use it because it helps with copying data

	//you could set the program to only capture on a single monitor as well

	decltype(monitors) firstmonitor;
	firstmonitor.push_back(monitors[0]);
	framgrabber.Set_CaptureMonitors(monitors);

	images.resize(monitors.size());
	imagewrapper.resize(monitors.size());

	auto diffunc = [&](const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Monitor& monitor) {

		auto r = realcounter.fetch_add(1);
		auto s = std::to_string(r) + std::string(" D") + std::string(".jpg");

		if (!images[Index(monitor)]) {
			//make sure to create a buffer first time through
			//the first time through the system will send the entire image, so this is the full size of the image. Next difs will be within the bounds of the first
			images[Index(monitor)] = std::make_unique<char[]>(RowStride(img)*Height(img));
			imagewrapper[Index(monitor)] = SL::Screen_Capture::CreateImage(Rect(img), 4, 0, images[Index(monitor)].get());
		}
		//copy the data in 
		Copy(*imagewrapper[Index(monitor)], img);

		/*if (!tje_encode_to_file(s.c_str(), Width(*imagewrapper[Index(monitor)]), Height(*imagewrapper[Index(monitor)]), 4, (const unsigned char*)images[Index(monitor)].get())) {
			std::cout << "Could not write JPEG\n";
		}*/
	};
	auto wholefunc = [&](const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Monitor& monitor) {

		auto r = realcounter.fetch_add(1);
		auto s = std::to_string(r) + std::string(" E") + std::string(".jpg");

		if (!images[Index(monitor)]) {
			//make sure to create a buffer first time through
			images[Index(monitor)] = std::make_unique<char[]>(RowStride(img)*Height(img));
			imagewrapper[Index(monitor)] = SL::Screen_Capture::CreateImage(Rect(img), 4, 0, images[Index(monitor)].get());
		}
		Copy(*imagewrapper[Index(monitor)], img);

		//if (!tje_encode_to_file(s.c_str(), Width(*imagewrapper[Index(monitor)]), Height(*imagewrapper[Index(monitor)]), 4, (const unsigned char*)images[Index(monitor)].get())) {
		//	std::cout << "Could not write JPEG\n";
		//}

	};


	framgrabber.Set_CaptureDifCallback(diffunc);
	//framgrabber.Set_CaptureEntireCallback(wholefunc);

	framgrabber.StartCapturing(100);

	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	framgrabber.StopCapturing();
	return 0;
}

