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

		DUPL_RETURN DesktopDuplicationSupported()
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
			return DUPL_RETURN::DUPL_RETURN_SUCCESS;
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


			UINT Size = ARRAYSIZE(g_VS);
			hr = data.Device->CreateVertexShader(g_VS, Size, nullptr, data.VertexShader.GetAddressOf());
			if (FAILED(hr))
			{
				return ProcessFailure(data.Device.Get(), L"Failed to create vertex shader in OUTPUTMANAGER", L"Error", hr, SystemTransitionsExpectedErrors);
			}


			// Input layout
			D3D11_INPUT_ELEMENT_DESC Layout[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
			};
			UINT NumElements = ARRAYSIZE(Layout);
			hr = data.Device->CreateInputLayout(Layout, NumElements, g_VS, Size, data.InputLayout.GetAddressOf());
			if (FAILED(hr))
			{
				return ProcessFailure(data.Device.Get(), L"Failed to create input layout in InitializeDx", L"Error", hr, SystemTransitionsExpectedErrors);
			}
			data.DeviceContext->IASetInputLayout(data.InputLayout.Get());

			Size = ARRAYSIZE(g_PS);
			hr = data.Device->CreatePixelShader(g_PS, Size, nullptr, data.PixelShader.GetAddressOf());

			if (FAILED(hr))
			{
				return ProcessFailure(data.Device.Get(), L"Failed to create pixel shader in InitializeDx", L"Error", hr, SystemTransitionsExpectedErrors);
			}

			// Set up sampler
			D3D11_SAMPLER_DESC SampDesc;
			RtlZeroMemory(&SampDesc, sizeof(SampDesc));
			SampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			SampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			SampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			SampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			SampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			SampDesc.MinLOD = 0;
			SampDesc.MaxLOD = D3D11_FLOAT32_MAX;
			hr = data.Device->CreateSamplerState(&SampDesc, data.SamplerLinear.GetAddressOf());
			if (FAILED(hr))
			{
				return ProcessFailure(data.Device.Get(), L"Failed to create sampler state in InitializeDx", L"Error", hr, SystemTransitionsExpectedErrors);
			}
			return DUPL_RETURN_SUCCESS;

		}


	}

}
