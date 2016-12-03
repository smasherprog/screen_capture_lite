// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once
#include <memory>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <warning.h>
#include <DirectXMath.h>

#include <d3dcompiler.h>

#pragma comment(lib,"d3dcompiler.lib")
using namespace DirectX;

namespace SL {
	namespace Screen_Capture {

		const char g_VS[] = "struct VS_INPUT{	float4 Pos : POSITION;	float2 Tex : TEXCOORD;};struct VS_OUTPUT{	float4 Pos : SV_POSITION;	float2 Tex : TEXCOORD;};VS_OUTPUT VS(VS_INPUT input){	return input;}";
		const char g_PS[] = "Texture2D tx : register( t0 );SamplerState samLinear : register(s0);struct PS_INPUT{	float4 Pos : SV_POSITION;	float2 Tex : TEXCOORD;};float4 PS(PS_INPUT input) : SV_Target{	return tx.Sample(samLinear, input.Tex);}";

		extern HRESULT SystemTransitionsExpectedErrors[];
		extern HRESULT CreateDuplicationExpectedErrors[];
		extern HRESULT FrameInfoExpectedErrors[];
		extern HRESULT EnumOutputsExpectedErrors[];

		 enum DUPL_RETURN
		{
			DUPL_RETURN_SUCCESS = 0,
			DUPL_RETURN_ERROR_EXPECTED = 1,
			DUPL_RETURN_ERROR_UNEXPECTED = 2
		};
		struct PTR_INFO
		{
			std::unique_ptr<BYTE[]> PtrShapeBuffer;
			DXGI_OUTDUPL_POINTER_SHAPE_INFO ShapeInfo = { 0 };
			POINT Position = { 0 };
			bool Visible = false;
			UINT BufferSize = 0;
			UINT WhoUpdatedPositionLast = 0;
			LARGE_INTEGER LastTimeStamp = { 0 };
		};

		struct FRAME_DATA
		{
			Microsoft::WRL::ComPtr<ID3D11Texture2D> Frame;
			DXGI_OUTDUPL_FRAME_INFO FrameInfo = { 0 };
			std::unique_ptr<BYTE[]> MetaData;
			UINT DirtyCount = 0;
			UINT MoveCount = 0;
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
		struct THREAD_DATA
		{
			// Used to indicate abnormal error condition
			HANDLE UnexpectedErrorEvent;
			// Used to indicate a transition event occurred e.g. PnpStop, PnpStart, mode change, TDR, desktop switch and the application needs to recreate the duplication interface
			HANDLE ExpectedErrorEvent;
			// Used by WinProc to signal to threads to exit
			HANDLE TerminateThreadsEvent;

			HANDLE TexSharedHandle;
			UINT Output;
			INT OffsetX;
			INT OffsetY;
			std::shared_ptr<PTR_INFO> PtrInfo;
			DX_RESOURCES DxRes;
		} ;

		struct VERTEX
		{
			DirectX::XMFLOAT3 Pos;
			DirectX::XMFLOAT2 TexCoord;
		};


		DUPL_RETURN ProcessFailure(ID3D11Device* Device, LPCWSTR Str, LPCWSTR Title, HRESULT hr, HRESULT* ExpectedErrors = nullptr);
		HRESULT CompileShader( LPCSTR pSrcData,  LPCSTR entryPoint,  LPCSTR profile,  ID3DBlob** blob);


	}
}
