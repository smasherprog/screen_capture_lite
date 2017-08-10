#include "ScreenCapture.h"
#include "SCCommon.h"
#include <algorithm>
#include <string>
#include "TargetConditionals.h"
#include <ApplicationServices/ApplicationServices.h>
#include <iostream>

namespace SL
{
namespace Screen_Capture
{

    std::vector<Window> GetWindows(const std::string& name1, WindowStringMatch searchby)
    {
        auto nametofind = name1;
        std::transform(nametofind.begin(), nametofind.end(), nametofind.begin(), [](char c) {
            return std::tolower(c, std::locale());
        }); // convert to lower
        auto windowList = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly, kCGNullWindowID);
        std::vector<Window> ret;
        CFIndex numWindows = CFArrayGetCount(windowList );
        
        for( int i = 0; i < (int)numWindows; i++ ) {
            char buffer[400] ={0};
            uint32_t windowid=0;
            auto dict = static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(windowList, i));
            auto cfwindowname = static_cast<CFStringRef>(CFDictionaryGetValue(dict, kCGWindowName));
            CFStringGetCString(cfwindowname, buffer, 400, kCFStringEncodingUTF8);
            std::string windowname=buffer;
            std::transform(windowname.begin(), windowname.end(), windowname.begin(), [](char c) {
                return std::tolower(c, std::locale());
            }); // convert to lower
            
            bool found = false;
            if(searchby == WindowStringMatch::STARTSWITH) {
                auto f = windowname.substr(0, nametofind.size());
                found = f == nametofind;
            } else if(searchby == WindowStringMatch::EXACT) {
                found = windowname == nametofind;
            } else if(searchby == WindowStringMatch::CONTAINS) {
                found = windowname.find(nametofind) != std::string::npos;
            }
            std::cout<<windowname<<std::endl;
            
            if(found){
               Window w;
                CFNumberGetValue(static_cast<CFNumberRef>(CFDictionaryGetValue(dict, kCGWindowNumber)), kCFNumberIntType, &windowid);
                w.Handle = static_cast<size_t>(windowid);
               
                auto dims =static_cast<CFDictionaryRef>(CFDictionaryGetValue(dict,kCGWindowBounds));
                CGRect rect;
                CGRectMakeWithDictionaryRepresentation(dims, &rect);
                w.Position.x = static_cast<int>(rect.origin.x);
                w.Position.y = static_cast<int>(rect.origin.y);
                
                w.Size.x = static_cast<int>(rect.size.width);
                w.Size.y = static_cast<int>(rect.size.height);
                
                memcpy(w.Name, windowname.c_str(), windowname.size() + 1);
                ret.push_back(w); 
            }
        }
        CFRelease(windowList);
        return ret;
    }
}
}
