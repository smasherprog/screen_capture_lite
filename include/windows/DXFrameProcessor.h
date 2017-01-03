#pragma once
#include "DXCommon.h"
#include "ThreadRunner.h"
#include <memory>

namespace SL {
	namespace Screen_Capture {
		class DXFrameProcessor {
		public:
			DXFrameProcessor();
			~DXFrameProcessor();
			DUPL_RETURN Init(std::shared_ptr<THREAD_DATA> data);
			DUPL_RETURN ProcessFrame();

		private:

			Microsoft::WRL::ComPtr<ID3D11Device> Device;
			Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeviceContext;
			Microsoft::WRL::ComPtr<ID3D11Texture2D> StagingSurf;

			Microsoft::WRL::ComPtr<IDXGIOutputDuplication> OutputDuplication;
			DXGI_OUTPUT_DESC OutputDesc;
			UINT Output;
			std::vector<BYTE> MetaDataBuffer;

			std::shared_ptr<THREAD_DATA> Data;
			std::unique_ptr<char[]> ImageBuffer;
			size_t ImageBufferSize;
		};
	}
}