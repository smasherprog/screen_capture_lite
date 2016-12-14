// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once
#include "CommonTypes.h"
#include <memory>
namespace SL {
	namespace Screen_Capture {

		class DXDuplicationManager {
		public:
			DXDuplicationManager();
			~DXDuplicationManager();
			DUPL_RETURN GetFrame(FRAME_DATA* Data, bool* Timeout);
			DUPL_RETURN DoneWithFrame();
			DUPL_RETURN InitDupl(ID3D11Device* Device, UINT Output);
			DUPL_RETURN GetMouse(PTR_INFO* PtrInfo, DXGI_OUTDUPL_FRAME_INFO* FrameInfo);
			void GetOutputDesc(DXGI_OUTPUT_DESC* DescPtr);

		private:

			Microsoft::WRL::ComPtr<IDXGIOutputDuplication> m_DeskDupl;
			Microsoft::WRL::ComPtr<ID3D11Texture2D> m_AcquiredDesktopImage;
			std::shared_ptr<BYTE> m_MetaDataBuffer;
			UINT m_MetaDataSize;
			UINT m_OutputNumber;
			DXGI_OUTPUT_DESC m_OutputDesc;
			Microsoft::WRL::ComPtr<ID3D11Device> m_Device;
		};
	}
}