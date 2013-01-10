
#include "cpic.h"




HRESULT CPic::Init(LPDIRECT3DDEVICE9 g_pd3dDevice, LPCWSTR picpath, int width, int height)
{
	HRESULT hr;

	//get device
	m_pd3dDevice = g_pd3dDevice;


	//sprite initalize
	hr = D3DXCreateTextureFromFileEx(
		m_pd3dDevice,
		picpath,
		height,
		width,
		D3DX_DEFAULT,
		D3DUSAGE_DYNAMIC,
		D3DFMT_UNKNOWN,
		D3DPOOL_DEFAULT,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		0, //D3DCOLOR_COLORVALUE(0,0,0,1),
		&img_info,
		NULL,
		&tex
		);


	char string[256];
	sprintf(string, "...Created texture: %s, %dx%d\n", picpath, (int)img_info.Width, (int)img_info.Height);
	LogPlease(string);

	if(FAILED(hr)) return hr;

	hr = D3DXCreateSprite(
		m_pd3dDevice,
		&spr);

	if(FAILED(hr)) return hr;

	return S_OK;
}

HRESULT CPic::Render(RECT *Rect, D3DXVECTOR2 *pScaling, D3DXVECTOR3 *pRotationCenter, FLOAT Rotation, D3DXVECTOR3 *pTranslation, BOOL bAlpha)
{
	
	//render
	spr->Begin( D3DXSPRITE_ALPHABLEND );

	m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
	m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
	if(bAlpha) m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);

	D3DXMATRIX mat;
	D3DXMatrixScaling( &mat, pScaling->x, pScaling->y, 1);

	spr->SetTransform( &mat );
	spr->Draw(tex,
			  Rect, 
			  pRotationCenter, 
			  pTranslation, 
			  D3DCOLOR_RGBA(255,255,255,255)
			  ); 

	m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE); 

	spr->End();
	
	return S_OK;
}

HRESULT CPic::Release()
{
	tex->Release();
	spr->Release();
	return S_OK;
}




//--------------------------------------------------------
//--------------------------------------------------------





