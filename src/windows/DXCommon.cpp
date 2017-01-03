#include "DXCommon.h"
#include <iostream>

namespace SL {
	namespace Screen_Capture {

		// These are the errors we expect from general Dxgi API due to a transition
		HRESULT SystemTransitionsExpectedErrors[] = {
			DXGI_ERROR_DEVICE_REMOVED,
			DXGI_ERROR_ACCESS_LOST,
			static_cast<HRESULT>(WAIT_ABANDONED),
			S_OK                                    // Terminate list with zero valued HRESULT
		};

		// These are the errors we expect from IDXGIOutput1::DuplicateOutput due to a transition
		HRESULT CreateDuplicationExpectedErrors[] = {
			DXGI_ERROR_DEVICE_REMOVED,
			static_cast<HRESULT>(E_ACCESSDENIED),
			DXGI_ERROR_UNSUPPORTED,
			DXGI_ERROR_SESSION_DISCONNECTED,
			S_OK                                    // Terminate list with zero valued HRESULT
		};

		// These are the errors we expect from IDXGIOutputDuplication methods due to a transition
		HRESULT FrameInfoExpectedErrors[] = {
			DXGI_ERROR_DEVICE_REMOVED,
			DXGI_ERROR_ACCESS_LOST,
			S_OK                                    // Terminate list with zero valued HRESULT
		};

		// These are the errors we expect from IDXGIAdapter::EnumOutputs methods due to outputs becoming stale during a transition
		HRESULT EnumOutputsExpectedErrors[] = {
			DXGI_ERROR_NOT_FOUND,
			S_OK                                    // Terminate list with zero valued HRESULT
		};



		DUPL_RETURN ProcessFailure(ID3D11Device * Device, LPCWSTR Str, LPCWSTR Title, HRESULT hr, HRESULT * ExpectedErrors)
		{
			HRESULT TranslatedHr;
			std::wcout << Str << "\t" << Title << std::endl;
			// On an error check if the DX device is lost
			if (Device)
			{
				HRESULT DeviceRemovedReason = Device->GetDeviceRemovedReason();

				switch (DeviceRemovedReason)
				{
				case DXGI_ERROR_DEVICE_REMOVED:
				case DXGI_ERROR_DEVICE_RESET:
				case static_cast<HRESULT>(E_OUTOFMEMORY) :
				{
					// Our device has been stopped due to an external event on the GPU so map them all to
					// device removed and continue processing the condition
					TranslatedHr = DXGI_ERROR_DEVICE_REMOVED;
					break;
				}

				case S_OK:
				{
					// Device is not removed so use original error
					TranslatedHr = hr;
					break;
				}

				default:
				{
					// Device is removed but not a error we want to remap
					TranslatedHr = DeviceRemovedReason;
				}
				}
			}
			else
			{
				TranslatedHr = hr;
			}

			// Check if this error was expected or not
			if (ExpectedErrors)
			{
				HRESULT* CurrentResult = ExpectedErrors;

				while (*CurrentResult != S_OK)
				{
					if (*(CurrentResult++) == TranslatedHr)
					{
						return DUPL_RETURN_ERROR_EXPECTED;
					}
				}
			}


			return DUPL_RETURN_ERROR_UNEXPECTED;


		}


		DUPL_RETURN Initialize(DX_RESOURCES& data)
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

			// Create device
			for (UINT DriverTypeIndex = 0; DriverTypeIndex < NumDriverTypes; ++DriverTypeIndex)
			{
				hr = D3D11CreateDevice(nullptr, DriverTypes[DriverTypeIndex], nullptr, 0, FeatureLevels, NumFeatureLevels, D3D11_SDK_VERSION, data.Device.GetAddressOf(), &FeatureLevel, data.DeviceContext.GetAddressOf());
				if (SUCCEEDED(hr))
				{
					// Device creation success, no need to loop anymore
					break;
				}
			}
			if (FAILED(hr))
			{
				return ProcessFailure(nullptr, L"Failed to create device in InitializeDx", L"Error", hr);
			}

			return DUPL_RETURN_SUCCESS;

		}

		DUPL_RETURN Initialize(DUPLE_RESOURCES & r, ID3D11Device* device, const UINT output)
		{

			// Get DXGI device
			Microsoft::WRL::ComPtr<IDXGIDevice> DxgiDevice;
			HRESULT hr = device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(DxgiDevice.GetAddressOf()));
			if (FAILED(hr))
			{
				return ProcessFailure(nullptr, L"Failed to QI for DXGI Device", L"Error", hr);
			}

			// Get DXGI adapter
			Microsoft::WRL::ComPtr<IDXGIAdapter> DxgiAdapter;
			hr = DxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(DxgiAdapter.GetAddressOf()));
			if (FAILED(hr))
			{
				return ProcessFailure(device, L"Failed to get parent DXGI Adapter", L"Error", hr, SystemTransitionsExpectedErrors);
			}

			// Get output
			Microsoft::WRL::ComPtr<IDXGIOutput> DxgiOutput;
			hr = DxgiAdapter->EnumOutputs(output, DxgiOutput.GetAddressOf());

			if (FAILED(hr))
			{
				return ProcessFailure(device, L"Failed to get specified output in DUPLICATIONMANAGER", L"Error", hr, EnumOutputsExpectedErrors);
			}

			DxgiOutput->GetDesc(&r.OutputDesc);

			// QI for Output 1
			Microsoft::WRL::ComPtr<IDXGIOutput1> DxgiOutput1;
			hr = DxgiOutput.Get()->QueryInterface(__uuidof(IDXGIOutput1), reinterpret_cast<void**>(DxgiOutput1.GetAddressOf()));
			if (FAILED(hr))
			{
				return ProcessFailure(nullptr, L"Failed to QI for DxgiOutput1 in DUPLICATIONMANAGER", L"Error", hr);
			}

			// Create desktop duplication
			hr = DxgiOutput1->DuplicateOutput(device, r.OutputDuplication.GetAddressOf());
			if (FAILED(hr))
			{
				return ProcessFailure(device, L"Failed to get duplicate output in DUPLICATIONMANAGER", L"Error", hr, CreateDuplicationExpectedErrors);
			}
			r.Output = output;
			return DUPL_RETURN_SUCCESS;


		}
		RECT ConvertRect(RECT Dirty, const DXGI_OUTPUT_DESC& DeskDesc) {
			RECT DestDirty = Dirty;
			INT Width = DeskDesc.DesktopCoordinates.right - DeskDesc.DesktopCoordinates.left;
			INT Height = DeskDesc.DesktopCoordinates.bottom - DeskDesc.DesktopCoordinates.top;

			// Set appropriate coordinates compensated for rotation
			switch (DeskDesc.Rotation)
			{
			case DXGI_MODE_ROTATION_ROTATE90:
			{

				DestDirty.left = Width - Dirty.bottom;
				DestDirty.top = Dirty.left;
				DestDirty.right = Width - Dirty.top;
				DestDirty.bottom = Dirty.right;

				break;
			}
			case DXGI_MODE_ROTATION_ROTATE180:
			{
				DestDirty.left = Width - Dirty.right;
				DestDirty.top = Height - Dirty.bottom;
				DestDirty.right = Width - Dirty.left;
				DestDirty.bottom = Height - Dirty.top;

				break;
			}
			case DXGI_MODE_ROTATION_ROTATE270:
			{
				DestDirty.left = Dirty.top;
				DestDirty.top = Height - Dirty.right;
				DestDirty.right = Dirty.bottom;
				DestDirty.bottom = Height - Dirty.left;

				break;
			}
			case DXGI_MODE_ROTATION_UNSPECIFIED:
			case DXGI_MODE_ROTATION_IDENTITY:
			{
				break;
			}
			default:
				break;
			}
			return DestDirty;
		}
	}

}
