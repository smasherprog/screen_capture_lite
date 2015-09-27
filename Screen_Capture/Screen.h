#pragma once

#include <memory>
#include <vector>

namespace SL {
	namespace Screen_Capture {
	
		struct Screen_Info {
			int Width = 0;
			int Height = 0;
			int Depth = 0;
			char Device[32];
			int Offsetx = 0;
			int Offsety = 0;
			int Index = 0;
		};
		class Screen {

			Screen_Info ScreenInfo;
			std::vector<char> Data;
		public:
			Screen() {}
			Screen(Screen_Info& m, std::vector<char>& d) : ScreenInfo(std::move(m)), Data(std::move(d)) {}

			const char* get_data() const { return Data.data(); }
			const Screen_Info get_MonitorInfo() const { return ScreenInfo; }
		};
		//getmonitors will give you information about the attached monitors, from left to right
		std::vector<SL::Screen_Capture::Screen_Info> GetMoitors();
		//GetScreens with index==-1 gets all available screens, otherwise the index wil be used to get the specific screen

		std::vector<std::shared_ptr<Screen>> GetScreens(int index = -1);

	}
};
