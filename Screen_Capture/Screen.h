#pragma once

#include <memory>
#include <vector>

namespace SL {
	namespace Screen_Capture {
		
		struct Screen_Info {
			int Width = 0;//width in pixels of the screen
			int Height = 0;//Height in pixels of the screen
			int Depth = 0;//Depth in pixels of the screen, i.e. 32 bit
			char Device[32];//name of the screen
			int Offsetx = 0;//distance in pixels from the MOST left screen. This can be negative because the primary monitor starts at 0, but this screen could be layed out to the left of the primary, in which case the offset is negative
			int Offsety = 0;//distance in pixels from the MOST bottom of the screen
			int Index = 0;//Index of the screen from LEFT to right of the physical monitors
		};
		class Screen {

			Screen_Info ScreenInfo;
			std::vector<char> Data;
		public:
			Screen() {}
			Screen(Screen_Info& m, std::vector<char>& d) : ScreenInfo(std::move(m)), Data(std::move(d)) {}
			//data is always 32 bit RGBA formatted
			const char* get_data() const { return Data.data(); }
			const Screen_Info get_MonitorInfo() const { return ScreenInfo; }
		};
		//getmonitors will give you information about the attached monitors, from left to right
		std::vector<SL::Screen_Capture::Screen_Info> GetMoitors();
		//GetScreens with index==-1 gets all available screens, otherwise the index will be used to get the specific screen
		//if capturemouse  == true, the mouse will be included in the output image.
		std::vector<std::shared_ptr<Screen>> GetScreens(bool capturemouse, int index = -1);

	}
};
