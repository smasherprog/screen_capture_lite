#include "DXThreadManager.h"
#include "DXFrameProcessor.h"
#include "DXDuplicationManager.h"
#include <string>

namespace SL {
	namespace Screen_Capture {


		UINT GetMonitorCount() {

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
			HRESULT hr;

			Microsoft::WRL::ComPtr<ID3D11Device> m_Device;
			Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_DeviceContext;
			Microsoft::WRL::ComPtr<IDXGIDevice> DxgiDevice;
			Microsoft::WRL::ComPtr<IDXGIAdapter> DxgiAdapter;

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



			hr = m_Device.Get()->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(DxgiDevice.GetAddressOf()));
			if (FAILED(hr))
			{
				return ProcessFailure(nullptr, L"Failed to QI for DXGI Device", L"Error", hr);
			}


			hr = DxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(DxgiAdapter.GetAddressOf()));

			if (FAILED(hr))
			{
				return ProcessFailure(m_Device.Get(), L"Failed to get parent DXGI Adapter", L"Error", hr, SystemTransitionsExpectedErrors);
			}


		

			// Figure out right dimensions for full size desktop texture and # of outputs to duplicate
			UINT OutputCount = 0;

			hr = S_OK;
			for (OutputCount = 0; SUCCEEDED(hr); ++OutputCount)
			{
				Microsoft::WRL::ComPtr<IDXGIOutput> DxgiOutput;
				hr = DxgiAdapter->EnumOutputs(OutputCount, DxgiOutput.GetAddressOf());
			}

			--OutputCount;
			return OutputCount;
		}

		DWORD ProcessExit(DUPL_RETURN Ret, THREAD_DATA* TData) {
			if (Ret != DUPL_RETURN_SUCCESS)
			{
				if (Ret == DUPL_RETURN_ERROR_EXPECTED)
				{
					// The system is in a transition state so request the duplication be restarted
					*TData->ExpectedErrorEvent = true;
				}
				else
				{
					// Unexpected error so exit the application
					*TData->UnexpectedErrorEvent = true;
				}
			}
			return 0;
		}



		DWORD WINAPI RunThread(void* Param) {

			std::shared_ptr<THREAD_DATA> TData = *(reinterpret_cast<std::shared_ptr<THREAD_DATA>*>(Param));
			// Classes
			DXFrameProcessor DispMgr;
			DXDuplicationManager DuplMgr;

			DUPL_RETURN Ret;
			HDESK CurrentDesktop = nullptr;
			CurrentDesktop = OpenInputDesktop(0, FALSE, GENERIC_ALL);
			if (!CurrentDesktop)
			{
				// We do not have access to the desktop so request a retry
				*TData->ExpectedErrorEvent = true;
				Ret = DUPL_RETURN_ERROR_EXPECTED;
				return ProcessExit(Ret, TData.get());
			}

			// Attach desktop to this thread
			bool DesktopAttached = SetThreadDesktop(CurrentDesktop) != 0;
			CloseDesktop(CurrentDesktop);
			CurrentDesktop = nullptr;
			if (!DesktopAttached)
			{
				// We do not have access to the desktop so request a retry
				Ret = DUPL_RETURN_ERROR_EXPECTED;
				return ProcessExit(Ret, TData.get());
			}

			// New display manager
			DispMgr.InitD3D(TData.get());


			// Make duplication manager
			Ret = DuplMgr.InitDupl(TData->DxRes.Device.Get(), TData->Output);
			if (Ret != DUPL_RETURN_SUCCESS)
			{
				return ProcessExit(Ret, TData.get());
			}

			// Get output description
			DXGI_OUTPUT_DESC DesktopDesc;
			RtlZeroMemory(&DesktopDesc, sizeof(DXGI_OUTPUT_DESC));
			DuplMgr.GetOutputDesc(&DesktopDesc);

			// Main duplication loop
			bool WaitToProcessCurrentFrame = false;
			FRAME_DATA CurrentData;

			while (!*TData->TerminateThreadsEvent)
			{
				auto start = std::chrono::high_resolution_clock::now();
				if (!WaitToProcessCurrentFrame)
				{
					// Get new frame from desktop duplication
					bool TimeOut;
					Ret = DuplMgr.GetFrame(&CurrentData, &TimeOut);
					if (Ret != DUPL_RETURN_SUCCESS)
					{
						// An error occurred getting the next frame drop out of loop which
						// will check if it was expected or not
						break;
					}

					// Check for timeout
					if (TimeOut)
					{
						// No new frame at the moment
						continue;
					}
				}


				// We can now process the current frame
				WaitToProcessCurrentFrame = false;

				// Get mouse info
				//Ret = DuplMgr.GetMouse(TData->PtrInfo.get(), &(CurrentData.FrameInfo), &DesktopDesc);
				if (Ret != DUPL_RETURN_SUCCESS)
				{
					DuplMgr.DoneWithFrame();
					break;
				}

				// Process new frame
				Ret = DispMgr.ProcessFrame(&CurrentData, &DesktopDesc);
				if (Ret != DUPL_RETURN_SUCCESS)
				{
					DuplMgr.DoneWithFrame();
					break;
				}
				// Release frame back to desktop duplication
				Ret = DuplMgr.DoneWithFrame();
				if (Ret != DUPL_RETURN_SUCCESS)
				{
					break;
				}
				std::string msg = "took ";
				msg += std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count()) + "ms\n";
				OutputDebugStringA(msg.c_str());


			}

			return 0;


		}

		DXThreadManager::DXThreadManager()
		{
			m_PtrInfo = std::make_shared<PTR_INFO>();
		}
		DXThreadManager::~DXThreadManager()
		{
			Clean();
		}
		void DXThreadManager::Clean()
		{
			m_PtrInfo = std::make_shared<PTR_INFO>();
			for (auto t : m_ThreadHandles) {
				CloseHandle(t);
			}
			m_ThreadHandles.resize(0);
			m_ThreadData.resize(0);
			m_ThreadCount = 0;
		}

		DUPL_RETURN DXThreadManager::Initialize(std::shared_ptr<std::atomic_bool> UnexpectedErrorEvent, std::shared_ptr<std::atomic_bool> ExpectedErrorEvent, std::shared_ptr<std::atomic_bool> TerminateThreadsEvent, ImageCallback& cb)
		{

			m_ThreadCount = GetMonitorCount();
			m_ThreadHandles.resize(m_ThreadCount);
			m_ThreadData.resize(m_ThreadCount);

			// Create appropriate # of threads for duplication
			DUPL_RETURN Ret = DUPL_RETURN_SUCCESS;
			for (int i = 0; i < m_ThreadCount; ++i)
			{
				m_ThreadData[i] = std::make_shared<THREAD_DATA>();
				m_ThreadData[i]->UnexpectedErrorEvent = UnexpectedErrorEvent;
				m_ThreadData[i]->ExpectedErrorEvent = ExpectedErrorEvent;
				m_ThreadData[i]->TerminateThreadsEvent = TerminateThreadsEvent;
				m_ThreadData[i]->Output = i;
				m_ThreadData[i]->CallBack = cb;
				m_ThreadData[i]->PtrInfo = m_PtrInfo;

				Ret = InitializeDx(&m_ThreadData[i]->DxRes);
				if (Ret != DUPL_RETURN_SUCCESS)
				{
					return Ret;
				}

				DWORD ThreadId;
				m_ThreadHandles[i] = CreateThread(nullptr, 0, SL::Screen_Capture::RunThread, &m_ThreadData[i], 0, &ThreadId);
				if (m_ThreadHandles[i] == nullptr)
				{
					return ProcessFailure(nullptr, L"Failed to create thread", L"Error", E_FAIL);
				}

			}

			return Ret;
		}
		PTR_INFO * DXThreadManager::GetPointerInfo()
		{
			return m_PtrInfo.get();
		}
		void DXThreadManager::WaitForThreadTermination()
		{
			if (m_ThreadCount != 0)
			{
				WaitForMultipleObjectsEx(m_ThreadCount, m_ThreadHandles.data(), TRUE, INFINITE, FALSE);
			}
		}
		DUPL_RETURN DXThreadManager::InitializeDx(DX_RESOURCES * Data)
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
				hr = D3D11CreateDevice(nullptr, DriverTypes[DriverTypeIndex], nullptr, 0, FeatureLevels, NumFeatureLevels, D3D11_SDK_VERSION, Data->Device.GetAddressOf(), &FeatureLevel, Data->DeviceContext.GetAddressOf());
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

			Microsoft::WRL::ComPtr<ID3D10Blob> pBlobVS;

			hr = CompileShader(g_VS, "VS", "vs_4_0", pBlobVS.GetAddressOf());
			if (FAILED(hr))
			{
				return ProcessFailure(Data->Device.Get(), L"Failed to create vertex shader in OUTPUTMANAGER", L"Error", hr, SystemTransitionsExpectedErrors);
			}
			hr = Data->Device->CreateVertexShader(pBlobVS->GetBufferPointer(), pBlobVS->GetBufferSize(), NULL, Data->VertexShader.GetAddressOf());
			if (FAILED(hr))
			{
				return ProcessFailure(Data->Device.Get(), L"Failed to create vertex shader in OUTPUTMANAGER", L"Error", hr, SystemTransitionsExpectedErrors);
			}


			// Input layout
			D3D11_INPUT_ELEMENT_DESC Layout[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
			};
			UINT NumElements = ARRAYSIZE(Layout);
			hr = Data->Device->CreateInputLayout(Layout, NumElements, pBlobVS->GetBufferPointer(), pBlobVS->GetBufferSize(), Data->InputLayout.GetAddressOf());
			if (FAILED(hr))
			{
				return ProcessFailure(Data->Device.Get(), L"Failed to create input layout in InitializeDx", L"Error", hr, SystemTransitionsExpectedErrors);
			}
			Data->DeviceContext->IASetInputLayout(Data->InputLayout.Get());

			Microsoft::WRL::ComPtr<ID3D10Blob> pBlobPS;
			hr = CompileShader(g_PS, "PS", "ps_4_0", pBlobPS.GetAddressOf());
			if (FAILED(hr))
			{
				return ProcessFailure(Data->Device.Get(), L"Failed to create pixel shader in OUTPUTMANAGER", L"Error", hr, SystemTransitionsExpectedErrors);
			}


			hr = Data->Device->CreatePixelShader(pBlobPS->GetBufferPointer(), pBlobPS->GetBufferSize(), nullptr, Data->PixelShader.GetAddressOf());
			if (FAILED(hr))
			{
				return ProcessFailure(Data->Device.Get(), L"Failed to create pixel shader in InitializeDx", L"Error", hr, SystemTransitionsExpectedErrors);
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
			hr = Data->Device->CreateSamplerState(&SampDesc, Data->SamplerLinear.GetAddressOf());
			if (FAILED(hr))
			{
				return ProcessFailure(Data->Device.Get(), L"Failed to create sampler state in InitializeDx", L"Error", hr, SystemTransitionsExpectedErrors);
			}
			return DUPL_RETURN_SUCCESS;

		}

	}
}