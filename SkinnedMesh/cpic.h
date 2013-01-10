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


#include "log.h"


//---------------------------------------------------------
//class CPic
//---------------------------------------------------------
class CPic
{
protected:
	LPD3DXSPRITE                spr;
	LPDIRECT3DTEXTURE9          tex;
	LPDIRECT3DDEVICE9			m_pd3dDevice;

public:
	D3DXIMAGE_INFO              img_info;

	HRESULT Init(LPDIRECT3DDEVICE9 g_pd3dDevice, LPCWSTR picpath, int width, int height);
	HRESULT Render(RECT *Rect, D3DXVECTOR2 *pScaling, D3DXVECTOR3 *pRotationCenter, FLOAT Rotation, D3DXVECTOR3 *pTranslation, BOOL bAlpha);
	HRESULT Release();
};
