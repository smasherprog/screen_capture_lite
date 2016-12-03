#include "DXThreadManager.h"
#include "DXFrameProcessor.h"
#include "DXDuplicationManager.h"
#include <string>

namespace SL {
	namespace Screen_Capture {



		HRESULT SystemTransitionsExpectedErrors[] = {
			DXGI_ERROR_DEVICE_REMOVED,
			DXGI_ERROR_ACCESS_LOST,
			static_cast<HRESULT>(WAIT_ABANDONED),
			S_OK                                    // Terminate list with zero valued HRESULT
		};
		DWORD ProcessExit(DUPL_RETURN Ret, THREAD_DATA* TData) {
			if (Ret != DUPL_RETURN_SUCCESS)
			{
				if (Ret == DUPL_RETURN_ERROR_EXPECTED)
				{
					// The system is in a transition state so request the duplication be restarted
					SetEvent(TData->ExpectedErrorEvent);
				}
				else
				{
					// Unexpected error so exit the application
					SetEvent(TData->UnexpectedErrorEvent);
				}
			}
			return 0;
		}



		DWORD WINAPI RunThread(void* Param) {

			std::shared_ptr<THREAD_DATA> TData = *(reinterpret_cast<std::shared_ptr<THREAD_DATA>*>(Param));
			// Classes
			DXFrameProcessor DispMgr;
			DXDuplicationManager DuplMgr;

			// D3D objects
			Microsoft::WRL::ComPtr<ID3D11Texture2D> SharedSurf;
			Microsoft::WRL::ComPtr<IDXGIKeyedMutex> KeyMutex;
			DUPL_RETURN Ret;
			HDESK CurrentDesktop = nullptr;
			CurrentDesktop = OpenInputDesktop(0, FALSE, GENERIC_ALL);
			if (!CurrentDesktop)
			{
				// We do not have access to the desktop so request a retry
				SetEvent(TData->ExpectedErrorEvent);
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
			DispMgr.InitD3D(&TData->DxRes);
	
			// Obtain handle to sync shared Surface
			HRESULT hr = TData->DxRes.Device->OpenSharedResource(TData->TexSharedHandle, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(SharedSurf.GetAddressOf()));
			if (FAILED(hr))
			{
				Ret = ProcessFailure(TData->DxRes.Device.Get(), L"Opening shared texture failed", L"Error", hr, SystemTransitionsExpectedErrors);
				return ProcessExit(Ret, TData.get());
			}

			hr = SharedSurf.Get()->QueryInterface(__uuidof(IDXGIKeyedMutex), reinterpret_cast<void**>(KeyMutex.GetAddressOf()));
			if (FAILED(hr))
			{
				Ret = ProcessFailure(nullptr, L"Failed to get keyed mutex interface in spawned thread", L"Error", hr);
				return ProcessExit(Ret, TData.get());
			}

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

			while ((WaitForSingleObjectEx(TData->TerminateThreadsEvent, 0, FALSE) == WAIT_TIMEOUT))
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

				// We have a new frame so try and process it
				// Try to acquire keyed mutex in order to access shared surface
				hr = KeyMutex->AcquireSync(0, 1000);
				if (hr == static_cast<HRESULT>(WAIT_TIMEOUT))
				{
					// Can't use shared surface right now, try again later
					WaitToProcessCurrentFrame = true;
					continue;
				}
				else if (FAILED(hr))
				{
					// Generic unknown failure
					Ret = ProcessFailure(TData->DxRes.Device.Get(), L"Unexpected error acquiring KeyMutex", L"Error", hr, SystemTransitionsExpectedErrors);
					DuplMgr.DoneWithFrame();
					break;
				}

				// We can now process the current frame
				WaitToProcessCurrentFrame = false;

				// Get mouse info
				Ret = DuplMgr.GetMouse(TData->PtrInfo.get(), &(CurrentData.FrameInfo), TData->OffsetX, TData->OffsetY);
				if (Ret != DUPL_RETURN_SUCCESS)
				{
					DuplMgr.DoneWithFrame();
					KeyMutex->ReleaseSync(1);
					break;
				}

				// Process new frame
				Ret = DispMgr.ProcessFrame(&CurrentData, SharedSurf.Get(), TData->OffsetX, TData->OffsetY, &DesktopDesc);
				if (Ret != DUPL_RETURN_SUCCESS)
				{
					DuplMgr.DoneWithFrame();
					KeyMutex->ReleaseSync(1);
					break;
				}

				// Release acquired keyed mutex
				hr = KeyMutex->ReleaseSync(1);
				if (FAILED(hr))
				{
					Ret = ProcessFailure(TData->DxRes.Device.Get(), L"Unexpected error releasing the keyed mutex", L"Error", hr, SystemTransitionsExpectedErrors);
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

		DUPL_RETURN DXThreadManager::Initialize(INT SingleOutput, UINT OutputCount, HANDLE UnexpectedErrorEvent, HANDLE ExpectedErrorEvent, HANDLE TerminateThreadsEvent, HANDLE SharedHandle, _In_ RECT* DesktopDim)
		{
			m_ThreadCount = OutputCount;
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
				m_ThreadData[i]->Output = (SingleOutput < 0) ? i : SingleOutput;
				m_ThreadData[i]->TexSharedHandle = SharedHandle;
				m_ThreadData[i]->OffsetX = DesktopDim->left;
				m_ThreadData[i]->OffsetY = DesktopDim->top;
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