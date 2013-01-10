// Water.h: interface for the CWater class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WATER_H__188515AD_442D_4480_A7C4_7A869F959283__INCLUDED_)
#define AFX_WATER_H__188515AD_442D_4480_A7C4_7A869F959283__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <windows.h>
#include <commdlg.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "d3dx9.h"
#include "d3dx9effect.h"

#include "DXUtil.h"
#include "D3DEnumeration.h"
#include "D3DSettings.h"
#include "D3DApp.h"
#include "D3DFont.h"
#include "D3DUtil.h"

//mine
#include "mmath.h"
#include "mstring.h"
#include "log.h"

#define U16 unsigned short



class WATERVERTEX
{
public:
    FLOAT x,  y,  z;     
    FLOAT nx, ny, nz;     
	FLOAT tu, tv;
	
	WATERVERTEX();
	WATERVERTEX(float, float, float, float, float, float, float, float);
	WATERVERTEX(D3DXVECTOR3, D3DXVECTOR3, D3DXVECTOR2);
	
};
#define D3DFVF_WATERVERTEX ( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 ) 





//-----------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------
class CWater  
{

// variables
protected:
	LPDIRECT3DDEVICE9			m_pd3dDevice;

	//------------------
	//shader stuff
	LPD3DXEFFECT				m_pEffect;
	//------------------

	LPDIRECT3DVERTEXBUFFER9     m_pBufferVertex;
	LPDIRECT3DINDEXBUFFER9      m_pBufferIndex;

	float						absoluteHeight;
	D3DXVECTOR2					segNum;
	D3DXVECTOR2					size;
	int							NumVert, NumFaces, NumIndices;


public:
	//3d bump texture
	LPDIRECT3DTEXTURE9			m_pBumpTex[60];

	//env map
	LPDIRECT3DTEXTURE9			m_pEnvTex;
	LPDIRECT3DSURFACE9			m_pEnvSurf;

	LPDIRECT3DTEXTURE9			m_pRefrTex;
	LPDIRECT3DSURFACE9			m_pRefrSurf;

	LPDIRECT3DTEXTURE9			m_pFresnel;

// functions
protected:

public:
	CWater();
	virtual ~CWater();

	//init block
	HRESULT Init(LPDIRECT3DDEVICE9, LPD3DXEFFECT, D3DXVECTOR3 iposition, D3DXVECTOR2 isize, D3DXVECTOR2 isegnum);

	//framemove block
	HRESULT Update();
	D3DXMATRIX GetReflectionMatrix();
	D3DXPLANE* GetClipPlaneFixedPipeline(D3DXPLANE* plane);
	D3DXPLANE* GetClipPlaneShader(D3DXPLANE* plane, D3DXMATRIX matViewProj);

	//render block
	HRESULT Render(D3DXMATRIX matView, D3DXMATRIX matProj);


	//free block
	HRESULT Release();

};








#endif // !defined(AFX_WATER_H__188515AD_442D_4480_A7C4_7A869F959283__INCLUDED_)
