//--------------------------------------------------------------------
// Shadow Volume Class
// header
//--------------------------------------------------------------------

#include <Windows.h>
#include <commctrl.h>
#include <math.h>
#include <stdio.h>
#include <D3DX9.h>
#include "DXUtil.h"
#include "D3DEnumeration.h"
#include "D3DSettings.h"
#include "D3DApp.h"
#include "D3DUtil.h"
#include "D3DFile.h"
#include "D3DFont.h"


#include "log.h"


//-----------------------------------------------------------------------------
// External definitions and prototypes
//-----------------------------------------------------------------------------
struct VERTEX
{
    D3DXVECTOR3 p;
    D3DXVECTOR3 n;
    FLOAT       tu, tv;

    static const DWORD FVF;
};
//const DWORD VERTEX::FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;

struct SHADOWVERTEX
{
    D3DXVECTOR4 p;
    D3DCOLOR    color;

    static const DWORD FVF;
};
//const DWORD SHADOWVERTEX::FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE;




//useful functions
VOID AddEdge( WORD* pEdges, DWORD& dwNumEdges, WORD v0, WORD v1 );


//-----------------------------------------------------------------------------
// Name: struct ShadowVolume
// Desc: A shadow volume object
//-----------------------------------------------------------------------------
class CShadowVolume
{
	//system stuff
	LPDIRECT3DDEVICE9			m_pd3dDevice;



	D3DXMATRIX*					m_matObjectMatrix;
    D3DXVECTOR3					m_pVertices[32000]; // Vertex data for rendering shadow volume
    DWORD						m_dwNumVertices;


	//big square!
	LPDIRECT3DVERTEXBUFFER9		m_pBigSquareVB;

public:
	//initialization
	HRESULT Init(LPDIRECT3DDEVICE9 d3dDevice,
				 float BackBufferWidth, 
				 float BackBufferHeight);


	//FrameMove
    VOID    Reset();
    HRESULT BuildFromMesh( LPD3DXMESH pObject, D3DXVECTOR3 vLight );
    

	//render
	HRESULT SetMatrix(D3DXMATRIX* matObjectMatrix);
	HRESULT Render();


	HRESULT DrawVolume();
	HRESULT DrawShadow();
	HRESULT RenderShadow();


	//delete
	HRESULT Release();
};

