


#include "mmath.h"



int qu(int a){ return a*a;}
float qu(float a){ return a*a;}
double qu(double a){ return a*a; }

double module( D3DXVECTOR3 vec1, D3DXVECTOR3 vec2 )
{
	return sqrt( qu(vec1.x-vec2.x) + qu(vec1.y-vec2.y) + qu(vec1.z-vec2.z) );
}



//--------------------------------------------
// Name:
// Desc:
//--------------------------------------------
HRESULT MatrixSRTDecompose(D3DXMATRIX* matrix, D3DXMATRIX* Rotation, D3DXVECTOR3* Translation, D3DXVECTOR3* Scale)
{
	//translation
	Translation = &D3DXVECTOR3( matrix[3][0], matrix[3][1], matrix[3][2] );


	//scale
	D3DXVECTOR3 vCols[3] = 
	{
			D3DXVECTOR3(matrix[0][0], matrix[0][1], matrix[0][2] ),
			D3DXVECTOR3(matrix[1][0], matrix[1][1], matrix[1][2] ),
			D3DXVECTOR3(matrix[2][0], matrix[2][1], matrix[2][2] )
	};

	Scale->x = D3DXVec3Length(&vCols[0]);
	Scale->y = D3DXVec3Length(&vCols[1]);
	Scale->z = D3DXVec3Length(&vCols[2]);

	//rotation
    vCols[0].x /= Scale->x;
    vCols[0].y /= Scale->x;
    vCols[0].z /= Scale->x;

    vCols[1].x /= Scale->y;
    vCols[1].y /= Scale->y;
    vCols[1].z /= Scale->y;

    vCols[2].x /= Scale->z;
    vCols[2].y /= Scale->z;
    vCols[2].z /= Scale->z;


	for(int x=0; x<3; x++)
	{
		Rotation[0][x] = vCols[x].x;
		Rotation[1][x] = vCols[x].y;
		Rotation[2][x] = vCols[x].z;
		Rotation[x][3] = 0;
		Rotation[3][x] = 0;
	}
	Rotation[3][3] = 1;



	return S_OK;
}


//--------------------------------------------
// Name:
// Desc:
//--------------------------------------------
HRESULT MatrixDescale(D3DXMATRIX* matrix)
{
	if( matrix == NULL ) return E_FAIL;

	//scale
	D3DXVECTOR3 vCols[3];
	int i,j;

	for( i=0; i<3; i++)
		for( j=0; j<3; j++)
		{
			vCols[i][j] = matrix[i][j];
		}


	D3DXVec3Normalize(&vCols[0], &vCols[0]);
	D3DXVec3Normalize(&vCols[1], &vCols[1]);
	D3DXVec3Normalize(&vCols[2], &vCols[2]);


	for( i=0; i<3; i++)
		for( j=0; j<3; j++)
		{
			matrix[i][j] = vCols[i][j];
		}


	return S_OK;
}



//--------------------------------------------
// Name:
// Desc:
//--------------------------------------------
D3DXVECTOR2* CrossOfVectors(D3DXVECTOR2 u1, D3DXVECTOR2 d1, D3DXVECTOR2 u2, D3DXVECTOR2 d2)
{
	D3DXVECTOR2* point = new D3DXVECTOR2;

	float ua = ( (d2.x-u2.x)*(u1.y-u2.y) - (d2.y-u2.y)*(u1.x-u2.x) ) / ( (d2.y-u2.y)*(d1.x-u1.x) - (d2.x-u2.x)*(d1.y-u1.y) );
	float ub = ( (d1.x-u1.x)*(u1.y-u2.y) - (d1.y-u1.y)*(u1.x-u2.x) ) / ( (d2.y-u2.y)*(d1.x-u1.x) - (d2.x-u2.x)*(d1.y-u1.y) );

	if( ua > 1 || ua < 0 || ub > 1 || ub < 0 ) point =  NULL;
	else *point = D3DXVECTOR2( u1.x + ua*( d1.x - u1.x ), u1.y + ua*( d1.y - u1.y) );

	return point;
}