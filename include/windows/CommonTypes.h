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
#include <DirectXMath.h>
#include <wrl.h>
#include <DirectXMath.h>

#include <d3dcompiler.h>
#include "PixelShader.h"
#include "VertexShader.h"

#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3d11.lib")

namespace SL {
	namespace Screen_Capture {

		extern HRESULT SystemTransitionsExpectedErrors[];
		extern HRESULT CreateDuplicationExpectedErrors[];
		extern HRESULT FrameInfoExpectedErrors[];
		extern HRESULT EnumOutputsExpectedErrors[];

		struct FRAME_DATA
		{
			Microsoft::WRL::ComPtr<ID3D11Texture2D> Frame;
			DXGI_OUTDUPL_FRAME_INFO FrameInfo = { 0 };
			std::shared_ptr<BYTE> MetaData;
			UINT DirtyCount = 0;
			UINT MoveCount = 0;
			UINT SrcreenIndex = 0;
		};		
		struct DX_RESOURCES
		{
			Microsoft::WRL::ComPtr<ID3D11Device> Device;
			Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeviceContext;
			Microsoft::WRL::ComPtr<ID3D11VertexShader> VertexShader;
			Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader;
			Microsoft::WRL::ComPtr<ID3D11InputLayout> InputLayout;
			Microsoft::WRL::ComPtr<ID3D11SamplerState> SamplerLinear;
		} ;


		struct VERTEX
		{
			DirectX::XMFLOAT3 Pos;
			DirectX::XMFLOAT2 TexCoord;
		};


		DUPL_RETURN ProcessFailure(ID3D11Device* Device, LPCWSTR Str, LPCWSTR Title, HRESULT hr, HRESULT* ExpectedErrors = nullptr);
#define RAIIHDC(handle) std::unique_ptr<std::remove_pointer<HDC>::type, decltype(&::DeleteDC)>(handle, &::DeleteDC)
#define RAIIHBITMAP(handle) std::unique_ptr<std::remove_pointer<HBITMAP>::type, decltype(&::DeleteObject)>(handle, &::DeleteObject)
#define RAIIHANDLE(handle) std::unique_ptr<std::remove_pointer<HANDLE>::type, decltype(&::CloseHandle)>(handle, &::CloseHandle)


	}
}
