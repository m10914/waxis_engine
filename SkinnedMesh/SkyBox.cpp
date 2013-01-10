// SkyBox.cpp: implementation of the CSkyBox class.
//
//////////////////////////////////////////////////////////////////////

#include "SkyBox.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSkyBox::CSkyBox()
{
	pTex = NULL;
	pMesh = NULL;
}

CSkyBox::~CSkyBox()
{

}



//--------------------------------------------------------------------------
// Name:
// Desc:
//--------------------------------------------------------------------------
HRESULT CSkyBox::Init(LPDIRECT3DDEVICE9 device, LPD3DXEFFECT g_pEffect, LPCWSTR meshpath)
{
	//setting device
	m_pd3dDevice = device;
	m_pEffect = g_pEffect;


	LPD3DXMESH	dummy;

	//loading mesh
	if(FAILED( D3DXLoadMeshFromX(meshpath,
					  D3DXMESH_MANAGED,
					  m_pd3dDevice,
					  0,
					  0,
					  0,
					  0,
					  &dummy)))
		return E_FAIL;

	dummy->CloneMeshFVF( D3DXMESH_MANAGED, SBVERTEX_FVF, m_pd3dDevice, &pMesh );

	dummy->Release();


	return S_OK;
}



//--------------------------------------------------------------------------
// Name:
// Desc:
//--------------------------------------------------------------------------
HRESULT CSkyBox::Render(D3DXVECTOR3 CameraCoord, float m_fElapsedTime, D3DXMATRIX* AdditionalMatrix,
						D3DXMATRIX matView, D3DXMATRIX matProj)
{
	DWORD i;


	//setting matrix
	D3DXMATRIX trans;
	D3DXMatrixTranslation( &trans, CameraCoord.x, CameraCoord.y, CameraCoord.z);
	if( AdditionalMatrix != NULL ) //D3DXMatrixMultiply(&trans, &trans, AdditionalMatrix);
		trans = (*AdditionalMatrix) * trans;


	//m_pd3dDevice->SetTransform( D3DTS_WORLD, &trans);
	m_pEffect->SetMatrix( "mViewProj", &(matView * matProj) );
	m_pEffect->SetMatrix( "mWorld", &trans);
	


	m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );		
	m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );

	m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
	m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
	m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE); 

	m_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU,  D3DTADDRESS_MIRROR );
	m_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV,  D3DTADDRESS_MIRROR );


	if(FAILED(m_pEffect->SetTexture( "tColorMap", pTex))) return E_FAIL;
	
	m_pEffect->CommitChanges();
	for( i = 0; i < pMesh->GetNumFaces(); i++)
		pMesh->DrawSubset(i);

	m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
	m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE); 

	return S_OK;
}



//--------------------------------------------------------------------------
// Name:
// Desc:
//--------------------------------------------------------------------------
HRESULT CSkyBox::LoadTexture(LPCWSTR temp)
{
	if( pTex != NULL) pTex->Release();

		D3DXCreateTextureFromFile(m_pd3dDevice,
								  temp,
								  &pTex);

	return S_OK;
}


//--------------------------------------------------------------------------
// Name:
// Desc:
//--------------------------------------------------------------------------
HRESULT CSkyBox::Release()
{

	if(pMesh != NULL) pMesh->Release();

	if(pTex != NULL) pTex->Release();


	return S_OK;
}