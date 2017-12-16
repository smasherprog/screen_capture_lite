#include "ScreenCapture.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <locale>
#include <string>
#include <thread>
#include <vector>

// THESE LIBRARIES ARE HERE FOR CONVINIENCE!! They are SLOW and ONLY USED FOR
// HOW THE LIBRARY WORKS!
#define TJE_IMPLEMENTATION
#include "tiny_jpeg.h"
#define LODEPNG_COMPILE_PNG
#define LODEPNG_COMPILE_DISK
#include "lodepng.h"
/////////////////////////////////////////////////////////////////////////

using namespace std::chrono_literals;
std::shared_ptr<SL::Screen_Capture::IScreenCaptureManager> framgrabber;
std::atomic<int> realcounter;
std::atomic<int> onNewFramecounter;

inline std::ostream &operator<<(std::ostream &os, const SL::Screen_Capture::ImageRect &p)
{
    return os << "left=" << p.left << " top=" << p.top << " right=" << p.right << " bottom=" << p.bottom;
}
inline std::ostream &operator<<(std::ostream &os, const SL::Screen_Capture::Monitor &p)
{
    return os << "Id=" << p.Id << " Index=" << p.Index << " Height=" << p.Height << " Width=" << p.Width << " OffsetX=" << p.OffsetX
              << " OffsetY=" << p.OffsetY << " Name=" << p.Name;
}

auto onNewFramestart = std::chrono::high_resolution_clock::now();
void createframegrabber()
{
    realcounter = 0;
    onNewFramecounter = 0;
    framgrabber =
        SL::Screen_Capture::CreateCaptureConfiguration([]() {
            auto mons = SL::Screen_Capture::GetMonitors();
            std::cout << "Library is requesting the list of monitors to capture!" << std::endl;
            for (auto &m : mons) {
                // capture just a 512x512 square...  USERS SHOULD MAKE SURE bounds are
                // valid!!!!
                
                            m.OffsetX += 512;
                            m.OffsetY += 512;
                            m.Height = 512;
                            m.Width = 512;
                
                std::cout << m << std::endl;
            }
            return mons;
        })
            ->onNewFrame([&](const SL::Screen_Capture::Image &img, const SL::Screen_Capture::Monitor &monitor) {

                auto r = realcounter.fetch_add(1);
                auto s = std::to_string(r) + std::string("MONITORNEW_") + std::string(".jpg");

                auto size = RowStride(img) * Height(img);
                // auto imgbuffer(std::make_unique<unsigned char[]>(size));
                // ExtractAndConvertToRGBA(img, imgbuffer.get(), size);
                // tje_encode_to_file(s.c_str(), Width(img), Height(img), 4, (const unsigned char*)imgbuffer.get());
                if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - onNewFramestart).count() >=
                    1000) {
                    std::cout << "onNewFrame fps" << onNewFramecounter << std::endl;
                    onNewFramecounter = 0;
                    onNewFramestart = std::chrono::high_resolution_clock::now();
                }
                onNewFramecounter += 1;
            }) 
            ->start_capturing();

    framgrabber->setFrameChangeInterval(std::chrono::milliseconds(0));
    framgrabber->setMouseChangeInterval(std::chrono::milliseconds(0));
}

void createwindowgrabber()
{
    realcounter = 0;
    onNewFramecounter = 0;
    framgrabber =
        SL::Screen_Capture::CreateCaptureConfiguration([]() {

            auto windows = SL::Screen_Capture::GetWindows();
            std::string srchterm = "cmake 3.8";
            // convert to lower case for easier comparisons
            std::transform(srchterm.begin(), srchterm.end(), srchterm.begin(), [](char c) { return std::tolower(c, std::locale()); });
            decltype(windows) filtereditems;
            for (auto &a : windows) {
                std::string name = a.Name;
                std::transform(name.begin(), name.end(), name.begin(), [](char c) { return std::tolower(c, std::locale()); });
                if (name.find(srchterm) != std::string::npos) {
                    filtereditems.push_back(a);
                    std::cout << "ADDING WINDOW  Height " << a.Size.y << "  Width  " << a.Size.x << "   " << a.Name << std::endl;
                }
            }
            return filtereditems;
        })

            ->onFrameChanged([&](const SL::Screen_Capture::Image &img, const SL::Screen_Capture::Window &window) {
                // std::cout << "Difference detected!  " << img.Bounds << std::endl;
                auto r = realcounter.fetch_add(1);
                auto s = std::to_string(r) + std::string("WINDIF_") + std::string(".jpg");
                auto size = RowStride(img) * Height(img);

                /* auto imgbuffer(std::make_unique<unsigned char[]>(size));
                 ExtractAndConvertToRGBA(img, imgbuffer.get(), size);
                 tje_encode_to_file(s.c_str(), Width(img), Height(img), 4, (const unsigned char*)imgbuffer.get());
                 */
            })
            ->onNewFrame([&](const SL::Screen_Capture::Image &img, const SL::Screen_Capture::Window &window) {

                auto r = realcounter.fetch_add(1);
                auto s = std::to_string(r) + std::string("WINNEW_") + std::string(".jpg");
                auto size = RowStride(img) * Height(img);
                /*
                               auto imgbuffer(std::make_unique<unsigned char[]>(size));
                                ExtractAndConvertToRGBA(img, imgbuffer.get(), size);
                                tje_encode_to_file(s.c_str(), Width(img), Height(img), 4, (const unsigned char*)imgbuffer.get());
                                */
                if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - onNewFramestart).count() >=
                    1000) {
                    std::cout << "onNewFrame fps" << onNewFramecounter << std::endl;
                    onNewFramecounter = 0;
                    onNewFramestart = std::chrono::high_resolution_clock::now();
                }
                onNewFramecounter += 1;

            })
            ->onMouseChanged([&](const SL::Screen_Capture::Image *img, const SL::Screen_Capture::Point &point) {

                auto r = realcounter.fetch_add(1);
                auto s = std::to_string(r) + std::string(" M") + std::string(".png");
                if (img) {
                    // std::cout << "New mouse coordinates  AND NEW Image received." << " x= " << point.x << " y= " <<
                    // point.y << std::endl;
                    // lodepng::encode(s,StartSrc(*img), Width(*img), Height(*img));
                }
                else {
                    // std::cout << "New mouse coordinates received." << " x= " << point.x << " y= " << point.y << " The
                    // mouse image is still the same
                    // as the last" << std::endl;
                }

            })
            ->start_capturing();

    framgrabber->setFrameChangeInterval(std::chrono::milliseconds(100));
    framgrabber->setMouseChangeInterval(std::chrono::milliseconds(100));
}
int main()
{
    std::cout << "Starting Capture Demo/Test" << std::endl;
    std::cout << "Testing captured monitor bounds check" << std::endl;
    
    std::cout << "Testing recreating" << std::endl;
    createframegrabber();
    std::this_thread::sleep_for(std::chrono::seconds(5));

    std::cout << "Testing destroy" << std::endl;
    framgrabber = nullptr;

    std::cout << "Testing recreating" << std::endl;
    createframegrabber();
    std::this_thread::sleep_for(std::chrono::seconds(5));

    return 0;
}
