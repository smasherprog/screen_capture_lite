// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "DXDuplicationManager.h"


namespace SL {
	namespace Screen_Capture {

		DXDuplicationManager::DXDuplicationManager()
		{
			m_OutputNumber = 0;
			m_MetaDataSize = 0;
			m_OutputDesc = { 0 };
		}

		DXDuplicationManager::~DXDuplicationManager()
		{
		}

		DUPL_RETURN DXDuplicationManager::GetFrame(FRAME_DATA * Data, bool * Timeout)
		{

			Microsoft::WRL::ComPtr<IDXGIResource> DesktopResource;
			DXGI_OUTDUPL_FRAME_INFO FrameInfo;

			// Get new frame
			HRESULT hr = m_DeskDupl->AcquireNextFrame(500, &FrameInfo, DesktopResource.GetAddressOf());
			if (hr == DXGI_ERROR_WAIT_TIMEOUT)
			{
				*Timeout = true;
				return DUPL_RETURN_SUCCESS;
			}
			*Timeout = false;

			if (FAILED(hr))
			{
				return ProcessFailure(m_Device.Get(), L"Failed to acquire next frame in DUPLICATIONMANAGER", L"Error", hr, FrameInfoExpectedErrors);
			}
			m_AcquiredDesktopImage = nullptr;
			// QI for IDXGIResource
			hr = DesktopResource.Get()->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void **>(m_AcquiredDesktopImage.GetAddressOf()));
			if (FAILED(hr))
			{
				return ProcessFailure(nullptr, L"Failed to QI for ID3D11Texture2D from acquired IDXGIResource in DUPLICATIONMANAGER", L"Error", hr);
			}
	
			// Get metadata
			if (FrameInfo.TotalMetadataBufferSize)
			{
				// Old buffer too small
				if (FrameInfo.TotalMetadataBufferSize > m_MetaDataSize)
				{

					m_MetaDataBuffer = std::shared_ptr<BYTE>(new BYTE[FrameInfo.TotalMetadataBufferSize], [](BYTE* ptr) { delete[] ptr; });
					if (!m_MetaDataBuffer)
					{
						m_MetaDataSize = 0;
						Data->MoveCount = 0;
						Data->DirtyCount = 0;
						return ProcessFailure(nullptr, L"Failed to allocate memory for metadata in DUPLICATIONMANAGER", L"Error", E_OUTOFMEMORY);
					}
					m_MetaDataSize = FrameInfo.TotalMetadataBufferSize;
				}

				UINT BufSize = FrameInfo.TotalMetadataBufferSize;

				// Get move rectangles
				hr = m_DeskDupl->GetFrameMoveRects(BufSize, reinterpret_cast<DXGI_OUTDUPL_MOVE_RECT*>(m_MetaDataBuffer.get()), &BufSize);
				if (FAILED(hr))
				{
					Data->MoveCount = 0;
					Data->DirtyCount = 0;
					return ProcessFailure(nullptr, L"Failed to get frame move rects in DUPLICATIONMANAGER", L"Error", hr, FrameInfoExpectedErrors);
				}
				Data->MoveCount = BufSize / sizeof(DXGI_OUTDUPL_MOVE_RECT);

				BYTE* DirtyRects = m_MetaDataBuffer.get() + BufSize;
				BufSize = FrameInfo.TotalMetadataBufferSize - BufSize;

				// Get dirty rectangles
				hr = m_DeskDupl->GetFrameDirtyRects(BufSize, reinterpret_cast<RECT*>(DirtyRects), &BufSize);
				if (FAILED(hr))
				{
					Data->MoveCount = 0;
					Data->DirtyCount = 0;
					return ProcessFailure(nullptr, L"Failed to get frame dirty rects in DUPLICATIONMANAGER", L"Error", hr, FrameInfoExpectedErrors);
				}
				Data->DirtyCount = BufSize / sizeof(RECT);

				Data->MetaData = m_MetaDataBuffer;
			}
			Data->Frame = m_AcquiredDesktopImage;
			Data->FrameInfo = FrameInfo;

			return DUPL_RETURN_SUCCESS;
		}

		DUPL_RETURN DXDuplicationManager::DoneWithFrame()
		{

			HRESULT hr = m_DeskDupl->ReleaseFrame();
			if (FAILED(hr))
			{
				return ProcessFailure(m_Device.Get(), L"Failed to release frame in DUPLICATIONMANAGER", L"Error", hr, FrameInfoExpectedErrors);
			}
			m_AcquiredDesktopImage = nullptr;
			return DUPL_RETURN_SUCCESS;

		}
		DUPL_RETURN DXDuplicationManager::InitDupl(ID3D11Device * Device, UINT Output)
		{

			m_Device = Device;

			// Get DXGI device
			Microsoft::WRL::ComPtr<IDXGIDevice> DxgiDevice;
			HRESULT hr = Device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(DxgiDevice.GetAddressOf()));
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
			hr = DxgiAdapter->EnumOutputs(Output, DxgiOutput.GetAddressOf());
		
			if (FAILED(hr))
			{
				return ProcessFailure(m_Device.Get(), L"Failed to get specified output in DUPLICATIONMANAGER", L"Error", hr, EnumOutputsExpectedErrors);
			}

			DxgiOutput->GetDesc(&m_OutputDesc);

			// QI for Output 1
			Microsoft::WRL::ComPtr<IDXGIOutput1> DxgiOutput1;
			hr = DxgiOutput.Get()->QueryInterface(__uuidof(IDXGIOutput1), reinterpret_cast<void**>(DxgiOutput1.GetAddressOf()));
			if (FAILED(hr))
			{
				return ProcessFailure(nullptr, L"Failed to QI for DxgiOutput1 in DUPLICATIONMANAGER", L"Error", hr);
			}

			// Create desktop duplication
			hr = DxgiOutput1->DuplicateOutput(Device, m_DeskDupl.GetAddressOf());
			if (FAILED(hr))
			{
				return ProcessFailure(m_Device.Get(), L"Failed to get duplicate output in DUPLICATIONMANAGER", L"Error", hr, CreateDuplicationExpectedErrors);
			}

			return DUPL_RETURN_SUCCESS;
		}

		void DXDuplicationManager::GetOutputDesc(DXGI_OUTPUT_DESC * DescPtr)
		{
			*DescPtr = m_OutputDesc;
		}

	}
}