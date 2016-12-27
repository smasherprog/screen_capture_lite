#include "DXFrameProcessor.h"
#include <string>

namespace SL {
	namespace Screen_Capture {

		DXFrameProcessor::DXFrameProcessor()
		{
			m_DirtyVertexBufferAllocSize = 0;
		}

		DXFrameProcessor::~DXFrameProcessor()
		{
		}
		void DXFrameProcessor::InitD3D(THREAD_DATA * Data)
		{
			m_Device = Data->DxRes.Device;
			m_DeviceContext = Data->DxRes.DeviceContext;
			m_VertexShader = Data->DxRes.VertexShader;
			m_PixelShader = Data->DxRes.PixelShader;
			m_InputLayout = Data->DxRes.InputLayout;
			m_SamplerLinear = Data->DxRes.SamplerLinear;
			CallBack = Data->CallBack;
		}
		void DXFrameProcessor::CleanRefs()
		{
			m_DeviceContext = nullptr;
			m_Device = nullptr;
			m_MoveSurf = nullptr;
			m_CopySurf = nullptr;

			m_VertexShader = nullptr;
			m_PixelShader = nullptr;
			m_InputLayout = nullptr;
			m_SamplerLinear = nullptr;
			m_RTV = nullptr;
			CallBack = [](const CapturedImage& img) {};
		}


		//
		// Process a given frame and its metadata
		//
		DUPL_RETURN DXFrameProcessor::ProcessFrame(FRAME_DATA* Data, DXGI_OUTPUT_DESC* DeskDesc)
		{
			DUPL_RETURN Ret = DUPL_RETURN_SUCCESS;

			// Process dirties and moves
			if (Data->FrameInfo.TotalMetadataBufferSize)
			{
				if (Data->MoveCount)
				{
					Ret = CopyMove(Data->Frame.Get(), reinterpret_cast<DXGI_OUTDUPL_MOVE_RECT*>(Data->MetaData.get()), Data->MoveCount, DeskDesc);
					if (FAILED(Ret))
					{
						return Ret;
					}
				}

				if (Data->DirtyCount)
				{
					auto dirtyrects = reinterpret_cast<RECT*>(Data->MetaData.get() + (Data->MoveCount * sizeof(DXGI_OUTDUPL_MOVE_RECT)));
			
					Ret = CopyDirty(Data->Frame.Get(), dirtyrects, Data->DirtyCount, DeskDesc);

					D3D11_TEXTURE2D_DESC ThisDesc;
					Data->Frame->GetDesc(&ThisDesc);
					HRESULT hr;
					if (!m_StagingSurf)
					{
						D3D11_TEXTURE2D_DESC StagingDesc;
						StagingDesc = ThisDesc;
						StagingDesc.BindFlags = 0;
						StagingDesc.Usage = D3D11_USAGE_STAGING;
						StagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
						StagingDesc.MiscFlags = 0;
						hr = m_Device->CreateTexture2D(&StagingDesc, nullptr, m_StagingSurf.GetAddressOf());
						if (FAILED(hr))
						{
							return ProcessFailure(m_Device.Get(), L"Failed to create staging texture for move rects", L"Error", hr, SystemTransitionsExpectedErrors);
						}
					}
			
					for (UINT i = 0; i < Data->DirtyCount; ++i)
					{
						D3D11_BOX Box;
						Box.left = dirtyrects[i].left;
						Box.top = dirtyrects[i].top;
						Box.front = 0;
						Box.right = dirtyrects[i].right;
						Box.bottom = dirtyrects[i].bottom;
						Box.back = 1;
						//m_DeviceContext->CopySubresourceRegion(m_StagingSurf.Get(), 0, dirtyrects[i].left, dirtyrects[i].top, 0, m_CopySurf.Get(), 0, &Box);
						m_DeviceContext->CopyResource(m_StagingSurf.Get(), m_CopySurf.Get());
						D3D11_MAPPED_SUBRESOURCE MappingDesc;
						hr = m_DeviceContext->Map(m_StagingSurf.Get(), 0, D3D11_MAP_READ, 0, &MappingDesc);

						// Get the data
						if (MappingDesc.pData == NULL) {
							return ProcessFailure(m_Device.Get(), L"DrawSurface_GetPixelColor: Could not read the pixel color because the mapped subresource returned NULL", L"Error", hr, SystemTransitionsExpectedErrors);
						}
						else {
						
							CapturedImage img;
							img.ScreenIndex = Data->SrcreenIndex;
							img.Height = (Box.bottom - Box.top);
							img.Width = (Box.right - Box.left);
							img.RelativeTop = Box.top;
							img.RelativeLeft = Box.left;
							img.AbsoluteLeft = DeskDesc->DesktopCoordinates.left + Box.left;
							img.AbsoluteTop = DeskDesc->DesktopCoordinates.top + Box.top;
							auto sizeneeded = 4 * img.Height *img.Width;
							img.Data = std::shared_ptr<char>(new char[sizeneeded], [](char* ptr) { if (ptr) delete[] ptr; });
							auto dststart = img.Data.get();
							auto srcstart = ((char*)MappingDesc.pData) + (Box.top * MappingDesc.RowPitch) + (Box.left * img.PixelStride);

							for (auto t = Box.top; t < Box.bottom; t++) {
								memcpy(dststart, srcstart, img.Width * img.PixelStride);
								dststart += img.Width * img.PixelStride;
								srcstart += MappingDesc.RowPitch;
							}
							CallBack(img);
						}
						// Unlock the memory
						m_DeviceContext->Unmap(m_StagingSurf.Get(), 0);

					}



				}
			}

			return Ret;
		}

		//
		// Set appropriate source and destination rects for move rects
		//
		void DXFrameProcessor::SetMoveRect(RECT* SrcRect, RECT* DestRect, DXGI_OUTPUT_DESC* DeskDesc, DXGI_OUTDUPL_MOVE_RECT* MoveRect, INT TexWidth, INT TexHeight)
		{
			switch (DeskDesc->Rotation)
			{
			case DXGI_MODE_ROTATION_UNSPECIFIED:
			case DXGI_MODE_ROTATION_IDENTITY:
			{
				SrcRect->left = MoveRect->SourcePoint.x;
				SrcRect->top = MoveRect->SourcePoint.y;
				SrcRect->right = MoveRect->SourcePoint.x + MoveRect->DestinationRect.right - MoveRect->DestinationRect.left;
				SrcRect->bottom = MoveRect->SourcePoint.y + MoveRect->DestinationRect.bottom - MoveRect->DestinationRect.top;

				*DestRect = MoveRect->DestinationRect;
				break;
			}
			case DXGI_MODE_ROTATION_ROTATE90:
			{
				SrcRect->left = TexHeight - (MoveRect->SourcePoint.y + MoveRect->DestinationRect.bottom - MoveRect->DestinationRect.top);
				SrcRect->top = MoveRect->SourcePoint.x;
				SrcRect->right = TexHeight - MoveRect->SourcePoint.y;
				SrcRect->bottom = MoveRect->SourcePoint.x + MoveRect->DestinationRect.right - MoveRect->DestinationRect.left;

				DestRect->left = TexHeight - MoveRect->DestinationRect.bottom;
				DestRect->top = MoveRect->DestinationRect.left;
				DestRect->right = TexHeight - MoveRect->DestinationRect.top;
				DestRect->bottom = MoveRect->DestinationRect.right;
				break;
			}
			case DXGI_MODE_ROTATION_ROTATE180:
			{
				SrcRect->left = TexWidth - (MoveRect->SourcePoint.x + MoveRect->DestinationRect.right - MoveRect->DestinationRect.left);
				SrcRect->top = TexHeight - (MoveRect->SourcePoint.y + MoveRect->DestinationRect.bottom - MoveRect->DestinationRect.top);
				SrcRect->right = TexWidth - MoveRect->SourcePoint.x;
				SrcRect->bottom = TexHeight - MoveRect->SourcePoint.y;

				DestRect->left = TexWidth - MoveRect->DestinationRect.right;
				DestRect->top = TexHeight - MoveRect->DestinationRect.bottom;
				DestRect->right = TexWidth - MoveRect->DestinationRect.left;
				DestRect->bottom = TexHeight - MoveRect->DestinationRect.top;
				break;
			}
			case DXGI_MODE_ROTATION_ROTATE270:
			{
				SrcRect->left = MoveRect->SourcePoint.x;
				SrcRect->top = TexWidth - (MoveRect->SourcePoint.x + MoveRect->DestinationRect.right - MoveRect->DestinationRect.left);
				SrcRect->right = MoveRect->SourcePoint.y + MoveRect->DestinationRect.bottom - MoveRect->DestinationRect.top;
				SrcRect->bottom = TexWidth - MoveRect->SourcePoint.x;

				DestRect->left = MoveRect->DestinationRect.top;
				DestRect->top = TexWidth - MoveRect->DestinationRect.right;
				DestRect->right = MoveRect->DestinationRect.bottom;
				DestRect->bottom = TexWidth - MoveRect->DestinationRect.left;
				break;
			}
			default:
			{
				RtlZeroMemory(DestRect, sizeof(RECT));
				RtlZeroMemory(SrcRect, sizeof(RECT));
				break;
			}
			}
		}

		//
		// Copy move rectangles
		//
		DUPL_RETURN DXFrameProcessor::CopyMove(ID3D11Texture2D* SrcSurface, DXGI_OUTDUPL_MOVE_RECT* MoveBuffer, UINT MoveCount, DXGI_OUTPUT_DESC* DeskDesc)
		{

			D3D11_TEXTURE2D_DESC ThisDesc;
			SrcSurface->GetDesc(&ThisDesc);

			// Make new intermediate surface to copy into for moving
			if (!m_MoveSurf)
			{

				D3D11_TEXTURE2D_DESC MoveDesc;
				MoveDesc = ThisDesc;
				MoveDesc.Width = DeskDesc->DesktopCoordinates.right - DeskDesc->DesktopCoordinates.left;
				MoveDesc.Height = DeskDesc->DesktopCoordinates.bottom - DeskDesc->DesktopCoordinates.top;
				MoveDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
				MoveDesc.MiscFlags = 0;
				HRESULT hr = m_Device->CreateTexture2D(&MoveDesc, nullptr, m_MoveSurf.GetAddressOf());
				if (FAILED(hr))
				{
					return ProcessFailure(m_Device.Get(), L"Failed to create staging texture for move rects", L"Error", hr, SystemTransitionsExpectedErrors);
				}
			}

			for (UINT i = 0; i < MoveCount; ++i)
			{
				RECT SrcRect;
				RECT DestRect;

				SetMoveRect(&SrcRect, &DestRect, DeskDesc, &(MoveBuffer[i]), ThisDesc.Width, ThisDesc.Height);

				// Copy rect out of shared surface
				D3D11_BOX Box;
				Box.left = SrcRect.left + DeskDesc->DesktopCoordinates.left;
				Box.top = SrcRect.top + DeskDesc->DesktopCoordinates.top;
				Box.front = 0;
				Box.right = SrcRect.right + DeskDesc->DesktopCoordinates.left;
				Box.bottom = SrcRect.bottom + DeskDesc->DesktopCoordinates.top;
				Box.back = 1;
				m_DeviceContext->CopySubresourceRegion(m_MoveSurf.Get(), 0, SrcRect.left, SrcRect.top, 0, SrcSurface, 0, &Box);

				// Copy back to shared surface
				Box.left = SrcRect.left;
				Box.top = SrcRect.top;
				Box.front = 0;
				Box.right = SrcRect.right;
				Box.bottom = SrcRect.bottom;
				Box.back = 1;
				m_DeviceContext->CopySubresourceRegion(SrcSurface, 0, DestRect.left + DeskDesc->DesktopCoordinates.left, DestRect.top + DeskDesc->DesktopCoordinates.top, 0, m_MoveSurf.Get(), 0, &Box);
			}


			return DUPL_RETURN_SUCCESS;
		}

		//
		// Sets up vertices for dirty rects for rotated desktops
		//

		void DXFrameProcessor::SetDirtyVert(VERTEX* Vertices, RECT* Dirty, DXGI_OUTPUT_DESC* DeskDesc, D3D11_TEXTURE2D_DESC* ThisDesc)
		{
			INT CenterX = ThisDesc->Width / 2;
			INT CenterY = ThisDesc->Height / 2;

			INT Width = DeskDesc->DesktopCoordinates.right - DeskDesc->DesktopCoordinates.left;
			INT Height = DeskDesc->DesktopCoordinates.bottom - DeskDesc->DesktopCoordinates.top;

			// Rotation compensated destination rect
			RECT DestDirty = *Dirty;

			// Set appropriate coordinates compensated for rotation
			switch (DeskDesc->Rotation)
			{
			case DXGI_MODE_ROTATION_ROTATE90:
			{
				DestDirty.left = Width - Dirty->bottom;
				DestDirty.top = Dirty->left;
				DestDirty.right = Width - Dirty->top;
				DestDirty.bottom = Dirty->right;

				Vertices[0].TexCoord = XMFLOAT2(Dirty->right / static_cast<FLOAT>(ThisDesc->Width), Dirty->bottom / static_cast<FLOAT>(ThisDesc->Height));
				Vertices[1].TexCoord = XMFLOAT2(Dirty->left / static_cast<FLOAT>(ThisDesc->Width), Dirty->bottom / static_cast<FLOAT>(ThisDesc->Height));
				Vertices[2].TexCoord = XMFLOAT2(Dirty->right / static_cast<FLOAT>(ThisDesc->Width), Dirty->top / static_cast<FLOAT>(ThisDesc->Height));
				Vertices[5].TexCoord = XMFLOAT2(Dirty->left / static_cast<FLOAT>(ThisDesc->Width), Dirty->top / static_cast<FLOAT>(ThisDesc->Height));
				break;
			}
			case DXGI_MODE_ROTATION_ROTATE180:
			{
				DestDirty.left = Width - Dirty->right;
				DestDirty.top = Height - Dirty->bottom;
				DestDirty.right = Width - Dirty->left;
				DestDirty.bottom = Height - Dirty->top;

				Vertices[0].TexCoord = XMFLOAT2(Dirty->right / static_cast<FLOAT>(ThisDesc->Width), Dirty->top / static_cast<FLOAT>(ThisDesc->Height));
				Vertices[1].TexCoord = XMFLOAT2(Dirty->right / static_cast<FLOAT>(ThisDesc->Width), Dirty->bottom / static_cast<FLOAT>(ThisDesc->Height));
				Vertices[2].TexCoord = XMFLOAT2(Dirty->left / static_cast<FLOAT>(ThisDesc->Width), Dirty->top / static_cast<FLOAT>(ThisDesc->Height));
				Vertices[5].TexCoord = XMFLOAT2(Dirty->left / static_cast<FLOAT>(ThisDesc->Width), Dirty->bottom / static_cast<FLOAT>(ThisDesc->Height));
				break;
			}
			case DXGI_MODE_ROTATION_ROTATE270:
			{
				DestDirty.left = Dirty->top;
				DestDirty.top = Height - Dirty->right;
				DestDirty.right = Dirty->bottom;
				DestDirty.bottom = Height - Dirty->left;

				Vertices[0].TexCoord = XMFLOAT2(Dirty->left / static_cast<FLOAT>(ThisDesc->Width), Dirty->top / static_cast<FLOAT>(ThisDesc->Height));
				Vertices[1].TexCoord = XMFLOAT2(Dirty->right / static_cast<FLOAT>(ThisDesc->Width), Dirty->top / static_cast<FLOAT>(ThisDesc->Height));
				Vertices[2].TexCoord = XMFLOAT2(Dirty->left / static_cast<FLOAT>(ThisDesc->Width), Dirty->bottom / static_cast<FLOAT>(ThisDesc->Height));
				Vertices[5].TexCoord = XMFLOAT2(Dirty->right / static_cast<FLOAT>(ThisDesc->Width), Dirty->bottom / static_cast<FLOAT>(ThisDesc->Height));
				break;
			}
			default:
				assert(false); // drop through
			case DXGI_MODE_ROTATION_UNSPECIFIED:
			case DXGI_MODE_ROTATION_IDENTITY:
			{
				Vertices[0].TexCoord = XMFLOAT2(Dirty->left / static_cast<FLOAT>(ThisDesc->Width), Dirty->bottom / static_cast<FLOAT>(ThisDesc->Height));
				Vertices[1].TexCoord = XMFLOAT2(Dirty->left / static_cast<FLOAT>(ThisDesc->Width), Dirty->top / static_cast<FLOAT>(ThisDesc->Height));
				Vertices[2].TexCoord = XMFLOAT2(Dirty->right / static_cast<FLOAT>(ThisDesc->Width), Dirty->bottom / static_cast<FLOAT>(ThisDesc->Height));
				Vertices[5].TexCoord = XMFLOAT2(Dirty->right / static_cast<FLOAT>(ThisDesc->Width), Dirty->top / static_cast<FLOAT>(ThisDesc->Height));
				break;
			}
			}

			// Set positions
			Vertices[0].Pos = XMFLOAT3((DestDirty.left- CenterX) / static_cast<FLOAT>(CenterX),
				-1 * (DestDirty.bottom  - CenterY) / static_cast<FLOAT>(CenterY),
				0.0f);
			Vertices[1].Pos = XMFLOAT3((DestDirty.left  - CenterX) / static_cast<FLOAT>(CenterX),
				-1 * (DestDirty.top - CenterY) / static_cast<FLOAT>(CenterY),
				0.0f);
			Vertices[2].Pos = XMFLOAT3((DestDirty.right- CenterX) / static_cast<FLOAT>(CenterX),
				-1 * (DestDirty.bottom  - CenterY) / static_cast<FLOAT>(CenterY),
				0.0f);
			Vertices[3].Pos = Vertices[2].Pos;
			Vertices[4].Pos = Vertices[1].Pos;
			Vertices[5].Pos = XMFLOAT3((DestDirty.right  - CenterX) / static_cast<FLOAT>(CenterX),
				-1 * (DestDirty.top - CenterY) / static_cast<FLOAT>(CenterY),
				0.0f);

			Vertices[3].TexCoord = Vertices[2].TexCoord;
			Vertices[4].TexCoord = Vertices[1].TexCoord;
		}

		//
		// Copies dirty rectangles
		//


		DUPL_RETURN DXFrameProcessor::CopyDirty(ID3D11Texture2D* SrcSurface, RECT* DirtyBuffer, UINT DirtyCount, DXGI_OUTPUT_DESC* DeskDesc)
		{
			HRESULT hr;
			D3D11_TEXTURE2D_DESC ThisDesc;
			SrcSurface->GetDesc(&ThisDesc);

			if (!m_CopySurf)
			{
				D3D11_TEXTURE2D_DESC CopyDesc;
				CopyDesc = ThisDesc;
				CopyDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
				CopyDesc.MiscFlags = 0;
				HRESULT hr = m_Device->CreateTexture2D(&CopyDesc, nullptr, m_CopySurf.GetAddressOf());
				if (FAILED(hr))
				{
					return ProcessFailure(m_Device.Get(), L"Failed to create staging texture for move rects", L"Error", hr, SystemTransitionsExpectedErrors);
				}
			}

			if (!m_RTV)
			{
				hr = m_Device->CreateRenderTargetView(m_CopySurf.Get(), nullptr, m_RTV.GetAddressOf());
				if (FAILED(hr))
				{
					return ProcessFailure(m_Device.Get(), L"Failed to create render target view for dirty rects", L"Error", hr, SystemTransitionsExpectedErrors);
				}
			}

			D3D11_SHADER_RESOURCE_VIEW_DESC ShaderDesc;
			ShaderDesc.Format = ThisDesc.Format;
			ShaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			ShaderDesc.Texture2D.MostDetailedMip = ThisDesc.MipLevels - 1;
			ShaderDesc.Texture2D.MipLevels = ThisDesc.MipLevels;

			// Create new shader resource view
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ShaderResource;
		
			hr = m_Device->CreateShaderResourceView(SrcSurface, &ShaderDesc, ShaderResource.GetAddressOf());

			if (FAILED(hr))
			{
				return ProcessFailure(m_Device.Get(), L"Failed to create shader resource view for dirty rects", L"Error", hr, SystemTransitionsExpectedErrors);
			}

			FLOAT BlendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
			m_DeviceContext->OMSetBlendState(nullptr, BlendFactor, 0xFFFFFFFF);
			m_DeviceContext->OMSetRenderTargets(1, m_RTV.GetAddressOf(), nullptr);
			m_DeviceContext->VSSetShader(m_VertexShader.Get(), nullptr, 0);
			m_DeviceContext->PSSetShader(m_PixelShader.Get(), nullptr, 0);
			m_DeviceContext->PSSetShaderResources(0, 1, ShaderResource.GetAddressOf());
			m_DeviceContext->PSSetSamplers(0, 1, m_SamplerLinear.GetAddressOf());
			m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// Create space for vertices for the dirty rects if the current space isn't large enough
			UINT BytesNeeded = sizeof(VERTEX) * 6 * DirtyCount;
			if (BytesNeeded > m_DirtyVertexBufferAllocSize)
			{
				m_DirtyVertexBufferAlloc = std::make_unique<BYTE[]>(BytesNeeded);
				if (!m_DirtyVertexBufferAlloc)
				{
					m_DirtyVertexBufferAllocSize = 0;
					return ProcessFailure(nullptr, L"Failed to allocate memory for dirty vertex buffer.", L"Error", E_OUTOFMEMORY);
				}

				m_DirtyVertexBufferAllocSize = BytesNeeded;
			}

			// Fill them in
			VERTEX* DirtyVertex = reinterpret_cast<VERTEX*>(m_DirtyVertexBufferAlloc.get());
			auto str = std::to_string(DirtyCount);
			OutputDebugStringA(str.c_str());

			for (UINT i = 0; i < DirtyCount; ++i, DirtyVertex += 6)
			{
				SetDirtyVert(DirtyVertex, &(DirtyBuffer[i]), DeskDesc, &ThisDesc);
			}

			// Create vertex buffer
			D3D11_BUFFER_DESC BufferDesc;
			RtlZeroMemory(&BufferDesc, sizeof(BufferDesc));
			BufferDesc.Usage = D3D11_USAGE_DEFAULT;
			BufferDesc.ByteWidth = BytesNeeded;
			BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			BufferDesc.CPUAccessFlags = 0;
			D3D11_SUBRESOURCE_DATA InitData;
			RtlZeroMemory(&InitData, sizeof(InitData));
			InitData.pSysMem = m_DirtyVertexBufferAlloc.get();

			Microsoft::WRL::ComPtr<ID3D11Buffer> VertBuf;
			hr = m_Device->CreateBuffer(&BufferDesc, &InitData, VertBuf.GetAddressOf());
			if (FAILED(hr))
			{
				return ProcessFailure(m_Device.Get(), L"Failed to create vertex buffer in dirty rect processing", L"Error", hr, SystemTransitionsExpectedErrors);
			}
			UINT Stride = sizeof(VERTEX);
			UINT Offset = 0;
			m_DeviceContext->IASetVertexBuffers(0, 1, VertBuf.GetAddressOf(), &Stride, &Offset);

			D3D11_VIEWPORT VP;
			VP.Width = static_cast<FLOAT>(ThisDesc.Width);
			VP.Height = static_cast<FLOAT>(ThisDesc.Height);
			VP.MinDepth = 0.0f;
			VP.MaxDepth = 1.0f;
			VP.TopLeftX = 0.0f;
			VP.TopLeftY = 0.0f;
			m_DeviceContext->RSSetViewports(1, &VP);

			m_DeviceContext->Draw(6 * DirtyCount, 0);


			return DUPL_RETURN_SUCCESS;
		}
	}
}