//-------------------------------------------------------------
//
// Decal.h: interface for the CDecal class.
//
//
// Desc: each decal is a face with texcoords, vertex position, normal
// Consists of class DECALVERTEX, struct sdecal and class CDecal
// only last one used by programmer, other two are shadowed =)
//



#if !defined(AFX_DECAL_H__905436E3_1C95_4954_9F11_19F740608779__INCLUDED_)
#define AFX_DECAL_H__905436E3_1C95_4954_9F11_19F740608779__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



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
#include "mmath.h"



#define MAXDECALVERTEXNUM 24



class DECALVERTEX
{
public:
    FLOAT x,  y,  z;     
    FLOAT nx, ny, nz;     
	FLOAT tu, tv;

	DECALVERTEX();
	DECALVERTEX(float, float, float, float, float, float, float, float);
	DECALVERTEX(D3DXVECTOR3, D3DXVECTOR3, D3DXVECTOR2);
};

#define D3DFVF_DECALVERTEX ( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 ) 




//-------------------------------------------------------
// Name: sdecal
// Desc: structure, storing decal
//--------------------------------------------------------
struct sdecal
{
	double birthtime; //time of birth
	double lifetime; //time of life


	int NumOfFaces;
	int NumOfVertex;
	DECALVERTEX*		pVert;

	sdecal* pNext;
};



//--------------------------------------------------------
// Name: CDecal
// Desc: decal manager class
//--------------------------------------------------------
class CDecal  
{

// variables
protected:
	LPDIRECT3DDEVICE9			m_pd3dDevice;
	LPD3DXEFFECT				m_pEffect;


	LPDIRECT3DTEXTURE9			pTexture;
	LPDIRECT3DTEXTURE9			pBumpTexture;
	LPDIRECT3DVERTEXBUFFER9     pBufferVertex;

	
	sdecal*						pFirstDecal;


public:




// functions
public:
	CDecal();
	virtual ~CDecal();


	HRESULT Init( char* sTexturePath, LPDIRECT3DDEVICE9 g_pd3dDevice, LPD3DXEFFECT g_pEffect );


	HRESULT Add( int NumOfFaces, DECALVERTEX* i_pVert, double i_lifetime ); //add new decal of this type
	HRESULT Update( double m_fElapsedTime ); //update system
	HRESULT Log();


	HRESULT Render();

	
	HRESULT Release( void );

protected:

};

#endif // !defined(AFX_DECAL_H__905436E3_1C95_4954_9F11_19F740608779__INCLUDED_)

