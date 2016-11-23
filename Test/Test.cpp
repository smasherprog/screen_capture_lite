// Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\Screen_Capture\Screen.h"
#include <iostream>
#include <chrono>

int main()
{
	auto monitors = SL::Screen_Capture::GetMoitors();
	for (const auto& a : monitors) {
		std::cout << "MonitorName: " << a.Device << std::endl;
		std::cout << "Width: " << a.Width << std::endl;
		std::cout << "Height: " << a.Height << std::endl;
		std::cout << "Depth: " << a.Depth << std::endl;
		std::cout << "Offsetx: " << a.Offsetx << std::endl;
		std::cout << "Offsety: " << a.Offsety << std::endl;
		std::cout << "Index: " << a.Index << std::endl;
	}

	auto start = std::chrono::high_resolution_clock::now();
	auto screen = SL::Screen_Capture::CaptureDesktopImage(monitors);
	std::cout << "took " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() << "ms\n";

	SL::Screen_Capture::Save(screen, "testing.bmp");

	int k;
	std::cin >> k;
	return 0;
}

