// Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\Screen_Capture\Screen.h"
#include <iostream>
#include <chrono>

int main()
{
	auto monitorinfo = SL::Screen_Capture::GetMoitors();
	for (auto& a : monitorinfo) {
		std::cout << "MonitorName: " << a.Device << std::endl;
		std::cout << "Width: " << a.Width << std::endl;
		std::cout << "Height: " << a.Height << std::endl;
		std::cout << "Depth: " << a.Depth << std::endl;
		std::cout << "Offsetx: " << a.Offsetx << std::endl;
		std::cout << "Offsety: " << a.Offsety << std::endl;
		std::cout << "Index: " << a.Index << std::endl;
	}

	auto start = std::chrono::high_resolution_clock::now();
	auto screens = SL::Screen_Capture::GetScreens(true);//pass -1 to get all, which is the default 
	std::cout << "took " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() << "ms\n";

	start = std::chrono::high_resolution_clock::now();
	screens = SL::Screen_Capture::GetScreens(false, -1 );//do not include the mouse
	std::cout << "took " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() << "ms\n";
	for (auto& a : screens) {
		//Do something with the raw data here.. 
		//a->get_data();
	}



	int k;
	std::cin >> k;
	return 0;
}

