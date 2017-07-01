#include "ScreenCapture.h"
#include <iostream>
#include <chrono>
#include <atomic>
#include <thread>
#include <string>
#include <vector>

// THESE LIBRARIES ARE HERE FOR CONVINIENCE!! They are SLOW and ONLY USED FOR HOW THE LIBRARY WORKS!
#define TJE_IMPLEMENTATION
#include "tiny_jpeg.h"
#define LODEPNG_COMPILE_PNG
#define LODEPNG_COMPILE_DISK
#include "lodepng.h"
/////////////////////////////////////////////////////////////////////////

using namespace std::chrono_literals;
SL::Screen_Capture::ScreenCaptureManager framgrabber;
std::atomic<int> realcounter;
std::atomic<int> onNewFramecounter;

auto onNewFramestart = std::chrono::high_resolution_clock::now();
void createframegrabber()
{
    realcounter = 0;
    onNewFramecounter = 0;
    framgrabber =
        SL::Screen_Capture::CreateScreeCapture([]() { return SL::Screen_Capture::GetMonitors(); })
            .onFrameChanged([&](const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Monitor& monitor) {
                std::cout << "height  " << Height(img) << "  width  " << Width(img) << std::endl;
                auto r = realcounter.fetch_add(1);
                auto s = std::to_string(r) + std::string(" D") + std::string(".jpg");
                auto size = RowStride(img) * Height(img);

                // auto imgbuffer(std::make_unique<char[]>(size));
                // you can extract the image as is in BGRA, or Convert
                // Extract(img, imgbuffer.get(), size);
                // ExtractAndConvertToRGBA(img, imgbuffer.get(), size);

                // tje_encode_to_file(s.c_str(), Width(img), Height(img), 4, (const unsigned char*)imgbuffer.get());
            })
            .onNewFrame([&](const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Monitor& monitor) {

                auto r = realcounter.fetch_add(1);
                auto s = std::to_string(r) + std::string(" E") + std::string(".jpg");

                auto size = RowStride(img) * Height(img);
                // auto imgbuffer(std::make_unique<char[]>(size));
                // Extract(img, imgbuffer.get(), size);
                // ExtractAndConvertToRGBA(img, imgbuffer.get(), size);

                if(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() -
                                                                         onNewFramestart).count() >= 1000) {
                    std::cout << "onNewFrame fps" << onNewFramecounter << std::endl;
                    onNewFramecounter = 0;
                    onNewFramestart = std::chrono::high_resolution_clock::now();
                }
                onNewFramecounter += 1;
                // tje_encode_to_file(s.c_str(), Width(img), Height(img), 4, (const unsigned char*)imgbuffer.get());
            })
            .onMouseChanged([&](const SL::Screen_Capture::Image* img, int x, int y) {

                auto r = realcounter.fetch_add(1);
                auto s = std::to_string(r) + std::string(" M") + std::string(".png");
                if(img) {
                    // std::cout << "New mouse coordinates AND NEW Image received." << " x= " << x << " y= " << y  <<
                    // std::endl;
                    // tje_encode_to_file(s.c_str(), Width(*img), Height(*img), 4, (const unsigned
                    // char*)StartSrc(*img));
                } else {
                    // std::cout << "New mouse coordinates received." << " x= " << x << " y= " << y << " The mouse image
                    // is still the same as the last" << std::endl;
                }

            })
            .start_capturing();

    framgrabber.setFrameChangeInterval(std::chrono::milliseconds(100));
    framgrabber.setMouseChangeInterval(std::chrono::milliseconds(100));
}
int main()
{
    std::cout << "Starting Capture Demo" << std::endl; 
    std::cout << "Running standard capturing "<< std::endl;
    createframegrabber();

    std::cout << "Pausing for 10 seconds. "<< std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));
    framgrabber.pause();
    auto i = 0;
    while(i++ < 10) {
        assert(framgrabber.isPaused());
        std::cout << " . ";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << std::endl << "Resuming  . . ." << std::endl;
    framgrabber.resume();
    std::this_thread::sleep_for(std::chrono::seconds(5));

    std::cout << "Testing changing the interval during runtime for race conditions ";

    // HAMMER THE SET FRAME INTERVAL FOR RACE CONDITIONS!!
    auto start = std::chrono::high_resolution_clock::now();
    while(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() <
          10) {
        for(auto t = 0; t < 100; t++) {
            framgrabber.setFrameChangeInterval(std::chrono::microseconds(100));
            framgrabber.setMouseChangeInterval(std::chrono::microseconds(100));
        }
    }
    std::cout << "Changing the cpature rate to 1 second";
    framgrabber.setFrameChangeInterval(std::chrono::seconds(1));
    std::this_thread::sleep_for(std::chrono::seconds(5));

    std::cout << "Setting timer using chrono literals";
    // You can use chron's literals as well!
    framgrabber.setFrameChangeInterval(10ms);
    std::this_thread::sleep_for(std::chrono::seconds(5));

    std::cout << "Testing recreating";
    createframegrabber();
    std::this_thread::sleep_for(std::chrono::seconds(5));

    std::cout << "Testing destroy";
    framgrabber.destroy();

    std::cout << "Testing recreating";
    createframegrabber();
    std::this_thread::sleep_for(std::chrono::seconds(5));

    return 0;
}
