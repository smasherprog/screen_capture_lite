#pragma once
#include "ScreenCapture.h"
#include "DXCommon.h"
#include <memory>

namespace SL {
	namespace Screen_Capture {
		class DXFrameProcessor {
		public:
			DXFrameProcessor();
			~DXFrameProcessor();
			DUPL_RETURN Init(ImageCallback& cb, UINT output);
			DUPL_RETURN ProcessFrame(bool* timedout);

		private:

			Microsoft::WRL::ComPtr<ID3D11Device> Device;
			Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeviceContext;
			Microsoft::WRL::ComPtr<ID3D11Texture2D> StagingSurf;

			Microsoft::WRL::ComPtr<IDXGIOutputDuplication> OutputDuplication;
			DXGI_OUTPUT_DESC OutputDesc;
			UINT Output;
			std::vector<BYTE> MetaDataBuffer;

			ImageCallback CallBack;

		};
	}
}