// Water.cpp: implementation of the CWater class.
//
//////////////////////////////////////////////////////////////////////

#include "Water.h"



//----------------------------
//constructors for watervertex

WATERVERTEX::WATERVERTEX(void)
{
	x = 0;
	y = 0;
	z = 0;
	nx = 0;
	ny = 0;
	nz = 0;
	tv = 0;
	tu = 0;
}

WATERVERTEX::WATERVERTEX(D3DXVECTOR3 pos, D3DXVECTOR3 normal, D3DXVECTOR2 texCoord)
{
	x = pos.x;
	y = pos.y;
	z = pos.z;
	nx = normal.x;
	ny = normal.y;
	nz = normal.z;
	tu = texCoord.x;
	tv = texCoord.y;
}

WATERVERTEX::WATERVERTEX(float ix, float iy, float iz, float inx, float iny, float inz, float itu, float itv)
{
	x = ix;
	y = iy;
	z = iz;
	nx = inx;
	ny = iny;
	nz = inz;
	tu = itu;
	tv = itv;
}

//
//----------------------------





//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CWater::CWater()
{
	m_pd3dDevice = NULL;
	m_pEffect = NULL;

	m_pBufferIndex = NULL;
	m_pBufferVertex = NULL;
}

CWater::~CWater()
{
}




//-----------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------
HRESULT CWater::Init(LPDIRECT3DDEVICE9 g_pd3dDevice, LPD3DXEFFECT g_pEffect, D3DXVECTOR3 iposition, D3DXVECTOR2 isize, D3DXVECTOR2 isegnum)
{
	int i,j;

	//device & effect
	if( g_pd3dDevice != NULL ) m_pd3dDevice = g_pd3dDevice;
	if( g_pEffect != NULL ) m_pEffect = g_pEffect;
	

	//creating textures
	if(FAILED(m_pd3dDevice->CreateTexture(	 512,
											 512,
											 1,
											 D3DUSAGE_RENDERTARGET,
											 D3DFMT_A8R8G8B8,
											 D3DPOOL_DEFAULT,
											 &m_pEnvTex,
											 NULL ))) return E_FAIL;

	m_pEnvTex->GetSurfaceLevel( 0, &m_pEnvSurf );


	if(FAILED(m_pd3dDevice->CreateTexture(	 512,
											 512,
											 1,
											 D3DUSAGE_RENDERTARGET,
											 D3DFMT_A8R8G8B8,
											 D3DPOOL_DEFAULT,
											 &m_pRefrTex,
											 NULL ))) return E_FAIL;
	m_pRefrTex->GetSurfaceLevel( 0, &m_pRefrSurf );


	if(FAILED( D3DXCreateTextureFromFile( m_pd3dDevice,
										  L"data/textures/water/fresnel.bmp",
										  &m_pFresnel ) )) return E_FAIL;

	//init bump textures
	WCHAR wstring[256];
	char string[256];
	for( i = 0; i < 60; i++ )
	{
		if(i >= 0 && i < 10) sprintf(string, "data/textures/water/ocean1000%d.tga", i);
		else sprintf(string, "data/textures/water/ocean100%d.tga", i);

		MultiByteToWideChar( CP_ACP, 0, string, -1, wstring, 256 );
		if( FAILED( D3DXCreateTextureFromFile( m_pd3dDevice, wstring, &m_pBumpTex[i] ) ) )
		{
			HTMLLog("<font color=red>Error! CWater::Init(): Cannot create BumpMap %s\n</font>", string);
			return E_FAIL;
		}
	}


	//-----
	// filling Vertex Buffer
	
	absoluteHeight = iposition.y; //absHeight;
	segNum = isegnum; //D3DXVECTOR2( 200, 200 );
	size = isize; //D3DXVECTOR2( 20000, 20000 );

	NumFaces = 2 * (segNum.x - 1) * (segNum.y - 1);
    NumIndices = 3 * NumFaces;
	NumVert = segNum.x * segNum.y;

	HTMLLog("\tCWater::Init():: NumFaces: %d, NumIndices: %d, NumVert: %d\n", NumFaces, NumIndices, NumVert);


	//--- Vertex Buffer
    double dx = size.x / double(segNum.x - 1);
    double dy = size.y / double(segNum.y - 1);
    
    double ypos = -size.y * 0.5f + iposition.z;

    WATERVERTEX* mesh_vert = new WATERVERTEX[NumVert];
    for(i = 0; i < segNum.y; i++, ypos += dy)
    {
		double xpos = -size.x * 0.5f + iposition.x;
		for(j = 0; j < segNum.x; j++, xpos += dx)
		{
			//Vertex initialization
			mesh_vert[j + i*(int)segNum.x].x = (float)xpos;
			mesh_vert[j + i*(int)segNum.x].y = absoluteHeight;//absHeight;
			mesh_vert[j + i*(int)segNum.x].z = (float)ypos;

			//normals
			mesh_vert[j + i*(int)segNum.x].nx = 0.0f;
			mesh_vert[j + i*(int)segNum.x].ny = 1.0f;
			mesh_vert[j + i*(int)segNum.x].nz = 0.0f;

	
			mesh_vert[j + i*(int)segNum.x].tu = (float)j;
			mesh_vert[j + i*(int)segNum.x].tv = (float)i;


		}
	}

	// Creating Vertex Buffer
    if( FAILED( m_pd3dDevice->CreateVertexBuffer(   NumVert * sizeof(WATERVERTEX),
													0, 
													D3DFVF_WATERVERTEX,
													D3DPOOL_DEFAULT, 
													&m_pBufferVertex, 
													NULL ) ) )
	{
		HTMLLog("\tCWater::Init() Error: cannot create vertex buffer\n");
        return E_FAIL;
	}

	VOID* pBV;
    if( FAILED( m_pBufferVertex->Lock( 0, 
									sizeof(mesh_vert), 
									(void**)&pBV, 
									0 ) ) ) 
        return E_FAIL;
    memcpy( pBV, mesh_vert, NumVert*sizeof(WATERVERTEX) );
    m_pBufferVertex->Unlock(); 

	//------------
	//-----


	//-----
	// filling Index Buffer

	U16* m_ind = new U16[NumIndices];
    U16* ind = m_ind;
    for(i = 0; i < segNum.y-1; i++)
    {
      for(j = 0; j < segNum.x-1; j++)
      {
          // first triangle
          *ind++ = (U16)(i*segNum.x + j);
          *ind++ = (U16)((i+1)*segNum.x + j);
          *ind++ = (U16)((i+1)*segNum.x+j+1);

          // second triangle      
          *ind++ = (U16)(i*segNum.x + j);
          *ind++ = (U16)((i+1)*segNum.x + j+1);
          *ind++ = (U16)(i*segNum.x + j+1);
      }
    }


	// Creating Index Buffer
    if(FAILED(m_pd3dDevice->CreateIndexBuffer(  NumIndices * sizeof(U16), 
												0, 
												D3DFMT_INDEX16, 
												D3DPOOL_DEFAULT,
												&m_pBufferIndex, 
												NULL))) 
	{
		HTMLLog("\tCWater::Init() Error: cannot create index buffer\n");
		return E_FAIL;
	}
		
	VOID* pBI;
    m_pBufferIndex->Lock( 0, sizeof(m_ind) , (void**)&pBI, 0 );
    memcpy( pBI, m_ind, NumIndices * sizeof(U16) );
    m_pBufferIndex->Unlock();

	//------------
	//-----


	return S_OK;
}


//-----------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------
HRESULT CWater::Release()
{

	if( m_pBufferIndex  != NULL)
        m_pBufferIndex->Release(); 

    if( m_pBufferVertex  != NULL)
        m_pBufferVertex->Release();
	
	m_pEnvSurf->Release();
	m_pEnvTex->Release();

	m_pRefrSurf->Release();
	m_pRefrTex->Release();

	m_pFresnel->Release();

	for(int i=0; i<60; i++)
		m_pBumpTex[i]->Release();
	


	return S_OK;
}



//-----------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------
HRESULT CWater::Render(D3DXMATRIX matView, D3DXMATRIX matProj)
{

	if(FAILED(m_pd3dDevice->SetStreamSource( 0, m_pBufferVertex, 0, sizeof(WATERVERTEX) ))) return E_FAIL;
	if(FAILED(m_pd3dDevice->SetFVF( D3DFVF_WATERVERTEX ))) return E_FAIL;
	if(FAILED(m_pd3dDevice->SetIndices(m_pBufferIndex))) return E_FAIL;


	D3DXMATRIX MatrixWorld;
	D3DXMatrixTranslation( &MatrixWorld, 0,0,0);
	m_pEffect->SetMatrix( "mWorld", &MatrixWorld);
	m_pEffect->SetMatrix( "mViewProj", &(matView * matProj) );


	//draw quad
	m_pEffect->SetTexture( "tColorMap", m_pEnvTex);
	m_pEffect->SetTexture( "tSampler2", m_pRefrTex);
	m_pEffect->SetTexture( "tSampler2b", m_pFresnel);
	m_pEffect->SetTexture( "tBumpMap", m_pBumpTex[timeGetTime()/30%60]);

	m_pEffect->CommitChanges();
	if(FAILED(m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, NumVert, 0, NumFaces))) return E_FAIL;
	

	return S_OK;
}



//-----------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------
HRESULT CWater::Update()
{



	return S_OK;
}


//-----------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------
D3DXMATRIX CWater::GetReflectionMatrix()
{
	D3DXMATRIX retMat;
	D3DXVECTOR3 a,b,c;
	D3DXPLANE plane;


	a = D3DXVECTOR3(0,absoluteHeight,0);


	D3DXPlaneFromPointNormal( &plane, &a, &D3DXVECTOR3(0,-1,0) );
    D3DXMatrixReflect( &retMat, &plane );

	return retMat;
}

//-----------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------
D3DXPLANE* CWater::GetClipPlaneFixedPipeline(D3DXPLANE* plane)
{
	D3DXVECTOR3 a,b,c;

	a = D3DXVECTOR3(0,absoluteHeight,0);
	
	D3DXPlaneFromPointNormal( plane, &a, &D3DXVECTOR3(0,-1,0) );
	
	return plane;
}


//-----------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------
D3DXPLANE* CWater::GetClipPlaneShader(D3DXPLANE* plane, D3DXMATRIX matViewProj)
{
	D3DXVECTOR3 a,b,c;
	D3DXMATRIX wmtr;
	D3DXPLANE planebase;
	
	a = D3DXVECTOR3(0,absoluteHeight,0);

	D3DXMatrixTranspose( &wmtr, &matViewProj);
	D3DXMatrixInverse(&wmtr, NULL, &wmtr);
	
	D3DXPlaneFromPointNormal( &planebase, &a, &D3DXVECTOR3(0,-1,0) );
	D3DXPlaneTransform( plane, &planebase, &wmtr); 
	
	return plane;
}