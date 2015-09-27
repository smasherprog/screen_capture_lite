// Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\Screen_Capture\VirtualScreen.h"
#include <iostream>

int main()
{
	auto screens = SL::Screen_Capture::Build();
	screens = SL::Screen_Capture::Build();
	screens = SL::Screen_Capture::Build();
	int k;
	std::cin >> k;
    return 0;
}

