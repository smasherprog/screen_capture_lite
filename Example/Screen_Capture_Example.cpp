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
	auto diffunc = [&](const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Monitor& monitor) {

		auto r = realcounter.fetch_add(1);
		//std::cout << " r " << r << "  Diff ScreenId" << Id(monitor) << " right " << Rect(img).right << ", bottom " << Rect(img).bottom << " top " << Rect(img).top << ", left " << Rect(img).left << std::endl;
		auto s = std::to_string(r) + std::string(" D") + std::string(".jpg");

		auto imgdata = std::make_unique<char[]>(RowStride(img)*Height(img));

		auto startdst = imgdata.get();
		auto startsrc = StartSrc(img);

		if (RowPadding(img) == 0) {//if there is no row padding, just do a single memcpy call instead of multple
			memcpy(startdst, startsrc, RowStride(img)*Height(img));
		}
		else {
			for (auto i = 0; i < Height(img); i++) {
				memcpy(startdst, startsrc, RowStride(img));
				startdst += RowStride(img);//advance to the next row
				startsrc += RowStride(img) + RowPadding(img);//advance to the next row
			}
		}


		if (!tje_encode_to_file(s.c_str(), Width(img), Height(img), 4, (const unsigned char*)imgdata.get())) {
			std::cout << "Could not write JPEG\n";
		}
	};
	auto wholefunc = [&](const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Monitor& monitor) {

		//std::cout << "Entire ScreenId " << monitor.Id << " Width " << monitor.Width << ", Height " << monitor.Height << std::endl;
		auto r = realcounter.fetch_add(1);
		auto s = std::to_string(r) + std::string(" E") + std::string(".jpg");

		auto imgdata = std::make_unique<char[]>(RowStride(img)*Height(img));

		auto startdst = imgdata.get();
		auto startsrc = StartSrc(img);

		if (RowPadding(img) == 0) {//if there is no row padding, just do a single memcpy call instead of multple
			memcpy(startdst, startsrc, RowStride(img)*Height(img));
		}
		else {
			for (auto i = 0; i < Height(img); i++) {
				memcpy(startdst, startsrc, RowStride(img));
				startdst += RowStride(img);//advance to the next row
				startsrc += RowStride(img) + RowPadding(img);//advance to the next row
			}
		}

		/*	if (!tje_encode_to_file(s.c_str(), Width(img), Height(img), 4, (const unsigned char*)imgdata.get())) {
				std::cout << "Could not write JPEG\n";
			}*/

	};

	auto monitors = SL::Screen_Capture::GetMonitors();

	//you could set the program to only capture on a single monitor as well
	decltype(monitors) firstmonitor;
	firstmonitor.push_back(monitors[0]);

	framgrabber.Set_CaptureMonitors(firstmonitor);

	framgrabber.Set_CaptureDifCallback(diffunc);
	//framgrabber.Set_CaptureEntireCallback(wholefunc);

	framgrabber.StartCapturing(100);

	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	framgrabber.StopCapturing();
	return 0;
}

