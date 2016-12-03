#pragma once
#include "CommonTypes.h"
#include <memory>
namespace SL {
	namespace Screen_Capture {

		class DXFrameProcessor {
		public:
			DXFrameProcessor();
			~DXFrameProcessor();
			void InitD3D(DX_RESOURCES* Data);
			void CleanRefs();
			DUPL_RETURN ProcessFrame( FRAME_DATA* Data,  ID3D11Texture2D* SharedSurf, INT OffsetX, INT OffsetY,  DXGI_OUTPUT_DESC* DeskDesc);
		
		private:
			// methods
			DUPL_RETURN CopyDirty( ID3D11Texture2D* SrcSurface,  ID3D11Texture2D* SharedSurf,  RECT* DirtyBuffer, UINT DirtyCount, INT OffsetX, INT OffsetY,  DXGI_OUTPUT_DESC* DeskDesc);
			DUPL_RETURN CopyMove( ID3D11Texture2D* SharedSurf,  DXGI_OUTDUPL_MOVE_RECT* MoveBuffer, UINT MoveCount, INT OffsetX, INT OffsetY,  DXGI_OUTPUT_DESC* DeskDesc, INT TexWidth, INT TexHeight);
			void SetDirtyVert(VERTEX* Vertices,  RECT* Dirty, INT OffsetX, INT OffsetY,  DXGI_OUTPUT_DESC* DeskDesc,  D3D11_TEXTURE2D_DESC* FullDesc,  D3D11_TEXTURE2D_DESC* ThisDesc);
			void SetMoveRect( RECT* SrcRect,  RECT* DestRect,  DXGI_OUTPUT_DESC* DeskDesc,  DXGI_OUTDUPL_MOVE_RECT* MoveRect, INT TexWidth, INT TexHeight);

			Microsoft::WRL::ComPtr<ID3D11Device> m_Device;
			Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_DeviceContext;
			Microsoft::WRL::ComPtr<ID3D11Texture2D> m_MoveSurf;
			Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
			Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
			Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
			Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_RTV;
			Microsoft::WRL::ComPtr<ID3D11SamplerState> m_SamplerLinear;

			std::unique_ptr<BYTE[]> m_DirtyVertexBufferAlloc;
			UINT m_DirtyVertexBufferAllocSize;
		};
	}
}