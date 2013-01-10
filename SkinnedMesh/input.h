


#pragma comment(lib, "dinput.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")


//Input class
#define INITGUID

#include "dinput.h"
#include "log.h"

#define KEYDOWN(name,key) (name[key]&0x80)
#define LEFT_BUTTON   0
#define RIGHT_BUTTON  1
#define MIDDLE_BUTTON 2
#define MOUSEBUTTON(name,key) (name.rgbButtons[key]&0x80) 



//--------------------------------------------------
// Name:
// Desc:
//--------------------------------------------------
class Input
{
public:
	char keyboard[256];
	DIMOUSESTATE mouse;
	LPDIRECTINPUT8 pInput;
	LPDIRECTINPUTDEVICE8 pKeyboard;
	LPDIRECTINPUTDEVICE8 pMouse;

	BOOL bCreated;

public:
	Input();
	int CreateInput(HINSTANCE);
	int CreateKeyboard(HWND);
	int CreateMouse(HWND);
	void DeleteKeyboard();
	void DeleteMouse();
	void DeleteInput();
};



//--------------------------------------------------
// Name:
// Desc:
//--------------------------------------------------
Input::Input()
{
	bCreated = FALSE;
	pInput=NULL;
	pKeyboard=NULL;
	pMouse=NULL;
}


//--------------------------------------------------
// Name:
// Desc:
//--------------------------------------------------
int Input::CreateInput(HINSTANCE hinstanceMain)
{



	if(DirectInput8Create(hinstanceMain,
						  DIRECTINPUT_VERSION,
						  IID_IDirectInput8,
						  (void**)&pInput,
						  NULL) 
						  == DIERR_INVALIDPARAM) 
						  LogPlease("Error creating DirectInput9!\n");
	else
		bCreated = TRUE;

	return 0;
}


//--------------------------------------------------
// Name:
// Desc:
//--------------------------------------------------
int Input::CreateKeyboard(HWND hwndMain)
{
	HRESULT hr;

	hr=pInput->CreateDevice(GUID_SysKeyboard,
							&pKeyboard,
							NULL);
	if(FAILED(hr))
	{
		DeleteKeyboard();
		return FALSE;
	}

	hr=pKeyboard->SetDataFormat(&c_dfDIKeyboard);
	if(FAILED(hr))
	{
		DeleteKeyboard();
		return FALSE;
	}
	hr=pKeyboard->SetCooperativeLevel(hwndMain,
									  DISCL_FOREGROUND|DISCL_NONEXCLUSIVE);
	if(FAILED(hr))
	{
		DeleteKeyboard();
		return FALSE;
	}
	hr=pKeyboard->Acquire();

	if(FAILED(hr))
	{
		DeleteKeyboard();
		return FALSE;
	}

	return 0;

}


//--------------------------------------------------
// Name:
// Desc:
//--------------------------------------------------
int Input::CreateMouse(HWND hwndMain)
{
	HRESULT hr;
	hr=pInput->CreateDevice(GUID_SysMouse,
							&pMouse,
							NULL);
	if(FAILED(hr))
	{
		DeleteMouse();
		return FALSE;
	}

	hr=pMouse->SetDataFormat(&c_dfDIMouse);
	if(FAILED(hr))
	{
		DeleteMouse();
		return FALSE;
	}
	hr=pMouse->SetCooperativeLevel(hwndMain,
									  DISCL_FOREGROUND|DISCL_NONEXCLUSIVE);
	if(FAILED(hr))
	{
		DeleteMouse();
		return FALSE;
	}

	hr=pMouse->Acquire();
	if(FAILED(hr))
	{
		DeleteMouse();
		return FALSE;
	}
	return 0;


}


//--------------------------------------------------
// Name:
// Desc:
//--------------------------------------------------
void Input::DeleteKeyboard()
{
	if(pKeyboard!=NULL)
		pKeyboard->Unacquire();
	if(pKeyboard!=NULL)
		pKeyboard->Release();
	pKeyboard=NULL;

}

//--------------------------------------------------
// Name:
// Desc:
//--------------------------------------------------
void Input::DeleteMouse()
{
	if(pMouse!=NULL)
		pMouse->Unacquire();
	if(pMouse!=NULL)
		pMouse->Release();
	pMouse=NULL;
}

//--------------------------------------------------
// Name:
// Desc:
//--------------------------------------------------
void Input::DeleteInput()
{
	if(pInput != NULL)
		pInput->Release();
	pInput=NULL;
}

