// Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\Screen_Capture\Screen.h"
#include <iostream>

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
	auto screens = SL::Screen_Capture::GetScreens();//pass -1 to get all, which is the default
	for (auto& a : screens) {
		//Do something with the raw data here.. 
		//a->get_data();
	}
	int k;
	std::cin >> k;
    return 0;
}

