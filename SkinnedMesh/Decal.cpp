//-------------------------------------------------------------
//
// Decal.cpp: implementation of the CDecal class.
//
//
// Desc: each decal is a face with texcoords, vertex position, normal



#include "Decal.h"



//----------------------------
//constructors for decalvertex

DECALVERTEX::DECALVERTEX(void)
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

DECALVERTEX::DECALVERTEX(D3DXVECTOR3 pos, D3DXVECTOR3 normal, D3DXVECTOR2 texCoord)
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

DECALVERTEX::DECALVERTEX(float ix, float iy, float iz, float inx, float iny, float inz, float itu, float itv)
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






//---------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------
CDecal::CDecal()
{
	pFirstDecal = NULL;

	pTexture = NULL;
	m_pd3dDevice = NULL;
	pBufferVertex = NULL;
}

CDecal::~CDecal()
{

}

//---------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------
HRESULT CDecal::Log()
{
	HTMLLog("\t\tCDecal Log:\n");

	int i;
	sdecal* pdec;


	for(pdec = pFirstDecal, i=0; pdec != NULL; pdec = pdec->pNext, i++)
		HTMLLog("\t\t%d: bt = %d, lt = %d, numoffaces = %d, vertex = %d\n", i, (int)pdec->birthtime,
																			   (int)pdec->lifetime,
																			   (int)pdec->NumOfFaces,
																			   (int)pdec->NumOfVertex);

	return S_OK;
}


//---------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------
HRESULT CDecal::Init(char* sTexturePath, LPDIRECT3DDEVICE9 g_pd3dDevice, LPD3DXEFFECT g_pEffect )
{
	HTMLLog("\t...Initializing decal system\n");


	//set device and effect pointers
	m_pd3dDevice = g_pd3dDevice;
	m_pEffect = g_pEffect;

	

	//create texture
	char string[256];
	WCHAR wstring[256];

	sprintf(string, "data/textures/decals/%s", sTexturePath);
	MultiByteToWideChar( CP_ACP, 0, string, -1, wstring, 256 );
	if(FAILED( D3DXCreateTextureFromFile( m_pd3dDevice, wstring, &pTexture) ))
	{
		HTMLLog("<font color=red>!Error: CDecal::Init() cannot create texture from file %s\n</font>", string);
		return E_FAIL;
	}

	sprintf(string, "data/textures/decals/n_%s", sTexturePath);
	MultiByteToWideChar( CP_ACP, 0, string, -1, wstring, 256 );
	if(FAILED( D3DXCreateTextureFromFile( m_pd3dDevice, wstring, &pBumpTexture) ))
	{
		HTMLLog("<font color=red>!Error: CDecal::Init() cannot create bump texture from file %s\n</font>", string);
		return E_FAIL;
	}


	// Creating Vertex Buffer
    if( FAILED( m_pd3dDevice->CreateVertexBuffer(	MAXDECALVERTEXNUM * sizeof(DECALVERTEX),
													0, 
													D3DFVF_DECALVERTEX,
													D3DPOOL_DEFAULT, 
													&pBufferVertex, 
													NULL ) ) )
        return E_FAIL;


	return S_OK;
}


//---------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------
HRESULT CDecal::Release()
{
	sdecal* pdec;
	sdecal* pdec2;

	for(pdec = pFirstDecal; pdec != NULL; pdec = pdec2)
	{
		delete [] pdec->pVert;

		pdec2 = pdec->pNext;
		delete pdec;
	}


	pTexture->Release();

	if(pBufferVertex != NULL) pBufferVertex->Release();

	return S_OK;
}


//---------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------
HRESULT CDecal::Add(int NumOfFaces, DECALVERTEX *i_pVert, double i_lifetime)
{
	sdecal* pdec;


	//find last empty decal
	if(pFirstDecal == NULL)
	{
		pFirstDecal = new sdecal;
		pdec = pFirstDecal;
	}
	else
	{
		for(pdec = pFirstDecal; pdec->pNext != NULL; pdec = pdec->pNext) continue;	
		pdec->pNext = new sdecal;
		pdec = pdec->pNext;
	}


	//filling
	pdec->pNext = NULL;
	pdec->birthtime = timeGetTime();
	pdec->lifetime = i_lifetime;
	pdec->NumOfFaces = NumOfFaces;
	pdec->NumOfVertex = 3*NumOfFaces;
	pdec->pVert = i_pVert;


	//Log();

	return S_OK;
}



//---------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------
HRESULT CDecal::Update(double m_fElapsedTime)
{
	sdecal* pdec;
	sdecal* pdec2;

	for( pdec = pFirstDecal; pdec != NULL; )
	{

		//-- delete decal!
		if( timeGetTime() - pdec->birthtime > pdec->lifetime )
		{
			//if first
			if(pdec == pFirstDecal)
			{
				pFirstDecal = pFirstDecal->pNext;
				
				delete [] pdec->pVert;
				delete pdec;

				pdec = pFirstDecal;
			}
			//else
			else
			{
				pdec2->pNext = pdec->pNext;

				delete [] pdec->pVert;
				delete pdec;

				pdec = pdec2->pNext;
			}

		}

		//-- just skip decal
		else
		{
			pdec2 = pdec;
			pdec = pdec->pNext;
		}
	}



	return S_OK;
}


//---------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------
HRESULT CDecal::Render()
{
	sdecal* pdec;
	VOID* pBV;

	
	m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
	m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
	m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE); 


	
	if(FAILED(m_pd3dDevice->SetStreamSource( 0, pBufferVertex, 0, sizeof(DECALVERTEX) ))) return E_FAIL;
	if(FAILED(m_pd3dDevice->SetFVF( D3DFVF_DECALVERTEX ))) return E_FAIL;
	
	D3DXMATRIX MatrixWorld;
	D3DXMatrixTranslation( &MatrixWorld,  0, 0, 0 );
	m_pEffect->SetMatrix( "mWorld", &MatrixWorld);


	if(FAILED(m_pEffect->SetTexture( "tColorMap", pTexture))) return E_FAIL;
	if(FAILED(m_pEffect->SetTexture( "tBumpMap", pBumpTexture))) return E_FAIL;


	m_pEffect->CommitChanges();
	for(pdec = pFirstDecal; pdec != NULL; pdec = pdec->pNext)
	{

		if( FAILED( pBufferVertex->Lock( 0, 
										 sizeof(pdec->pVert), 
										 (void**)&pBV, 
										 0 ) ) ) 
			return E_FAIL;
		memcpy( pBV, pdec->pVert, pdec->NumOfVertex*sizeof(DECALVERTEX) );
		pBufferVertex->Unlock();


		//drawing
		m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, pdec->NumOfFaces);
	}


	m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE); 

	return S_OK;
}