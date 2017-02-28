#include "DXFrameProcessor.h"
#include <memory>
#include <atomic>
#include <mutex>
#include <string>
#include <iostream>

#if (_MSC_VER >= 1700) && defined(_USING_V110_SDK71_)
namespace SL {
	namespace Screen_Capture {

		struct DXFrameProcessorImpl {

		};


		DXFrameProcessor::DXFrameProcessor()
		{

		}

		DXFrameProcessor::~DXFrameProcessor()
		{

		}
		DUPL_RETURN DXFrameProcessor::Init(std::shared_ptr<Monitor_Thread_Data> data) {
			return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
		}
		DUPL_RETURN DXFrameProcessor::ProcessFrame() {
			return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
		}

	}
}
#else

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl.h>

#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3d11.lib")


namespace SL {
	namespace Screen_Capture {
		struct DX_RESOURCES
		{
			Microsoft::WRL::ComPtr<ID3D11Device> Device;
			Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeviceContext;
		};
		struct DUPLE_RESOURCES
		{
			Microsoft::WRL::ComPtr<IDXGIOutputDuplication> OutputDuplication;
			DXGI_OUTPUT_DESC OutputDesc;
			UINT Output;
		};


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



		DUPL_RETURN ProcessFailure(ID3D11Device * Device, LPCWSTR Str, LPCWSTR Title, HRESULT hr, HRESULT * ExpectedErrors = nullptr)
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

		class AquireFrameRAII {

			IDXGIOutputDuplication* _DuplLock;
			bool AquiredLock;
			void TryRelease() {
				if (AquiredLock) {
					auto hr = _DuplLock->ReleaseFrame();
					if (FAILED(hr) && hr != DXGI_ERROR_WAIT_TIMEOUT)
					{
						ProcessFailure(nullptr, L"Failed to release frame in DUPLICATIONMANAGER", L"Error", hr, FrameInfoExpectedErrors);
					}
				}
				AquiredLock = false;
			}
		public:
			AquireFrameRAII(IDXGIOutputDuplication* dupl) : _DuplLock(dupl), AquiredLock(false) {	}

			~AquireFrameRAII() {
				TryRelease();
			}
			HRESULT AcquireNextFrame(UINT TimeoutInMilliseconds, DXGI_OUTDUPL_FRAME_INFO *pFrameInfo, IDXGIResource **ppDesktopResource) {
				auto hr = _DuplLock->AcquireNextFrame(TimeoutInMilliseconds, pFrameInfo, ppDesktopResource);
				TryRelease();
				AquiredLock = SUCCEEDED(hr);
				return hr;
			}
		};
		class MAPPED_SUBRESOURCERAII {
			ID3D11DeviceContext* _Context;
			ID3D11Resource *_Resource;
			UINT _Subresource;
		public:
			MAPPED_SUBRESOURCERAII(ID3D11DeviceContext* context) : _Context(context), _Resource(nullptr), _Subresource(0) {	}

			~MAPPED_SUBRESOURCERAII() {
				_Context->Unmap(_Resource, _Subresource);
			}
			HRESULT Map(ID3D11Resource *pResource, UINT Subresource, D3D11_MAP MapType, UINT MapFlags, D3D11_MAPPED_SUBRESOURCE *pMappedResource) {
				if (_Resource != nullptr) {
					_Context->Unmap(_Resource, _Subresource);
				}
				_Resource = pResource;
				_Subresource = Subresource;
				return _Context->Map(_Resource, _Subresource, MapType, MapFlags, pMappedResource);
			}
		};

		struct DXFrameProcessorImpl {
			Microsoft::WRL::ComPtr<ID3D11Device> Device;
			Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeviceContext;
			Microsoft::WRL::ComPtr<ID3D11Texture2D> StagingSurf;

			Microsoft::WRL::ComPtr<IDXGIOutputDuplication> OutputDuplication;
			DXGI_OUTPUT_DESC OutputDesc;
			UINT Output;
			std::vector<BYTE> MetaDataBuffer;

			std::shared_ptr<Monitor_Thread_Data> Data;
		};


		DXFrameProcessor::DXFrameProcessor()
		{
			_DXFrameProcessorImpl = std::make_unique<DXFrameProcessorImpl>();
		}

		DXFrameProcessor::~DXFrameProcessor()
		{

		}
		DUPL_RETURN DXFrameProcessor::Init(std::shared_ptr<Monitor_Thread_Data> data) {
			DX_RESOURCES res;
			auto ret = Initialize(res);
			if (ret != DUPL_RETURN_SUCCESS) {
				return ret;
			}
			DUPLE_RESOURCES dupl;
			ret = Initialize(dupl, res.Device.Get(), Id(data->SelectedMonitor));
			if (ret != DUPL_RETURN_SUCCESS) {
				return ret;
			}
			_DXFrameProcessorImpl->Device = res.Device;
			_DXFrameProcessorImpl->DeviceContext = res.DeviceContext;
			_DXFrameProcessorImpl->OutputDuplication = dupl.OutputDuplication;
			_DXFrameProcessorImpl->OutputDesc = dupl.OutputDesc;
			_DXFrameProcessorImpl->Output = dupl.Output;

			_DXFrameProcessorImpl->Data = data;
			return ret;
		}
		//
		// Process a given frame and its metadata
		//
		DUPL_RETURN DXFrameProcessor::ProcessFrame()
		{
			auto Ret = DUPL_RETURN_SUCCESS;

			Microsoft::WRL::ComPtr<IDXGIResource> DesktopResource;
			DXGI_OUTDUPL_FRAME_INFO FrameInfo;
			AquireFrameRAII frame(_DXFrameProcessorImpl->OutputDuplication.Get());

			// Get new frame
			auto hr = frame.AcquireNextFrame(500, &FrameInfo, DesktopResource.GetAddressOf());
			if (hr == DXGI_ERROR_WAIT_TIMEOUT)
			{
				return DUPL_RETURN_SUCCESS;
			}
			else if (FAILED(hr))
			{
				return ProcessFailure(_DXFrameProcessorImpl->Device.Get(), L"Failed to acquire next frame in DUPLICATIONMANAGER", L"Error", hr, FrameInfoExpectedErrors);
			}
			Microsoft::WRL::ComPtr<ID3D11Texture2D> aquireddesktopimage;
			// QI for IDXGIResource
			hr = DesktopResource.Get()->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void **>(aquireddesktopimage.GetAddressOf()));
			if (FAILED(hr))
			{
				return ProcessFailure(nullptr, L"Failed to QI for ID3D11Texture2D from acquired IDXGIResource in DUPLICATIONMANAGER", L"Error", hr);
			}

			D3D11_TEXTURE2D_DESC ThisDesc;
			aquireddesktopimage->GetDesc(&ThisDesc);

			if (!_DXFrameProcessorImpl->StagingSurf)
			{
				D3D11_TEXTURE2D_DESC StagingDesc;
				StagingDesc = ThisDesc;
				StagingDesc.BindFlags = 0;
				StagingDesc.Usage = D3D11_USAGE_STAGING;
				StagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
				StagingDesc.MiscFlags = 0;
				hr = _DXFrameProcessorImpl->Device->CreateTexture2D(&StagingDesc, nullptr, _DXFrameProcessorImpl->StagingSurf.GetAddressOf());
				if (FAILED(hr))
				{
					return ProcessFailure(_DXFrameProcessorImpl->Device.Get(), L"Failed to create staging texture for move rects", L"Error", hr, SystemTransitionsExpectedErrors);
				}
			}

			auto movecount = 0;
			auto dirtycount = 0;
			RECT* dirtyrects = nullptr;
			// Get metadata
			if (FrameInfo.TotalMetadataBufferSize > 0)
			{
				_DXFrameProcessorImpl->MetaDataBuffer.reserve(FrameInfo.TotalMetadataBufferSize);
				UINT bufsize = FrameInfo.TotalMetadataBufferSize;

				// Get move rectangles
				hr = _DXFrameProcessorImpl->OutputDuplication->GetFrameMoveRects(bufsize, reinterpret_cast<DXGI_OUTDUPL_MOVE_RECT*>(_DXFrameProcessorImpl->MetaDataBuffer.data()), &bufsize);
				if (FAILED(hr))
				{
					return ProcessFailure(nullptr, L"Failed to get frame move rects in DUPLICATIONMANAGER", L"Error", hr, FrameInfoExpectedErrors);
				}
				movecount = bufsize / sizeof(DXGI_OUTDUPL_MOVE_RECT);

				dirtyrects = reinterpret_cast<RECT*>(_DXFrameProcessorImpl->MetaDataBuffer.data() + bufsize);
				bufsize = FrameInfo.TotalMetadataBufferSize - bufsize;

				// Get dirty rectangles
				hr = _DXFrameProcessorImpl->OutputDuplication->GetFrameDirtyRects(bufsize, dirtyrects, &bufsize);
				if (FAILED(hr))
				{
					return ProcessFailure(nullptr, L"Failed to get frame dirty rects in DUPLICATIONMANAGER", L"Error", hr, FrameInfoExpectedErrors);
				}
				dirtycount = bufsize / sizeof(RECT);
				//convert rects to their correct coords
				for (auto i = 0; i < dirtycount; i++) {
					dirtyrects[i] = ConvertRect(dirtyrects[i], _DXFrameProcessorImpl->OutputDesc);
				}
			}
			_DXFrameProcessorImpl->DeviceContext->CopyResource(_DXFrameProcessorImpl->StagingSurf.Get(), aquireddesktopimage.Get());

			D3D11_MAPPED_SUBRESOURCE MappingDesc;
			MAPPED_SUBRESOURCERAII mappedresrouce(_DXFrameProcessorImpl->DeviceContext.Get());
			hr = mappedresrouce.Map(_DXFrameProcessorImpl->StagingSurf.Get(), 0, D3D11_MAP_READ, 0, &MappingDesc);
			// Get the data
			if (MappingDesc.pData == NULL) {
				return ProcessFailure(_DXFrameProcessorImpl->Device.Get(), L"DrawSurface_GetPixelColor: Could not read the pixel color because the mapped subresource returned NULL", L"Error", hr, SystemTransitionsExpectedErrors);
			}
			auto startsrc = (char*)MappingDesc.pData;
			ImageRect ret;
			// Process dirties 

			if (dirtycount > 0 && dirtyrects != nullptr && _DXFrameProcessorImpl->Data->CaptureDifMonitor)
			{
				for (auto i = 0; i < dirtycount; i++)
				{
					ret.left = dirtyrects[i].left;
					ret.top = dirtyrects[i].top;
					ret.bottom = dirtyrects[i].bottom;
					ret.right = dirtyrects[i].right;
					//pad is the number of bytes to advance to the next starting point of data. this is NOT the padding to the rightmost side of the image
					auto pad = MappingDesc.RowPitch - (dirtyrects[i].right *PixelStride) + (dirtyrects[i].left *PixelStride);

					auto startdata = startsrc + (MappingDesc.RowPitch*ret.top) + (PixelStride*ret.left);
					auto img = Create(ret, PixelStride, pad, startdata);
					_DXFrameProcessorImpl->Data->CaptureDifMonitor(img, _DXFrameProcessorImpl->Data->SelectedMonitor);
				}
			}
			if (_DXFrameProcessorImpl->Data->CaptureEntireMonitor) {
				ret.left = ret.top = 0;
				ret.bottom = Height(_DXFrameProcessorImpl->Data->SelectedMonitor);
				ret.right = Width(_DXFrameProcessorImpl->Data->SelectedMonitor);
				auto img = Create(ret, PixelStride, MappingDesc.RowPitch - (ret.right*PixelStride), startsrc);
				_DXFrameProcessorImpl->Data->CaptureEntireMonitor(img, _DXFrameProcessorImpl->Data->SelectedMonitor);
			}
			return Ret;
		}

	}
}

#endif

