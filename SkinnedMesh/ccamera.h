
#include <windows.h>
#include <commdlg.h>
#include <tchar.h>
#include <strsafe.h>
#include <stdio.h>
#include <d3dx9.h>
#include "DXUtil.h"
#include "D3DEnumeration.h"
#include "D3DSettings.h"
#include "D3DApp.h"
#include "D3DFont.h"
#include "D3DUtil.h"



//defines
#define ANG 0.2f
#define CAMERASPEED 450.f;

#define FIRSTPERSONVIEW 0
#define THIRDPERSONVIEW 1


//-----------------------------------------------------------------------------
// Name:
// Desc:
//------------------------------------------------------------------------------
class CCamera
{

//variables
public:
	CD3DCamera		util;


	D3DXVECTOR3		vEye, vAt, vUp;
	D3DXVECTOR3		MousePos;
	D3DXVECTOR2		angle;
	char			mode;

	float	baseAngle;
	float	CRange;

	double m_fElapsedTime;


//functions
public:
	CCamera();
	HRESULT Update( D3DXVECTOR3, D3DXVECTOR3, float, double,
					float, float, float, float); //projection
	void Apply(LPDIRECT3DDEVICE9);

protected:
	HRESULT Transformation();
};