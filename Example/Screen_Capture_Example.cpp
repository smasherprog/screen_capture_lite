#include "ScreenCapture.h"
#include <atomic>
#include <chrono>
#include <iostream>
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

auto onNewFramestart = std::chrono::high_resolution_clock::now();
void createframegrabber()
{
    realcounter = 0;
    onNewFramecounter = 0;
    framgrabber = SL::Screen_Capture::CreateCaptureConfiguration([]() {
        auto mons = SL::Screen_Capture::GetMonitors();
        std::cout << "Library is requesting the list of monitors to capture!"
            << std::endl;
        for (auto& m : mons) {
            // capture just a 512x512 square...  USERS SHOULD MAKE SURE bounds are
            // valid!!!!
/*
            m.OffsetX += 512;
            m.OffsetY += 512;
            m.Height = 512;
            m.Width = 512;
*/
            std::cout << m << std::endl;
        }
        return mons;
    })->onFrameChanged([&](const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Monitor& monitor) {
        // std::cout << "Difference detected!  " << img.Bounds << std::endl;
        auto r = realcounter.fetch_add(1);
        auto s = std::to_string(r) + std::string("MONITORDIF_") + std::string(".jpg");
        auto size = RowStride(img) * Height(img);

        //auto imgbuffer(std::make_unique<unsigned char[]>(size));
        //ExtractAndConvertToRGBA(img, imgbuffer.get(), size);
        //tje_encode_to_file(s.c_str(), Width(img), Height(img), 4, (const unsigned char*)imgbuffer.get());
    })->onNewFrame([&](const SL::Screen_Capture::Image& img,
        const SL::Screen_Capture::Monitor& monitor) {

        auto r = realcounter.fetch_add(1);
        auto s = std::to_string(r) + std::string("MONITORNEW_") + std::string(".jpg");

        auto size = RowStride(img) * Height(img);
        //auto imgbuffer(std::make_unique<unsigned char[]>(size));
        //ExtractAndConvertToRGBA(img, imgbuffer.get(), size);
        //tje_encode_to_file(s.c_str(), Width(img), Height(img), 4, (const unsigned char*)imgbuffer.get());
        if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - onNewFramestart).count() >= 1000) {
            std::cout << "onNewFrame fps" << onNewFramecounter << std::endl;
            onNewFramecounter = 0;
            onNewFramestart = std::chrono::high_resolution_clock::now();
        }
        onNewFramecounter += 1;
    })->onMouseChanged([&](const SL::Screen_Capture::Image* img, const SL::Screen_Capture::Point& point) {

        auto r = realcounter.fetch_add(1);
        auto s = std::to_string(r) + std::string(" M") + std::string(".png");
        if (img) {
            //std::cout << "New mouse coordinates  AND NEW Image received." << " x= " << point.x << " y= " << point.y << std::endl;
            //lodepng::encode(s,StartSrc(*img), Width(*img), Height(*img));
        }
        else {
            //std::cout << "New mouse coordinates received." << " x= " << point.x << " y= " << point.y << " The mouse image is still the same as the last" << std::endl;
        }

    })->start_capturing();

    framgrabber->setFrameChangeInterval(std::chrono::milliseconds(100));
    framgrabber->setMouseChangeInterval(std::chrono::milliseconds(100));
}

void createwindowgrabber()
{

    realcounter = 0;
    onNewFramecounter = 0;
    framgrabber = SL::Screen_Capture::CreateCaptureConfiguration([]() {
        auto tmp = SL::Screen_Capture::GetWindows("cmake 3.8", SL::Screen_Capture::WindowStringMatch::CONTAINS);
        //the get windows function might return multiple results so its best to make sure it is the right window.
        decltype(tmp) filtereditems;
        for (auto& a : tmp) {
            filtereditems.push_back(a);
            std::cout << "ADDING WINDOW  Height " << a.Size.y << "  Width  " << a.Size.x << "   " << a.Name << std::endl;
        }

        return filtereditems;
    })->onFrameChanged([&](const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Window& window) {
        // std::cout << "Difference detected!  " << img.Bounds << std::endl;
        auto r = realcounter.fetch_add(1);
        auto s = std::to_string(r) + std::string("WINDIF_") + std::string(".jpg");
        auto size = RowStride(img) * Height(img);

       /* auto imgbuffer(std::make_unique<unsigned char[]>(size));
        ExtractAndConvertToRGBA(img, imgbuffer.get(), size);
        tje_encode_to_file(s.c_str(), Width(img), Height(img), 4, (const unsigned char*)imgbuffer.get());
        */
    })->onNewFrame([&](const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Window& window) {

        auto r = realcounter.fetch_add(1);
        auto s = std::to_string(r) + std::string("WINNEW_") + std::string(".jpg");
        auto size = RowStride(img) * Height(img);
        
       /* auto imgbuffer(std::make_unique<unsigned char[]>(size));
        ExtractAndConvertToRGBA(img, imgbuffer.get(), size);
        tje_encode_to_file(s.c_str(), Width(img), Height(img), 4, (const unsigned char*)imgbuffer.get());
        */
        if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - onNewFramestart).count() >= 1000) {
            std::cout << "onNewFrame fps" << onNewFramecounter << std::endl;
            onNewFramecounter = 0;
            onNewFramestart = std::chrono::high_resolution_clock::now();
        }
        onNewFramecounter += 1;

    })->onMouseChanged([&](const SL::Screen_Capture::Image* img, const SL::Screen_Capture::Point& point) {

        auto r = realcounter.fetch_add(1);
        auto s = std::to_string(r) + std::string(" M") + std::string(".png");
        if (img) {
            //std::cout << "New mouse coordinates  AND NEW Image received." << " x= " << point.x << " y= " << point.y << std::endl;
            //lodepng::encode(s,StartSrc(*img), Width(*img), Height(*img));
        }
        else {
            //std::cout << "New mouse coordinates received." << " x= " << point.x << " y= " << point.y << " The mouse image is still the same as the last" << std::endl;
        }

    })->start_capturing();

    framgrabber->setFrameChangeInterval(std::chrono::milliseconds(100));
    framgrabber->setMouseChangeInterval(std::chrono::milliseconds(100));

}
int main()
{
    std::cout << "Starting Capture Demo/Test" << std::endl;
    std::cout << "Testing captured monitor bounds check" << std::endl;
    auto goodmonitors = SL::Screen_Capture::GetMonitors();
    for (auto& m : goodmonitors) {
        std::cout << m << std::endl;
        assert(isMonitorInsideBounds(goodmonitors, m));
    }
    auto badmonitors = SL::Screen_Capture::GetMonitors();

    for (auto m : badmonitors) {
        m.Height += 1;
        std::cout << m << std::endl;
        assert(!isMonitorInsideBounds(goodmonitors, m));
    }
    for (auto m : badmonitors) {
        m.Width += 1;
        std::cout << m << std::endl;
        assert(!isMonitorInsideBounds(goodmonitors, m));
    }
    std::cout << "Running standard capturing for 10 seconds" << std::endl;
    createwindowgrabber();
    std::this_thread::sleep_for(std::chrono::seconds(10));

    std::cout << "Running standard capturing for 10 seconds" << std::endl;
    createframegrabber();
    std::this_thread::sleep_for(std::chrono::seconds(10));

    std::cout << "Pausing for 10 seconds. " << std::endl;
    framgrabber->pause();
    auto i = 0;
    while (i++ < 10) {
        assert(framgrabber->isPaused());
        std::cout << " . ";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << std::endl << "Resuming  . . . for 5 seconds" << std::endl;
    framgrabber->resume();
    std::this_thread::sleep_for(std::chrono::seconds(5));

    std::cout
        << "Testing changing the interval during runtime for race conditions "
        << std::endl;

    // HAMMER THE SET FRAME INTERVAL FOR RACE CONDITIONS!!
    auto start = std::chrono::high_resolution_clock::now();
    while (std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::high_resolution_clock::now() - start)
        .count() < 10) {
        for (auto t = 0; t < 100; t++) {
            framgrabber->setFrameChangeInterval(std::chrono::microseconds(100));
            framgrabber->setMouseChangeInterval(std::chrono::microseconds(100));
        }
    }

    std::cout << "Changing the cpature rate to 1 second" << std::endl;
    framgrabber->setFrameChangeInterval(std::chrono::seconds(1));
    std::this_thread::sleep_for(std::chrono::seconds(5));

    std::cout << "Setting timer using chrono literals" << std::endl;
    // You can use chron's literals as well!
    framgrabber->setFrameChangeInterval(10ms);
    std::this_thread::sleep_for(std::chrono::seconds(5));

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
