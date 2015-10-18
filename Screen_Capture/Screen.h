#pragma once

#include <memory>
#include <vector>
#include <algorithm>
#include <mutex>
#include <memory>

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
		struct Blk;
		size_t getSize(const std::shared_ptr<Blk>& b);
		char* getData(const std::shared_ptr<Blk>&b);

 		std::shared_ptr<Blk> AquireBuffer(size_t req_bytes);
		void ReleaseBuffer(std::shared_ptr<Blk>& buffer);

		class Image {
			std::shared_ptr<Blk> _Data;
			size_t _Height;
			size_t _Width;
		public:
			Image() {}
			Image(Image&& img) : _Height(std::move(img._Height)), _Width(std::move(img._Width)), _Data(std::move(img._Data)) {}
			Image(size_t h, size_t w, std::shared_ptr<Blk>& d) : _Height(h), _Width(w), _Data(d) {}
			~Image();
			//data is always rgba 32 bit stride
			char* getData() const;
			size_t Height() const{ return _Height; }
			size_t Width() const{ return _Width; }
			

		};
		inline void Reorder(std::vector<SL::Screen_Capture::Screen_Info>& screens) {
			//organize the monitors so that the ordering is left to right for displaying purposes
			std::sort(begin(screens), end(screens), [](const SL::Screen_Capture::Screen_Info& i, const SL::Screen_Capture::Screen_Info& j) { return i.Offsetx < j.Offsetx; });
			auto index = 0;
			for (auto& x : screens) x.Index = index++;
		}
		//getmonitors will give you information about the attached monitors, from left to right
		std::vector<SL::Screen_Capture::Screen_Info> GetMoitors();
		//if capturemouse  == true, the mouse will be included in the output image.
		Image CaptureDesktop(bool capturemouse);

	}
};
