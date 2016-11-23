#include "stdafx.h"
#include "Screen.h"
#include <algorithm>


namespace SL {
	namespace Screen_Capture {

		void Reorder(std::vector<SL::Screen_Capture::ScreenInfo>& screens) {
			//organize the monitors so that the ordering is left to right for displaying purposes
			std::sort(begin(screens), end(screens), [](const SL::Screen_Capture::ScreenInfo& i, const SL::Screen_Capture::ScreenInfo& j) { return i.Offsetx < j.Offsetx; });
			auto index = 0;
			for (auto& x : screens) x.Index = index++;
		}

	}
}


