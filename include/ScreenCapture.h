#pragma once
#include <memory>
#include <functional>
#include <vector>
#include <string>
#include <ostream>

namespace SL {
	namespace Screen_Capture {

		struct Monitor;

		struct ImageRect {
			int    left = 0;
			int    top = 0;
			int    right = 0;
			int    bottom = 0;
			bool Contains(const ImageRect& a) const {
				return left <= a.left && right >= a.right && top <= a.top && bottom >= a.bottom;
			}
		
		};
		inline bool operator==(const ImageRect& a, const ImageRect& b) {
			return b.left == a.left && b.right == a.right && b.top == a.top && b.bottom == a.bottom;
		}
		inline std::ostream& operator<<(std::ostream &os, const ImageRect& p)
		{
			return os << "left=" << p.left << " top=" << p.top << " right=" << p.right << " bottom=" << p.bottom;
		}
		//the Image Struct does NOT OWN any of the data it holds. It Describes an image and provides helpful functions. 
		struct Image;
		//data is not owned by the Image Struct. It could point to an area within an exisiting image. 
		std::shared_ptr<Image> CreateImage(const ImageRect& rect, int pixelstride, int rowpadding, char* data);
		//this function will copy data from the src into the dst. The only requirement is that src must not be larger than dst, but it can be smaller
		void Copy(const Image& dst, const Image& src);

		//index to self in the GetMonitors() function
		int Index(const Monitor& mointor);
		//unique identifier
		int Id(const Monitor& mointor);
		int OffsetX(const Monitor& mointor);
		int OffsetY(const Monitor& mointor);
		const std::string& Name(const Monitor& mointor);
		int Height(const Monitor& mointor);
		int Width(const Monitor& mointor);

		int Height(const ImageRect& rect);
		int Width(const ImageRect& rect);

		int Height(const Image& img);
		int Width(const Image& img);
		const ImageRect& Rect(const Image& img);

		//number of bytes per row, NOT including the Rowpadding
		int RowStride(const Image& img);
		//number of bytes per row of padding
		int RowPadding(const Image& img);
		char* StartSrc(const Image& img);

		std::vector<std::shared_ptr<Monitor>> GetMonitors();

		typedef std::function<void(const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Monitor& monitor)> CaptureCallback;

		class ScreenCaptureManagerImpl;
		class ScreenCaptureManager {
			std::unique_ptr<ScreenCaptureManagerImpl> _ScreenCaptureManagerImpl;

		public:
			ScreenCaptureManager();
			~ScreenCaptureManager();
			//Set which monitors should be captured. This allows users to capture only a specific monitor, or all
			void Set_CaptureMonitors(const std::vector<std::shared_ptr<Monitor>>& monitorstocapture);
			//set this callback if you want to capture the entire Montitor each new frame
			//Each monitor will always have the same thread execute its callback. 
			void Set_CaptureEntireCallback(CaptureCallback img_cb);
			//set this callback if you want to capture just differences between the frames. 
			//Each monitor will always have the same thread execute its callback. 
			void Set_CaptureDifCallback(CaptureCallback img_cb);

			void StartCapturing(int min_interval);
			void StopCapturing();
		};
	
	}
}