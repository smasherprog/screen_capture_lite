#include "windows\ScreenCaptureDX.h"
#include <iostream> 
#include <chrono> 

int main()
{
	SL::Screen_Capture::ScreenCaptureDX dx;
	SL::Screen_Capture::ImageCallback func = [](const SL::Screen_Capture::Image& img, SL::Screen_Capture::Captured_Image type) {
		//std::cout << "Got here" << std::endl;
	};
	dx.StartProcessing(func);
	while (true) {



		std::this_thread::sleep_for(std::chrono::milliseconds(50));

	}
	dx.StopProcessing();
	int k = 0;
	std::cin >> k;
	return 0;
}