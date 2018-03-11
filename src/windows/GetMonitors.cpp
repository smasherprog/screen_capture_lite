#include "SCCommon.h"
#include "ScreenCapture.h"
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <Windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl.h>

#include <locale>
#include <codecvt>

#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3d11.lib")

namespace SL {
namespace Screen_Capture {

    float scaleFromDpi(int xdpi)
    {
        switch (xdpi) {
        case 96:
            return 1.0f;
            break;
        case 120:
            return 1.25f;
            break;
        case 144:
            return 1.5f;
            break;
        case 192:
            return 2.0f;
            break;
        }

        return 1.0f;
    }

    std::vector<Monitor> GetMonitors()
    {
        std::vector<Monitor> ret;

        IDXGIAdapter * pAdapter;
        std::vector <IDXGIAdapter*> vAdapters;
        IDXGIFactory* pFactory = NULL;

        // Create a DXGIFactory object.
        if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory)))
        {
            //Couldn't enumerate displays with DXGI, fall back to EnumDisplayDevices
            DISPLAY_DEVICEA dd;
            ZeroMemory(&dd, sizeof(dd));
            dd.cb = sizeof(dd);
            for (auto i = 0; EnumDisplayDevicesA(NULL, i, &dd, 0); i++) {
                // monitor must be attached to desktop and not a mirroring device

                if (((dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) != 0 || (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) != 0) &&
                    (dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) == 0) {
                    DEVMODEA devMode;
                    devMode.dmSize = sizeof(devMode);
                    EnumDisplaySettingsA(dd.DeviceName, ENUM_CURRENT_SETTINGS, &devMode);
                    std::string name = dd.DeviceName;
                    auto mon = CreateDCA(dd.DeviceName, NULL, NULL, NULL);
                    auto xdpi = GetDeviceCaps(mon, LOGPIXELSX);
                    DeleteDC(mon);
                    auto scale = scaleFromDpi(xdpi);
                    
                    ret.push_back(CreateMonitor(static_cast<int>(ret.size()), i, devMode.dmPelsHeight, devMode.dmPelsWidth, devMode.dmPosition.x,
                        devMode.dmPosition.y, name, scale));
                }
            }
            return ret;
        }

        for (UINT i = 0;
            pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND;
            ++i)
        {
            IDXGIOutput * pOutput;

            for (UINT j = 0;
                pAdapter->EnumOutputs(j, &pOutput) != DXGI_ERROR_NOT_FOUND;
                ++j)
            {
                DXGI_OUTPUT_DESC desc;
                pOutput->GetDesc(&desc);

                std::wstring wname = desc.DeviceName;

                int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wname[0], (int)wname.size(), NULL, 0, NULL, NULL);
                std::string name(size_needed, 0);
                WideCharToMultiByte(CP_UTF8, 0, &wname[0], (int)wname.size(), &name[0], size_needed, NULL, NULL);

                DEVMODEW devMode;
                EnumDisplaySettingsW(desc.DeviceName, ENUM_CURRENT_SETTINGS, &devMode);

                auto mon = CreateDCW(desc.DeviceName, NULL, NULL, NULL);
                auto xdpi = GetDeviceCaps(mon, LOGPIXELSX);
                DeleteDC(mon);
                auto scale = scaleFromDpi(xdpi);

                bool flipSides = desc.Rotation == DXGI_MODE_ROTATION_ROTATE90 || desc.Rotation == DXGI_MODE_ROTATION_ROTATE270;
                ret.push_back(CreateMonitor(static_cast<int>(ret.size()), j, i, flipSides ? devMode.dmPelsWidth : devMode.dmPelsHeight, flipSides ? devMode.dmPelsHeight : devMode.dmPelsWidth,
                    devMode.dmPosition.x, devMode.dmPosition.y, name, scale));
            }
        }

        return ret;
    }
} // namespace Screen_Capture
} // namespace SL