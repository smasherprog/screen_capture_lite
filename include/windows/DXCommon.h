// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once
#include <memory>
#include <atomic>
#include <mutex>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl.h>

#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3d11.lib")

#include "ScreenCapture.h"

namespace SL {
	namespace Screen_Capture {

		extern HRESULT SystemTransitionsExpectedErrors[];
		extern HRESULT CreateDuplicationExpectedErrors[];
		extern HRESULT FrameInfoExpectedErrors[];
		extern HRESULT EnumOutputsExpectedErrors[];

		struct DX_RESOURCES
		{
			Microsoft::WRL::ComPtr<ID3D11Device> Device;
			Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeviceContext;
		} ;
		struct DUPLE_RESOURCES
		{
			Microsoft::WRL::ComPtr<IDXGIOutputDuplication> OutputDuplication;
			DXGI_OUTPUT_DESC OutputDesc;
			UINT Output;
		};
	
		DUPL_RETURN ProcessFailure(ID3D11Device* Device, LPCWSTR Str, LPCWSTR Title, HRESULT hr, HRESULT* ExpectedErrors = nullptr);

		DUPL_RETURN Initialize(DX_RESOURCES& r);
		DUPL_RETURN Initialize(DUPLE_RESOURCES& r, ID3D11Device* device, const UINT output);
		RECT ConvertRect(RECT Dirty, const DXGI_OUTPUT_DESC& DeskDesc);

	}
}
