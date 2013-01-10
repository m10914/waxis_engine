
//basic includes
//#define STRICT
#define NOMINMAX  //this somehow connected to PhysX

#include <windows.h>
#include <commdlg.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <commctrl.h>
#include <basetsd.h>
#include <dxerr9.h>
#include <dshow.h>
#include <dxfile.h>

//physics
#include "NxPhysics.h"
#include "UserAllocator.h"
#include "MyCharacterController.h"
#include "Stream.h"
#include "cooking.h"
#include "ErrorStream.h"
//#include "PerfRenderer.h"
//#include "Utilities.h"
//#include "SamplesVRDSettings.h"


//DX includes
#include "d3dx9.h"
#include "d3dx9effect.h"
#include "D3DFile.h"
#include "DXUtil.h"
#include "D3DEnumeration.h"
#include "D3DSettings.h"
#include "D3DApp.h"
#include "D3DFont.h"
#include "D3DUtil.h"
#include "DMUtil.h"

//mine
#include "mmath.h"
#include "mstring.h"
#include "log.h"


//STL
#include <vector>
#include <list>
#include <iostream>
using namespace std; //for STL to get it work

#define U16 unsigned short


typedef struct{
	D3DXVECTOR2 size, elements;
	char* TexturePath;
	char* HeightmapPath;
	D3DXVECTOR3 coordinate;
	char type;
	int Q;
}TERRAININFO;



class CUSTOMVERTEX
{
public:
    FLOAT x,  y,  z;     
    FLOAT nx, ny, nz;     
	FLOAT tu, tv;
	
	CUSTOMVERTEX();
	CUSTOMVERTEX(float, float, float, float, float, float, float, float);
	CUSTOMVERTEX(D3DXVECTOR3, D3DXVECTOR3, D3DXVECTOR2);
	
};
#define D3DFVF_CUSTOMVERTEX ( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 ) 


#define OUTOFTERRAIN -99999.0f


// terrain type
//up - floor
//down - roof
//none - smth else
#define UP 1
#define DOWN 2
#define NONE 3





//orientations
#define OR_NONE -1
#define OR_N 0
#define OR_NE 1
#define OR_E 2
#define OR_SE 3
#define OR_S 4
#define OR_SW 5
#define OR_W 6
#define OR_NW 7




//-------------------------------------------------------------
// Name:
// Desc:
//-------------------------------------------------------------
class CTextureTable
{
public:
	LPDIRECT3DDEVICE9			m_pd3dDevice;

	LPDIRECT3DTEXTURE9*			ppTex;
	LPDIRECT3DTEXTURE9*			ppBumpTex;

	UINT						elementCount;

public:
	HRESULT Release();
	HRESULT CreateFromFile(LPDIRECT3DDEVICE9, char*);
};

/* example of texture table file:
-----------------------------------------
[Table] 3
0 snow01.jpg
25 snow02.jpg
50 ice01.jpg

-----------------------------------------
*/



//-------------------------------------------------------------
// Name:
// Desc:
//-------------------------------------------------------------
class CTerrain
{
public:
	//message
	char						msg;
	char						type; 

protected:
	LPDIRECT3DDEVICE9			pDirect3DDevice;

	//------------------
	//shader stuff
	LPD3DXEFFECT				m_pEffect;
	//------------------


	D3DXVECTOR3					coordinate;
	D3DXVECTOR3					angle;

	LPDIRECT3DVERTEXBUFFER9     pBufferVertex;
	LPDIRECT3DINDEXBUFFER9      pBufferIndex;
	LPDIRECT3DINDEXBUFFER9		pBufferIndexLOD2;
	LPDIRECT3DINDEXBUFFER9		pBufferIndexLOD4;
	LPDIRECT3DINDEXBUFFER9		pBufferIndexLOD8;
	LPDIRECT3DINDEXBUFFER9		pBufferIndexLOD16;

	//colormap - weights of textures
	LPDIRECT3DTEXTURE9			pTextureMap;


	CTextureTable*				texTable;


	UINT						m_numFaces;
	UINT						m_numIndices;
	UINT						m_numVert;

	int							seg_x, seg_y;
	int							terWidth, terHeight;
	int							HeightQ;

	//render
	int							area;


public:

	//----- physX
	
	NxPhysicsSDK*	mPhysicsSDK;
	NxScene*		mScene;
	NxActor*		object;

	//get functions
	vector<float>				GetVertices();
	vector<U16>					GetIndices();
	LPDIRECT3DVERTEXBUFFER9     GetVertexBuffer() { return pBufferVertex; }
	LPDIRECT3DINDEXBUFFER9      GetIndexBuffer() { return pBufferIndex; }

	//-----

	HRESULT			Init(char*, LPDIRECT3DDEVICE9, CTextureTable*, LPD3DXEFFECT g_pEffect); //string "width height nx ny q TYPE transBmp mapBMP"

	HRESULT			Create(int width, int height, int nx, int ny, int Q, char t_type);
	HRESULT			TransformFromBMP(LPCWSTR filePath);
	HRESULT			InitPhysX( NxPhysicsSDK* gPhysicsSDK, NxScene* gScene );

	
	HRESULT			CalculateNormals();
	
	
	FLOAT			GetYByXZ(float x, float z);
	FLOAT			GetYByQuadAndXZ(float x, float z, CUSTOMVERTEX* Vert);
	D3DXVECTOR2		GetNByXZ(float x, float z);
	CUSTOMVERTEX*	GetQuadByN(D3DXVECTOR2 n);
	D3DXVECTOR3		GetNormalByXZ(float x, float z);

	HRESULT			SetCoordinate( D3DXVECTOR3 coordinate );	


	HRESULT			SetArea(int);
	HRESULT			Render(D3DXMATRIX, D3DXMATRIX, D3DXVECTOR2, D3DXVECTOR2 campos);
	HRESULT			RenderLOD(int LOD, D3DXVECTOR2 A,D3DXVECTOR2 B,D3DXVECTOR2 C,D3DXVECTOR2 D); 


	HRESULT			Release();
	
	~CTerrain() { };
	CTerrain();
};

