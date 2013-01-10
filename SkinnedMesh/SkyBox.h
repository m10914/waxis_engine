// SkyBox.h: interface for the CSkyBox class.
//
//////////////////////////////////////////////////////////////////////


#include <windows.h>
#include <commdlg.h>
#include <tchar.h>
#include <stdio.h>
#include <d3dx9.h>
#include "DXUtil.h"
#include "D3DEnumeration.h"
#include "D3DSettings.h"
#include "D3DApp.h"
#include "D3DFont.h"
#include "D3DUtil.h"

#include "log.h"



class SBVERTEX
{
public:
	float x,y,z;
	float tu,tv;
};
#define SBVERTEX_FVF ( D3DFVF_XYZ | D3DFVF_TEX1 )



//--------------------------------------------------------------------------
// Name:
// Desc:
//--------------------------------------------------------------------------
class CSkyBox  
{

//variables
protected:
	LPDIRECT3DDEVICE9 m_pd3dDevice;
	LPD3DXEFFECT m_pEffect;
	LPD3DXMESH	pMesh;
	LPDIRECT3DTEXTURE9 pTex;



//functions
public:
	HRESULT Init(LPDIRECT3DDEVICE9, LPD3DXEFFECT g_pEffect, LPCWSTR);

	HRESULT LoadTexture(LPCWSTR temp);
	
	HRESULT Render(D3DXVECTOR3 CameraCoord, float m_fElapsedTime, D3DXMATRIX* AdditionalMatrix, D3DXMATRIX matView, D3DXMATRIX matProj);
	
	HRESULT Release();


	CSkyBox();
	virtual ~CSkyBox();

};
