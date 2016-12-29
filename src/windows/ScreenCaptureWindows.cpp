#include "ScreenCaptureWindows.h"
#include "CommonTypes.h"
#include <thread>
#include "DXDuplicationManager.h"
#include "DXFrameProcessor.h"
#include "DXThreadManager.h"
#include <memory>

namespace SL {
	namespace Screen_Capture {

	
		class ScreenCaptureWindowsImpl {

		public:
			std::shared_ptr<THREAD_DATA> Data;



			static auto desktopdc(RAIIHDC(CreateDCA("DISPLAY", NULL, NULL, NULL)));
			static auto capturedc(RAIIHDC(CreateCompatibleDC(desktopdc.get())));
			static auto capturebmp(RAIIHBITMAP(CreateCompatibleBitmap(desktopdc.get(), ret.Width, ret.Height)));



			ScreenCaptureWindowsImpl() {

			}
			~ScreenCaptureWindowsImpl() {

			}
		};

		ScreenCaptureWindows::ScreenCaptureWindows(std::shared_ptr<THREAD_DATA>& data)
		{

		}
		ScreenCaptureWindows::~ScreenCaptureWindows()
		{
			
		}


		SL::Screen_Capture::DUPL_RETURN ScreenCaptureWindows::IsDesktopDuplicationSupported()
		{
			HRESULT hr = S_OK;
			// Driver types supported
			D3D_DRIVER_TYPE DriverTypes[] =
			{
				D3D_DRIVER_TYPE_HARDWARE,
				D3D_DRIVER_TYPE_WARP,
				D3D_DRIVER_TYPE_REFERENCE,
			};
			UINT NumDriverTypes = ARRAYSIZE(DriverTypes);

			// Feature levels supported
			D3D_FEATURE_LEVEL FeatureLevels[] =
			{
				D3D_FEATURE_LEVEL_11_0,
				D3D_FEATURE_LEVEL_10_1,
				D3D_FEATURE_LEVEL_10_0,
				D3D_FEATURE_LEVEL_9_1
			};
			UINT NumFeatureLevels = ARRAYSIZE(FeatureLevels);
			D3D_FEATURE_LEVEL FeatureLevel;


			Microsoft::WRL::ComPtr<ID3D11Device> m_Device;
			Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_DeviceContext;

			for (UINT DriverTypeIndex = 0; DriverTypeIndex < NumDriverTypes; ++DriverTypeIndex)
			{
				hr = D3D11CreateDevice(nullptr, DriverTypes[DriverTypeIndex], nullptr, 0, FeatureLevels, NumFeatureLevels, D3D11_SDK_VERSION, m_Device.GetAddressOf(), &FeatureLevel, m_DeviceContext.GetAddressOf());
				if (SUCCEEDED(hr))
				{
					// Device creation succeeded, no need to loop anymore
					break;
				}
			}
			if (FAILED(hr))
			{
				return ProcessFailure(m_Device.Get(), L"Device creation in OUTPUTMANAGER failed", L"Error", hr, SystemTransitionsExpectedErrors);
			}

			// Get DXGI device
			Microsoft::WRL::ComPtr<IDXGIDevice> DxgiDevice;
			hr = m_Device.Get()->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(DxgiDevice.GetAddressOf()));
			if (FAILED(hr))
			{
				return ProcessFailure(nullptr, L"Failed to QI for DXGI Device", L"Error", hr);
			}

			// Get DXGI adapter
			Microsoft::WRL::ComPtr<IDXGIAdapter> DxgiAdapter;
			hr = DxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(DxgiAdapter.GetAddressOf()));
			if (FAILED(hr))
			{
				return ProcessFailure(m_Device.Get(), L"Failed to get parent DXGI Adapter", L"Error", hr, SystemTransitionsExpectedErrors);
			}

			// Get output
			Microsoft::WRL::ComPtr<IDXGIOutput> DxgiOutput;
			hr = DxgiAdapter->EnumOutputs(0, DxgiOutput.GetAddressOf());

			if (FAILED(hr))
			{
				return ProcessFailure(m_Device.Get(), L"Failed to get specified output in DUPLICATIONMANAGER", L"Error", hr, EnumOutputsExpectedErrors);
			}

			// QI for Output 1
			Microsoft::WRL::ComPtr<IDXGIOutput1> DxgiOutput1;
			hr = DxgiOutput.Get()->QueryInterface(__uuidof(IDXGIOutput1), reinterpret_cast<void**>(DxgiOutput1.GetAddressOf()));
			if (FAILED(hr))
			{
				return ProcessFailure(nullptr, L"Failed to QI for DxgiOutput1 in DUPLICATIONMANAGER", L"Error", hr);
			}
			Microsoft::WRL::ComPtr<IDXGIOutputDuplication> m_DeskDupl;
			// Create desktop duplication
			hr = DxgiOutput1->DuplicateOutput(m_Device.Get(), m_DeskDupl.GetAddressOf());
			if (FAILED(hr))
			{
				return ProcessFailure(m_Device.Get(), L"Failed to get duplicate output in DUPLICATIONMANAGER", L"Error", hr, CreateDuplicationExpectedErrors);
			}
			return SL::Screen_Capture::DUPL_RETURN::DUPL_RETURN_SUCCESS;
		}


	}
}


