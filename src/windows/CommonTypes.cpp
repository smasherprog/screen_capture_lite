#include "CommonTypes.h"

// These are the errors we expect from general Dxgi API due to a transition
HRESULT SL::Screen_Capture::SystemTransitionsExpectedErrors[] = {
	DXGI_ERROR_DEVICE_REMOVED,
	DXGI_ERROR_ACCESS_LOST,
	static_cast<HRESULT>(WAIT_ABANDONED),
	S_OK                                    // Terminate list with zero valued HRESULT
};

// These are the errors we expect from IDXGIOutput1::DuplicateOutput due to a transition
HRESULT SL::Screen_Capture::CreateDuplicationExpectedErrors[] = {
	DXGI_ERROR_DEVICE_REMOVED,
	static_cast<HRESULT>(E_ACCESSDENIED),
	DXGI_ERROR_UNSUPPORTED,
	DXGI_ERROR_SESSION_DISCONNECTED,
	S_OK                                    // Terminate list with zero valued HRESULT
};

// These are the errors we expect from IDXGIOutputDuplication methods due to a transition
HRESULT SL::Screen_Capture::FrameInfoExpectedErrors[] = {
	DXGI_ERROR_DEVICE_REMOVED,
	DXGI_ERROR_ACCESS_LOST,
	S_OK                                    // Terminate list with zero valued HRESULT
};

// These are the errors we expect from IDXGIAdapter::EnumOutputs methods due to outputs becoming stale during a transition
HRESULT SL::Screen_Capture::EnumOutputsExpectedErrors[] = {
	DXGI_ERROR_NOT_FOUND,
	S_OK                                    // Terminate list with zero valued HRESULT
};


SL::Screen_Capture::DUPL_RETURN SL::Screen_Capture::ProcessFailure(ID3D11Device * Device, LPCWSTR Str, LPCWSTR Title, HRESULT hr, HRESULT * ExpectedErrors)
{
	HRESULT TranslatedHr;

	// On an error check if the DX device is lost
	if (Device)
	{
		HRESULT DeviceRemovedReason = Device->GetDeviceRemovedReason();

		switch (DeviceRemovedReason)
		{
		case DXGI_ERROR_DEVICE_REMOVED:
		case DXGI_ERROR_DEVICE_RESET:
		case static_cast<HRESULT>(E_OUTOFMEMORY) :
		{
			// Our device has been stopped due to an external event on the GPU so map them all to
			// device removed and continue processing the condition
			TranslatedHr = DXGI_ERROR_DEVICE_REMOVED;
			break;
		}

		case S_OK:
		{
			// Device is not removed so use original error
			TranslatedHr = hr;
			break;
		}

		default:
		{
			// Device is removed but not a error we want to remap
			TranslatedHr = DeviceRemovedReason;
		}
		}
	}
	else
	{
		TranslatedHr = hr;
	}

	// Check if this error was expected or not
	if (ExpectedErrors)
	{
		HRESULT* CurrentResult = ExpectedErrors;

		while (*CurrentResult != S_OK)
		{
			if (*(CurrentResult++) == TranslatedHr)
			{
				return DUPL_RETURN_ERROR_EXPECTED;
			}
		}
	}


	return DUPL_RETURN_ERROR_UNEXPECTED;


}
