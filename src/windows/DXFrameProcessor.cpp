#include "DXFrameProcessor.h"
#include "DXCommon.h"
#include <string>

namespace SL {
	namespace Screen_Capture {
		class AquireFrameRAII {
			IDXGIOutputDuplication* _DuplLock;
		public:
			AquireFrameRAII(IDXGIOutputDuplication* dupl): _DuplLock(dupl){	}

			~AquireFrameRAII() {
				auto hr = _DuplLock->ReleaseFrame();
				if (FAILED(hr))
				{
					ProcessFailure(nullptr, L"Failed to release frame in DUPLICATIONMANAGER", L"Error", hr, FrameInfoExpectedErrors);
				}
			}
			HRESULT AcquireNextFrame(UINT TimeoutInMilliseconds, DXGI_OUTDUPL_FRAME_INFO *pFrameInfo, IDXGIResource **ppDesktopResource) {
				return _DuplLock->AcquireNextFrame(TimeoutInMilliseconds, pFrameInfo, ppDesktopResource);
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
			HRESULT Map(ID3D11Resource *pResource,UINT Subresource,D3D11_MAP MapType, UINT MapFlags, D3D11_MAPPED_SUBRESOURCE *pMappedResource) {
				if (_Resource != nullptr) {
					_Context->Unmap(_Resource, _Subresource);
				}
				_Resource = pResource;
				_Subresource = Subresource;
				return _Context->Map(_Resource, _Subresource, MapType, MapFlags, pMappedResource);
			}
		};



		DXFrameProcessor::DXFrameProcessor()
		{

		}

		DXFrameProcessor::~DXFrameProcessor()
		{

		}
		DUPL_RETURN DXFrameProcessor::Init(ImageCallback& cb, Monitor monitor){
			DX_RESOURCES data;
			auto ret = Initialize(data);
			if (ret != DUPL_RETURN_SUCCESS) {
				return ret;
			}
			DUPLE_RESOURCES dupl;
			ret = Initialize(dupl, data.Device.Get(), monitor.Index);
			if (ret != DUPL_RETURN_SUCCESS) {
				return ret;
			}
			Device = data.Device;
			DeviceContext = data.DeviceContext;
			OutputDuplication = dupl.OutputDuplication;
			OutputDesc = dupl.OutputDesc;
			Output = dupl.Output;
			CallBack = cb;
			CurrentMonitor = monitor;
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
			AquireFrameRAII frame(OutputDuplication.Get());

			// Get new frame
			auto hr = frame.AcquireNextFrame(500, &FrameInfo, DesktopResource.GetAddressOf());
			if (hr == DXGI_ERROR_WAIT_TIMEOUT)
			{
				return DUPL_RETURN_SUCCESS;
			}else if (FAILED(hr))
			{
				return ProcessFailure(Device.Get(), L"Failed to acquire next frame in DUPLICATIONMANAGER", L"Error", hr, FrameInfoExpectedErrors);
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
		
			if (!StagingSurf)
			{
				D3D11_TEXTURE2D_DESC StagingDesc;
				StagingDesc = ThisDesc;
				StagingDesc.BindFlags = 0;
				StagingDesc.Usage = D3D11_USAGE_STAGING;
				StagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
				StagingDesc.MiscFlags = 0;
				hr = Device->CreateTexture2D(&StagingDesc, nullptr, StagingSurf.GetAddressOf());
				if (FAILED(hr))
				{
					return ProcessFailure(Device.Get(), L"Failed to create staging texture for move rects", L"Error", hr, SystemTransitionsExpectedErrors);
				}
			}

			auto movecount = 0;
			auto dirtycount = 0;
			RECT* dirtyrects = nullptr;
			// Get metadata
			if (FrameInfo.TotalMetadataBufferSize >0 )
			{
				MetaDataBuffer.reserve(FrameInfo.TotalMetadataBufferSize);
				UINT bufsize = FrameInfo.TotalMetadataBufferSize;

				// Get move rectangles
				hr = OutputDuplication->GetFrameMoveRects(bufsize, reinterpret_cast<DXGI_OUTDUPL_MOVE_RECT*>(MetaDataBuffer.data()), &bufsize);
				if (FAILED(hr))
				{
					return ProcessFailure(nullptr, L"Failed to get frame move rects in DUPLICATIONMANAGER", L"Error", hr, FrameInfoExpectedErrors);
				}
				movecount = bufsize / sizeof(DXGI_OUTDUPL_MOVE_RECT);

				dirtyrects = reinterpret_cast<RECT*>(MetaDataBuffer.data() + bufsize);
				bufsize = FrameInfo.TotalMetadataBufferSize - bufsize;

				// Get dirty rectangles
				hr = OutputDuplication->GetFrameDirtyRects(bufsize, dirtyrects, &bufsize);
				if (FAILED(hr))
				{
					return ProcessFailure(nullptr, L"Failed to get frame dirty rects in DUPLICATIONMANAGER", L"Error", hr, FrameInfoExpectedErrors);
				}
				dirtycount = bufsize / sizeof(RECT);
				//convert rects to their correct coords
				for (auto i = 0; i < dirtycount; i++) {
					dirtyrects[i]=ConvertRect(dirtyrects[i], OutputDesc);
				}
			}

			// Process dirties 
			if (dirtycount > 0 && dirtyrects != nullptr)
			{
			
				DeviceContext->CopyResource(StagingSurf.Get(), aquireddesktopimage.Get());
				D3D11_MAPPED_SUBRESOURCE MappingDesc;
				MAPPED_SUBRESOURCERAII mappedresrouce(DeviceContext.Get());

				hr = mappedresrouce.Map(StagingSurf.Get(), 0, D3D11_MAP_READ, 0, &MappingDesc);
				// Get the data
				if (MappingDesc.pData == NULL) {
					return ProcessFailure(Device.Get(), L"DrawSurface_GetPixelColor: Could not read the pixel color because the mapped subresource returned NULL", L"Error", hr, SystemTransitionsExpectedErrors);
				}

				for (auto i = 0; i < dirtycount; i++) 
				{
					CapturedImage img;
					img.Height = (dirtyrects[i].bottom - dirtyrects[i].top);
					img.Width = (dirtyrects[i].right - dirtyrects[i].left);
					img.OffsetY = dirtyrects[i].top;
					img.Offsetx = dirtyrects[i].left;
					auto sizeneeded = img.PixelStride * img.Height *img.Width;
					img.Data = std::shared_ptr<char>(new char[sizeneeded], [](char* ptr) { if (ptr) delete[] ptr; });
					auto dststart = img.Data.get();
					auto srcstart = ((char*)MappingDesc.pData) + (dirtyrects[i].top * MappingDesc.RowPitch) + (dirtyrects[i].left * img.PixelStride);

					for (auto t = dirtyrects[i].top; t <dirtyrects[i].bottom; t++) {
						memcpy(dststart, srcstart, img.Width * img.PixelStride);
						dststart += img.Width * img.PixelStride;
						srcstart += MappingDesc.RowPitch;
					}
					CallBack(img, CurrentMonitor);
				}
		
			}

			return Ret;
		}

	}
}