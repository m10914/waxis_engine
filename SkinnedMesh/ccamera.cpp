//-----------------------------------------------------------------------------
//  CCAMERA.H
//	          -= header =-
//-----------------------------------------------------------------------------


#include "ccamera.h"



//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
CCamera::CCamera()
{
	vUp = D3DXVECTOR3(0,1,0);
	vEye = D3DXVECTOR3(300,300,300);
	vAt = D3DXVECTOR3(0,0,0);

	angle = D3DXVECTOR2(0,0);
	CRange = 300.f;

	mode = THIRDPERSONVIEW;
}



//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CCamera::Transformation()
{

	CRange -= MousePos.z * 5 * (float)m_fElapsedTime; 
	if(CRange < 5) CRange = 5;

	//mouse
	angle.x = baseAngle;
	//angle.x += ANG * MousePos.x * (float)m_fElapsedTime;
	angle.y += ANG * MousePos.y * (float)m_fElapsedTime;



	//3rd person view camera
	if( mode == THIRDPERSONVIEW )
	{
		if( angle.y > 1.47f) angle.y = 1.47f;
		if( angle.y < -0.1f) angle.y = -0.1f;

		vAt.y += 20.f;

		vEye.x = (float)(vAt.x + sin(angle.x)*cos(angle.y)*CRange) ; 
		vEye.z = (float)(vAt.z + cos(angle.x)*cos(angle.y)*CRange) ; 
		vEye.y = (float)(vAt.y + sin(angle.y)*CRange) ;

		vAt.x = (float)(vAt.x - sin(angle.x)*cos(angle.y)*100) ; 
		vAt.z = (float)(vAt.z - cos(angle.x)*cos(angle.y)*100) ; 
		vAt.y = (float)(vAt.y - sin(angle.y)*100) ;

		//slow camera
		vEye = util.GetEyePt() + (vEye - util.GetEyePt()) * m_fElapsedTime * 20.0f;
	}

	else if( mode == FIRSTPERSONVIEW )
	{
		//1st person view camera
		if( angle.y > 1.44f) angle.y = 1.44f;
		if( angle.y < -0.5f) angle.y = -0.5f;

		vAt.y += 5;//48; //to look from eyes, not from neck

		vEye.x = (float)(vAt.x - sin(angle.x)*10.5f) ; 
		vEye.z = (float)(vAt.z - cos(angle.x)*10.5f) ; 
		vEye.y = vAt.y; //(float)(vAt.y - sin(angle.y)*4.5f) ;

		vAt.x = (float)(vAt.x - sin(angle.x)*cos(angle.y)*100) ; 
		vAt.z = (float)(vAt.z - cos(angle.x)*cos(angle.y)*100) ; 
		vAt.y = (float)(vAt.y - sin(angle.y)*100) ;
	}


	return S_OK;
}

//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CCamera::Update(D3DXVECTOR3 g_mousePos, D3DXVECTOR3 At,
						float g_baseAngle, double eltime,
						float fFov, float fAspect, float fNearPlane, float fFarPlane)
{
	vAt = At;
	baseAngle = g_baseAngle;
	m_fElapsedTime = eltime;

	//update mouse
	MousePos = g_mousePos;

	//transform mouse position
	if( FAILED( Transformation() ))  return E_FAIL;

	util.SetViewParams(vEye, vAt, vUp);
	util.SetProjParams(fFov, fAspect, fNearPlane, fFarPlane);

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
void CCamera::Apply(LPDIRECT3DDEVICE9 m_pd3dDevice)
{
    m_pd3dDevice->SetTransform( D3DTS_VIEW,  &util.GetViewMatrix() );
	m_pd3dDevice->SetTransform( D3DTS_PROJECTION,  &util.GetProjMatrix() );

	return;
}