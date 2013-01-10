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


#define MIN(a,b) (a<b)?a:b
#define MAX(a,b) (a>b)?a:b

#define NxVec3ToDxVec3(str) D3DXVECTOR3(str.x, str.y, str.z)
#define DxVec3ToNxVec3(str) NxVec3(str.x, str.y, str.z)

int qu(int a);
float qu(float a);
double qu(double a);

double module( D3DXVECTOR3 vec1, D3DXVECTOR3 vec2 );
HRESULT MatrixSRTDecompose(D3DXMATRIX* matrix, D3DXMATRIX* Rotation, D3DXVECTOR3* Translation, D3DXVECTOR3* Scale);
HRESULT MatrixDescale(D3DXMATRIX* matrix);


D3DXVECTOR2* CrossOfVectors(D3DXVECTOR2 u1, D3DXVECTOR2 d1, D3DXVECTOR2 u2, D3DXVECTOR2 d2);