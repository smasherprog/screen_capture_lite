#include "Screen.h"
#include <assert.h>
#include <algorithm>
#include <fstream>
#include <thread>
#include <mutex>
#include <deque>
#include <condition_variable>


#if _WIN32

#elif defined(__linux__) && !defined(__ANDROID__)

#elif __APPLE__

#endif
namespace SL {
	namespace Screen_Capture {

		class Screen_Capture_ManagerImpl {
		public:

			std::thread thread;
			std::mutex m;
			std::condition_variable cv;
			bool should_exit = false;
			std::deque<std::function<void()>> tasks;

			std::function<void(const std::shared_ptr<const Image>&img)> img_cb;

			Screen_Capture_ManagerImpl() {
				should_exit = false;
				thread = std::thread(&SL::Screen_Capture::Screen_Capture_ManagerImpl::run, this);
			}
			~Screen_Capture_ManagerImpl() {

			}
			void run() {

				while (!should_exit) {
					std::unique_lock<std::mutex> lk(m);
					cv.wait(lk, [] {return ready; });



					lk.unlock();
					cv.notify_one();
				}

			}
		};

		Screen_Capture_Manager::Screen_Capture_Manager()
		{
			_Screen_Capture_ManagerImpl = std::make_unique<Screen_Capture_ManagerImpl>();
		}

		Screen_Capture_Manager::~Screen_Capture_Manager()
		{
			if (_Screen_Capture_ManagerImpl->thread.joinable()) {
				_Screen_Capture_ManagerImpl->thread.join();
			}
		}
		void Screen_Capture_Manager::CaptureDesktopImage(std::function<void(const std::shared_ptr<const Image>&img)> img_cb)
		{
			_Screen_Capture_ManagerImpl->add(img_cb);
		}
	


	}
}



#if _WIN32

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <Windows.h>

//RAII Objects to ensure proper destruction
#define RAIIHDC(handle) std::unique_ptr<std::remove_pointer<HDC>::type, decltype(&::DeleteDC)>(handle, &::DeleteDC)
#define RAIIHBITMAP(handle) std::unique_ptr<std::remove_pointer<HBITMAP>::type, decltype(&::DeleteObject)>(handle, &::DeleteObject)
#define RAIIHANDLE(handle) std::unique_ptr<std::remove_pointer<HANDLE>::type, decltype(&::CloseHandle)>(handle, &::CloseHandle)

void SL::Screen_Capture::Save(const Image& img, std::string path)
{
	assert(path.find(".bmp") != path.npos);
	//must be bmp.. save is for testing that the file is correct
	BITMAPINFOHEADER bi;
	memset(&bi, 0, sizeof(bi));

	bi.biSize = sizeof(BITMAPINFOHEADER);

	bi.biWidth = img.Width;
	bi.biHeight = -img.Height;
	bi.biPlanes = 1;
	bi.biBitCount = get_pixelstride() * 8; //always 32 bits damnit!!!
	bi.biCompression = BI_RGB;
	bi.biSizeImage = ((img.Width * bi.biBitCount + 31) / (get_pixelstride() * 8)) * get_pixelstride() * img.Height;

	BITMAPFILEHEADER bmfHeader;
	memset(&bmfHeader, 0, sizeof(bmfHeader));
	// A file is created, this is where we will save the screen capture.
	std::ofstream hFile(path, std::ios::trunc | std::ios::binary);
	assert(hFile);

	//Offset to where the actual bitmap bits start.
	bmfHeader.bfOffBits = (DWORD)sizeof(bmfHeader) + (DWORD)sizeof(bi);

	//Size of the file
	bmfHeader.bfSize = bi.biSizeImage + bmfHeader.bfOffBits;

	//bfType must always be BM for Bitmaps
	bmfHeader.bfType = 0x4D42; //BM

	hFile.write((char*)&bmfHeader, sizeof(bmfHeader));
	hFile.write((char*)&bi, sizeof(bi));
	hFile.write(img.Data.get(), bi.biSizeImage);
}

SL::Screen_Capture::Image SL::Screen_Capture::CaptureDesktopImage(const std::vector<ScreenInfo>& screens)
{
	auto start = std::chrono::high_resolution_clock::now();

	Image ret;
	int left(0), top(0);
	for (const auto& mon : screens) {
		ret.Width += mon.Width;
		ret.Height = std::max(ret.Height, mon.Height);
		left = std::min(left, mon.Offsetx);
		top = std::min(top, mon.Offsety);
	}
	assert(ret.Width >= 0);
	assert(ret.Height >= 0);

	static auto desktopdc(RAIIHDC(CreateDCA("DISPLAY", NULL, NULL, NULL)));
	static auto capturedc(RAIIHDC(CreateCompatibleDC(desktopdc.get())));
	static auto capturebmp(RAIIHBITMAP(CreateCompatibleBitmap(desktopdc.get(), ret.Width, ret.Height)));

	if (!desktopdc || !capturedc || !capturebmp)
		return ret;

	// Selecting an object into the specified DC
	auto originalBmp = SelectObject(capturedc.get(), capturebmp.get());

	ret.Data = std::shared_ptr<char>(new char[get_imagesize(ret)], [](char* p) { delete[] p; }); //always

	if (BitBlt(capturedc.get(), 0, 0, ret.Width, ret.Height, desktopdc.get(), left, top, SRCCOPY | CAPTUREBLT) == FALSE) {
		//if the screen cannot be captured, set everything to 1 and return
		memset(ret.Data.get(), 1, get_imagesize(ret));
		SelectObject(capturedc.get(), originalBmp);
		return ret;
	}
	BITMAPINFOHEADER bi;
	memset(&bi, 0, sizeof(bi));

	bi.biSize = sizeof(BITMAPINFOHEADER);

	bi.biWidth = ret.Width;
	bi.biHeight = -ret.Height;
	bi.biPlanes = 1;
	bi.biBitCount = get_pixelstride() * 8; //always 32 bits damnit!!!
	bi.biCompression = BI_RGB;
	bi.biSizeImage = ((ret.Width * bi.biBitCount + 31) / (get_pixelstride() * 8)) * get_pixelstride() * ret.Height;

	GetDIBits(desktopdc.get(), capturebmp.get(), 0, (UINT)ret.Height, ret.Data.get(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);

	SelectObject(capturedc.get(), originalBmp);

	std::cout << "took " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() << "ms\n";
	return ret;
}

std::vector<SL::Screen_Capture::ScreenInfo> SL::Screen_Capture::GetMoitors()
{
	std::vector<SL::Screen_Capture::ScreenInfo> ret;
	DISPLAY_DEVICEA dd;
	ZeroMemory(&dd, sizeof(dd));
	dd.cb = sizeof(dd);
	for (auto i = 0; EnumDisplayDevicesA(NULL, i, &dd, 0); i++) {
		//monitor must be attached to desktop and not a mirroring device
		if ((dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) & !(dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER)) {
			SL::Screen_Capture::ScreenInfo temp;
			DEVMODEA devMode;
			devMode.dmSize = sizeof(devMode);
			EnumDisplaySettingsA(dd.DeviceName, ENUM_CURRENT_SETTINGS, &devMode);
			strncpy_s(temp.Device, dd.DeviceName, ARRAYSIZE(dd.DeviceName));
			temp.Offsetx = devMode.dmPosition.x;
			temp.Offsety = devMode.dmPosition.y;
			temp.Width = devMode.dmPelsWidth;
			temp.Height = devMode.dmPelsHeight;
			temp.Depth = devMode.dmBitsPerPel;
			ret.push_back(temp);
		}
	}
	//always reorder the screens left to right... as it should be, right?
	Reorder(ret);
	return ret;
}
#elif __ANDROID__

#elif __linux__

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <sys/shm.h>
#include <X11/extensions/XTest.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/XShm.h>

typedef struct
{
	short x_org;
	short y_org;
	short width;
	short height;
} FLScreenInfo;
static FLScreenInfo screens[16];
static float dpi[16][2];

void SL::Screen_Capture::Save(const Image& img, std::string path)
{
}

std::vector<SL::Screen_Capture::ScreenInfo> SL::Screen_Capture::GetMoitors()
{
	if (!fl_display)
		fl_open_display();
	num_screens = ScreenCount(fl_display);
	if (num_screens > MAX_SCREENS)
		num_screens = MAX_SCREENS;

	for (int i = 0; i < num_screens; i++) {
		screens[i].x_org = 0;
		screens[i].y_org = 0;
		screens[i].width = DisplayWidth(fl_display, i);
		screens[i].height = DisplayHeight(fl_display, i);

		int mm = DisplayWidthMM(fl_display, i);
		dpi[i][0] = mm ? DisplayWidth(fl_display, i) * 25.4f / mm : 0.0f;
		mm = DisplayHeightMM(fl_display, i);
		dpi[i][1] = mm ? DisplayHeight(fl_display, i) * 25.4f / mm : 0.0f;
	}
}

std::shared_ptr<Image> SL::Screen_Capture::CaptureDesktopImage()
{
	auto display = XOpenDisplay(NULL);
	auto root = DefaultRootWindow(display);
	auto screen = XDefaultScreen(display);
	auto visual = DefaultVisual(display, screen);
	auto depth = DefaultDepth(display, screen);

	XWindowAttributes gwa;
	XGetWindowAttributes(display, root, &gwa);
	auto width = gwa.width;
	auto height = gwa.height;

	XShmSegmentInfo shminfo;
	auto image = XShmCreateImage(display, visual, depth, ZPixmap, NULL, &shminfo, width, height);
	shminfo.shmid = shmget(IPC_PRIVATE, image->bytes_per_line * image->height, IPC_CREAT | 0777);

	shminfo.readOnly = False;
	shminfo.shmaddr = image->data = (char*)shmat(shminfo.shmid, 0, 0);

	XShmAttach(display, &shminfo);

	XShmGetImage(display, root, image, 0, 0, AllPlanes);

	XShmDetach(display, &shminfo);

	auto px = Image::CreateImage(height, width, (char*)shminfo.shmaddr, image->bits_per_pixel / 8);
	assert(image->bits_per_pixel == 32); //this should always be true... Ill write a case where it isnt, but for now it should be

	XDestroyImage(image);
	shmdt(shminfo.shmaddr);
	shmctl(shminfo.shmid, IPC_RMID, 0);
	XCloseDisplay(display);

	return px;
}

#elif __APPLE__
#include "TargetConditionals.h"
#if TARGET_IPHONE_SIMULATOR
// iOS Simulator
#elif TARGET_OS_IPHONE
// iOS device
#elif TARGET_OS_MAC
// Other kinds of Mac OS
#else
#error "Unknown Apple platform"
#endif

#include <ApplicationServices/ApplicationServices.h>

std::shared_ptr<Image> SL::Screen_Capture::CaptureDesktopImage()
{
	auto image_ref = CGDisplayCreateImage(CGMainDisplayID());
	auto provider = CGImageGetDataProvider(image_ref);
	auto dataref = CGDataProviderCopyData(provider);
	size_t w, h;
	w = CGImageGetWidth(image_ref);
	h = CGImageGetHeight(image_ref);
	size_t bpp = CGImageGetBitsPerPixel(image_ref) / 8;

	auto img = Image::CreateImage(h, w, (const char*)(CFDataGetBytePtr(dataref)), bpp);

	CFRelease(dataref);
	CFRelease(provider);
	CGImageRelease(image_ref);
	return img;
}
#endif
