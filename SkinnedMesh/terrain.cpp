//-------------------------------------------------------------------
//  TERRAIN.C
//
//-------------------------------------------------------------------


#include "terrain.h"






//----------------------------
//constructors for customvertex

CUSTOMVERTEX::CUSTOMVERTEX(void)
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

CUSTOMVERTEX::CUSTOMVERTEX(D3DXVECTOR3 pos, D3DXVECTOR3 normal, D3DXVECTOR2 texCoord)
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

CUSTOMVERTEX::CUSTOMVERTEX(float ix, float iy, float iz, float inx, float iny, float inz, float itu, float itv)
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




//-------------------------------------------------------------
// Name:
// Desc:
//-------------------------------------------------------------
HRESULT CTextureTable::CreateFromFile(LPDIRECT3DDEVICE9 pDirect3DDevice, char* fileName)
{
	m_pd3dDevice = pDirect3DDevice;


	FILE* file = fopen(fileName, "r");
	if(file == NULL) return E_FAIL;


	FindWord(file, "[Table]");
	elementCount = 5;

	ppTex		= new LPDIRECT3DTEXTURE9[elementCount];
	ppBumpTex	= new LPDIRECT3DTEXTURE9[elementCount];


	WCHAR wstring[256];
	char fstring[256];
	char* name;

	for(UINT i=0; i < elementCount; i++)
	{
		name = GetWord(file);

		//init ordinary texture
		sprintf(fstring, "data/textures/terrain/%s", name);
		MultiByteToWideChar( CP_ACP, 0, fstring, -1, wstring, 256 );
		if( FAILED( D3DXCreateTextureFromFile( m_pd3dDevice, wstring, &ppTex[i] ) ) )
		{
			HTMLLog("<font color=red>Error! CTextureTable::CreateFromFile: Cannot create texture %s\n</font>", fstring);
			return E_FAIL;
		}
		
		//init bump texture
		sprintf(fstring, "data/textures/terrain/n_%s", name);
		MultiByteToWideChar( CP_ACP, 0, fstring, -1, wstring, 256 );
		if( FAILED( D3DXCreateTextureFromFile( m_pd3dDevice, wstring, &ppBumpTex[i] ) ) )
		{
			HTMLLog("<font color=red>Error! CTextureTable::CreateFromFile: Cannot create texture %s\n</font>", fstring);
			return E_FAIL;
		}

	}

	fclose(file);

	return S_OK;
}


//-------------------------------------------------------------
// Name:
// Desc:
//-------------------------------------------------------------
HRESULT CTextureTable::Release()
{	
	for(UINT i=0; i < elementCount; i++)
	{
		ppBumpTex[i]->Release();
		ppTex[i]->Release();
	}

	free(ppBumpTex);
	free(ppTex);

	return S_OK;
}




//-------------------------------------------------------------
// Name:
// Desc:
//-------------------------------------------------------------
HRESULT CTerrain::CalculateNormals()
{
	UINT i;
	D3DXVECTOR3 p, q, n;

	U16* pIndices;
	CUSTOMVERTEX* pVertices;

	pBufferIndex->Lock( 0, 0, (void**)&pIndices, 0 );
	pBufferVertex->Lock( 0, 0, (void**)&pVertices, 0);

	for( i = 0; i < m_numIndices; i += 3)
	{
		p = D3DXVECTOR3( pVertices[ pIndices[i+1] ].x - pVertices[ pIndices[i] ].x,
						 pVertices[ pIndices[i+1] ].y - pVertices[ pIndices[i] ].y,
						 pVertices[ pIndices[i+1] ].z - pVertices[ pIndices[i] ].z );
		q = D3DXVECTOR3( pVertices[ pIndices[i+2] ].x - pVertices[ pIndices[i] ].x,
						 pVertices[ pIndices[i+2] ].y - pVertices[ pIndices[i] ].y,
						 pVertices[ pIndices[i+2] ].z - pVertices[ pIndices[i] ].z );

		D3DXVec3Cross(&n, &p, &q);
		D3DXVec3Normalize(&n, &n);

		pVertices[ pIndices[i] ].nx += n.x;
		pVertices[ pIndices[i] ].ny += n.y;
		pVertices[ pIndices[i] ].nz += n.z;
		pVertices[ pIndices[i+1] ].nx += n.x;
		pVertices[ pIndices[i+1] ].ny += n.y;
		pVertices[ pIndices[i+1] ].nz += n.z;
		pVertices[ pIndices[i+2] ].nx += n.x;
		pVertices[ pIndices[i+2] ].ny += n.y;
		pVertices[ pIndices[i+2] ].nz += n.z;

	}

	
	for( i = 0; i < m_numVert; i++)
	{
		n = D3DXVECTOR3( pVertices[i].nx, pVertices[i].ny, pVertices[i].nz);
		D3DXVec3Normalize(&n, &n);
		pVertices[i].nx = n.x;
		pVertices[i].ny = n.y;
		pVertices[i].nz = n.z;
	}
	

	pBufferVertex->Unlock();
	pBufferIndex->Unlock();

	return S_OK;
}



//-------------------------------------------------------------
// Name: GetQuadByN()
// Desc: return a pointer to massive of 4 CUSTOMVERTEX
//-------------------------------------------------------------
CUSTOMVERTEX* CTerrain::GetQuadByN(D3DXVECTOR2 n)
{
	CUSTOMVERTEX* Vert = new CUSTOMVERTEX[4];

	float dx = (float)terWidth/(seg_x-1);
	float dy = (float)terHeight/(seg_y-1);

	int J = (int)n.x;
	int I = (int)n.y;


	CUSTOMVERTEX *pVertices;
    pBufferVertex->Lock( 0, 0, (void**)&pVertices, 0 );
	Vert[0] = pVertices[J + I*seg_x];
	Vert[1] = pVertices[J + 1 + I*seg_x];
	Vert[2] = pVertices[J + (I + 1)*seg_x];
	Vert[3] = pVertices[J + 1 + (I + 1)*seg_x];
	pBufferVertex->Unlock();


	Vert[0].x += coordinate.x;
	Vert[0].y += coordinate.y;
	Vert[0].z += coordinate.z;
	Vert[1].x += coordinate.x;
	Vert[1].y += coordinate.y;
	Vert[1].z += coordinate.z;
	Vert[2].x += coordinate.x;
	Vert[2].y += coordinate.y;
	Vert[2].z += coordinate.z;
	Vert[3].x += coordinate.x;
	Vert[3].y += coordinate.y;
	Vert[3].z += coordinate.z;


	return Vert;	
}


//-------------------------------------------------------------
// Name: 
// Desc:
//-------------------------------------------------------------
HRESULT	CTerrain::InitPhysX(NxPhysicsSDK* gPhysicsSDK, NxScene* gScene)
{
	LogPlease("\t\t<kbd>...Init PhysX()...</kbd>\n");
	vector<U16> Indices;
	vector<float> Vertices;
	int i;

	//checkups
	if( gPhysicsSDK != NULL ) mPhysicsSDK = gPhysicsSDK;
	if( gScene != NULL ) mScene = gScene;

	Indices = GetIndices();
	Vertices = GetVertices();
	LogPlease("\t\tGot indices and vertices...\n");


	//here's the translation we wanted
	//Create pointer for vertices
    NxVec3* verts = new NxVec3[m_numVert];
	int ii = -1;
    for(i = 0; i < m_numVert; i++)
    {
        ++ii;
        verts[i].x = Vertices[ii];
        verts[i].y = Vertices[++ii];
        verts[i].z = Vertices[++ii];
    }
    //Create pointer for indices
    NxU16 *inds = new NxU16[m_numIndices];
    for(i = m_numIndices - 1; i >= 0; i--)
        inds[i] = Indices[i];
	
	LogPlease("\t\tTranslation complete...\n");

	NxTriangleMeshShapeDesc ShapeDesc;
	NxTriangleMeshDesc TriMeshDesc;
	NxActorDesc	ActorDesc;
	NxTriangleMesh* TriMesh = NULL;

    TriMeshDesc.numVertices = m_numVert;
    TriMeshDesc.numTriangles = m_numFaces;
    TriMeshDesc.pointStrideBytes = sizeof(NxVec3);
    TriMeshDesc.triangleStrideBytes = 3*sizeof(NxU16);
    TriMeshDesc.points = verts;
    TriMeshDesc.triangles = inds;
    TriMeshDesc.flags = NX_MF_16_BIT_INDICES ;

	TriMeshDesc.heightFieldVerticalAxis	= NX_Y;
	TriMeshDesc.heightFieldVerticalExtent = -1000.f;


	bool status = InitCooking();
	if (!status) {
		HTMLLog("Unable to initialize the cooking library. Please make sure that you have correctly installed the latest version of the AGEIA PhysX SDK.");
		exit(1);
	}

	MemoryWriteBuffer buf;
	status = CookTriangleMesh(TriMeshDesc, buf);
	if (!status) {
		HTMLLog("Unable to cook a triangle mesh.");
		exit(1);
	}
	MemoryReadBuffer readBuffer(buf.data);
	TriMesh = gPhysicsSDK->createTriangleMesh(readBuffer);
	CloseCooking();

	ShapeDesc.group	= GROUP_COLLIDABLE_NON_PUSHABLE;
	ShapeDesc.meshData = TriMesh;
	ActorDesc.shapes.pushBack(&ShapeDesc);
	ActorDesc.body = NULL; //it means terrian is static (which is always TRUE)
	ActorDesc.globalPose.t = NxVec3(0,0,0);
	object = mScene->createActor(ActorDesc);

	int NumOfShapes = (int)object->getNbShapes();
	HTMLLog("\t\tcreated %d shapes\n", NumOfShapes);

	if( object == NULL ) { HTMLLog("Error creating PhysX objects...\n"); exit(1); }



	HTMLLog("\t\t<kbd>PhysX initialization complete!\n</kbd>");
	return S_OK;
}

//-------------------------------------------------------------
// Name: 
// Desc:
//-------------------------------------------------------------
vector<float> CTerrain::GetVertices(void)
{
	int i;
    vector<float> vertices;

    //BYTE* vbptr = NULL;
	CUSTOMVERTEX *pVertices = NULL;
    pBufferVertex->Lock(0, 0, (LPVOID*)&pVertices, 0);

    for( i = 0; i < m_numVert; i++)
    {
		vertices.push_back(pVertices[i].x);
        vertices.push_back(pVertices[i].y);
        vertices.push_back(pVertices[i].z);     
    }
	
	/*for( i = 0; i < m_numVert/10; i++)
	{
		HTMLLog("(%d,%d,%d) - (%d,%d,%d)\n", (int)pVertices[i].x, (int)pVertices[i].y, (int)pVertices[i].z, (int)vertices[3*i], (int)vertices[3*i+1], (int)vertices[3*i+2]);
	}*/

    pBufferVertex->Unlock(); 

    return vertices;
}


//-------------------------------------------------------------
// Name: 
// Desc:
//-------------------------------------------------------------
vector<U16> CTerrain::GetIndices(void)
{
	int i;
    U16* indices;

    vector<U16> copy;

    pBufferIndex->Lock(0, 0, (LPVOID*)&indices, 0);

    for(int i = 0; i < m_numIndices; i++)
    {
        copy.push_back(indices[i]);
    }



	/*for( i = 0; i < m_numIndices/50; i++)
	{
		HTMLLog("%d - %d\n", (int)indices[i], (int)copy[i]);
	}*/

    pBufferIndex->Unlock();
    return copy;
}


//-------------------------------------------------------------
// Name: 
// Desc:
//-------------------------------------------------------------
D3DXVECTOR2 CTerrain::GetNByXZ(float x, float z)
{
	float dx = (float)terWidth/(seg_x-1);
	float dy = (float)terHeight/(seg_y-1);

	int J = (int)( (x + 0.5f*terWidth - coordinate.x) /dx);
	int I = (int)( (z + 0.5f*terHeight - coordinate.z)/dy);


	return D3DXVECTOR2(J, I);
}


//-------------------------------------------------------------
// Name:
// Desc:
//-------------------------------------------------------------
D3DXVECTOR3 CTerrain::GetNormalByXZ(float x, float z)
{
	CUSTOMVERTEX   Vert[4];


	float dx = (float)terWidth/(seg_x-1);
	float dy = (float)terHeight/(seg_y-1);

	int J = (int)( (x + 0.5f*terWidth - coordinate.x) /dx);
	int I = (int)( (z + 0.5f*terHeight - coordinate.z)/dy);


	if(J > seg_x-2 || J < 0 || I > seg_y-2 || I < 0)
		return D3DXVECTOR3(0,0,0);

	CUSTOMVERTEX *pVertices;
    pBufferVertex->Lock( 0, 0, (void**)&pVertices, 0 );
	Vert[0] = pVertices[J + I*seg_x];
	Vert[1] = pVertices[J + 1 + I*seg_x];
	Vert[2] = pVertices[J + (I + 1)*seg_x];
	Vert[3] = pVertices[J + 1 + (I + 1)*seg_x];
	pBufferVertex->Unlock();


	int index3;

	//index1 = 0, index2 = 3

	if(	qu(x-(Vert[2].x+coordinate.x)) + qu(z-(Vert[2].z+coordinate.z))
		<	qu(x-(Vert[1].x+coordinate.x)) + qu(z-(Vert[1].z+coordinate.z)) )

		 index3 = 2;

	else index3 = 1;


	D3DXVECTOR3 vec3 = D3DXVECTOR3( Vert[0].nx + Vert[3].nx + Vert[index3].nx,
									Vert[0].ny + Vert[3].ny + Vert[index3].ny,
									Vert[0].nz + Vert[3].nz + Vert[index3].nz );
	
	D3DXVec3Normalize(&vec3, &vec3);

	return vec3;
}



//-------------------------------------------------------------
// Name:
// Desc:
//-------------------------------------------------------------
FLOAT CTerrain::GetYByQuadAndXZ(float x, float z, CUSTOMVERTEX *Vert)
{
	//check
	if(Vert==NULL) { HTMLLog("CTerrain::GetYByQuadAndXZ() Error!\n"); return -1; }


	float	x1= Vert[0].x,
			x2= Vert[3].x,
			y1= Vert[0].y,
			y2= Vert[3].y,
			z1= Vert[0].z,
			z2= Vert[3].z;
	float	x3, y3, z3;

	//what face of quad
	if( qu(x-Vert[2].x) + qu(z-Vert[2].z) < qu(x-Vert[1].x) + qu(z-Vert[1].z))
	{
		x3 = Vert[2].x;
		y3 = Vert[2].y;
		z3 = Vert[2].z;
	}
	else
	{
		x3 = Vert[1].x;
		y3 = Vert[1].y;
		z3 = Vert[1].z;
	}


	double a= (y2-y1)*(z3-z1)-(z2-z1)*(y3-y1);
	double b= ((x2-x1)*(z3-z1)-(z2-z1)*(x3-x1));
	double c= (x2-x1)*(y3-y1)-(y2-y1)*(x3-x1);
	double d= - a*x1 + b*y1 - c*z1;

	float Y = float((a*x+c*z+d)/b);
	return Y;
}


//-------------------------------------------------------------
// Name:
// Desc:
//-------------------------------------------------------------
FLOAT CTerrain::GetYByXZ(float x, float z)
{
	CUSTOMVERTEX   Vert[4];


	float dx = (float)terWidth/(seg_x-1);
	float dy = (float)terHeight/(seg_y-1);

	int J = (int)( (x + 0.5f*terWidth - coordinate.x) /dx);
	int I = (int)( (z + 0.5f*terHeight - coordinate.z)/dy);


	if(J > seg_x-2 || J < 0 || I > seg_y-2 || I < 0)
		return OUTOFTERRAIN;

	CUSTOMVERTEX *pVertices;
    pBufferVertex->Lock( 0, 0, (void**)&pVertices, 0 );
	Vert[0] = pVertices[J + I*seg_x];
	Vert[1] = pVertices[J + 1 + I*seg_x];
	Vert[2] = pVertices[J + (I + 1)*seg_x];
	Vert[3] = pVertices[J + 1 + (I + 1)*seg_x];
	pBufferVertex->Unlock();


	Vert[0].x += coordinate.x;
	Vert[0].y += coordinate.y;
	Vert[0].z += coordinate.z;
	Vert[1].x += coordinate.x;
	Vert[1].y += coordinate.y;
	Vert[1].z += coordinate.z;
	Vert[2].x += coordinate.x;
	Vert[2].y += coordinate.y;
	Vert[2].z += coordinate.z;
	Vert[3].x += coordinate.x;
	Vert[3].y += coordinate.y;
	Vert[3].z += coordinate.z;


	return GetYByQuadAndXZ(x, z, Vert);
}


//-------------------------------------------------------------
// Name:
// Desc:
//-------------------------------------------------------------
HRESULT CTerrain::SetArea(int dist)
{
	float dx = (float)terWidth/(seg_x-1);

	area = dist/dx;

	return S_OK;
}



//-------------------------------------------------------------
// Name:
// Desc:
//-------------------------------------------------------------
HRESULT CTerrain::SetCoordinate(D3DXVECTOR3 in_coord)
{
	coordinate = in_coord;
	return S_OK;
}


/*
//-------------------------------------------------------------
// Name:
// Desc:
//-------------------------------------------------------------
HRESULT CTerrain::LoadMappingFromBMP(char* filePath)
{
	HBITMAP     hbm;                                                   
    BITMAP      bm;                                                     
    COLORREF    pix;                                                   


	//loading and initializing bitmap
    hbm = (HBITMAP) LoadImage (NULL,
							   filePath, 
							   IMAGE_BITMAP, 
							   0, 
							   0,  
                               LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	if(hbm == NULL) return E_FAIL;
    GetObject (hbm, sizeof(bm), &bm);                                   
                                                                                                                                               
    HDC hdc = CreateCompatibleDC (0);                                   
    SelectObject (hdc, hbm);                                                                                                                   

	//init colors
	colors = new int[m_numFaces/2];

	for(UINT i=0; i < m_numFaces/2; i++)
	{
        pix = GetPixel (hdc, i%(seg_x-1), i/(seg_x-1));                                   
        colors[i] = (int)(GetRValue(pix) + GetGValue(pix) + GetBValue(pix));                                                                                 
	}                                                          


    ReleaseDC (0, hdc);                                                 
    DeleteObject(hbm);


	return S_OK;
}
*/

//-------------------------------------------------------------
// Name:
// Desc: non-directx function
//-------------------------------------------------------------
HRESULT CTerrain::TransformFromBMP(LPCWSTR filePath)
{
	HBITMAP     hbm;                                                   
    BITMAP      bm;                                                     
    COLORREF    pix;                                                   


	//loading and initializing bitmap
    hbm = (HBITMAP) LoadImage (NULL, filePath, IMAGE_BITMAP, 0, 0,  
                                LR_LOADFROMFILE | LR_CREATEDIBSECTION); 
    GetObject (hbm, sizeof(bm), &bm);                                   
                                                                                                                                               
    HDC hdc = CreateCompatibleDC (0);                                   
    SelectObject (hdc, hbm);                                                                                                                   

	//buffer

	CUSTOMVERTEX *pVertices;
    pBufferVertex->Lock( 0, 0, (void**)&pVertices, 0 );
	for(DWORD i=0; i < m_numVert; i++)
	{
        pix = GetPixel (hdc, i%seg_x, i/seg_x);                                   
        pVertices[i].y += (float)(GetRValue(pix)+GetGValue(pix)+GetBValue(pix))/2*HeightQ;                                                                                 
	}                                                           
              


	pBufferVertex->Unlock();

    ReleaseDC (0, hdc);                                                 
    DeleteObject(hbm);


	return S_OK;
}


//-------------------------------------------------------------
// Name:
// Desc:
//-------------------------------------------------------------
CTerrain::CTerrain()
{
	coordinate = D3DXVECTOR3(0,0,0);
	pBufferVertex = NULL;
	pBufferIndex   = NULL;
	pBufferIndexLOD2   = NULL;
	pBufferIndexLOD4   = NULL;
	pBufferIndexLOD8   = NULL;
	pBufferIndexLOD16   = NULL;

	//message
	msg = 0;
	area = 50;
}



//-------------------------------------------------------------
// Name: Init()
// Desc: string must be: "width height nx ny q TYPE transBmp mapBMP"
//-------------------------------------------------------------
HRESULT CTerrain::Init(char* string, LPDIRECT3DDEVICE9 g_pDirect3DDevice, CTextureTable* g_texTable, LPD3DXEFFECT g_pEffect)
{
	//init device
	pDirect3DDevice = g_pDirect3DDevice;
	m_pEffect = g_pEffect;

	if(g_texTable != NULL) texTable = g_texTable;


	WCHAR wstring[256];
	int width, height;
	int nx, ny;
	int Q;
	char t_type[10];
	char r_type;
	char hm_path[256];
	char tm_path[256];
	char strCoord[256];

	sscanf(string,"%d %d %d %d %d %s %s %s %s", &width, &height, &nx, &ny, &Q, &t_type, hm_path, tm_path, strCoord);
	
	if( !strcmp( t_type, "UP" ) ) r_type = UP;
	else if( !strcmp( t_type, "DOWN" ) ) r_type = DOWN;
	else r_type = NONE;

	
	if(FAILED(Create(width, height, nx, ny, Q, r_type))) return E_FAIL;

	MultiByteToWideChar( CP_ACP, 0, hm_path, -1, wstring, 256 );
	if(FAILED(TransformFromBMP(wstring))) return E_FAIL;
	

	LogPlease("\t...loading texture map from file...\n");
	MultiByteToWideChar( CP_ACP, 0, tm_path, -1, wstring, 256 );
	if( FAILED( D3DXCreateTextureFromFile( pDirect3DDevice, wstring, &pTextureMap ) ) )
	{
		HTMLLog("<font color=red>Error! CTerrain::Init(): Cannot create TextureMap %s\n</font>", tm_path);
		return E_FAIL;
	}


	LogPlease("\t...calculating normals...\n");
	if(FAILED(CalculateNormals())) return E_FAIL;

	SetCoordinate(ConstructVec3(strCoord));

	LogPlease("Terrain innitialized successfully!\n");

	return S_OK;
}



//-------------------------------------------------------------
// Name:
// Desc:
//-------------------------------------------------------------
HRESULT CTerrain::Create(int width, int height, 
						 int nx, int ny, int Q,
						 char t_type)
{

	//setting variables
	HeightQ = Q;
	seg_x = nx;
	seg_y = ny;
	terWidth = width;
	terHeight = height;
	type = t_type;


	LogPlease("\t...Terrain initialization...\n");


	//simple initializing
	int i,j;




	//--- IndexBuffer
	m_numFaces = 2 * (nx-1)*(ny-1);
    m_numIndices = 3 * m_numFaces;

	//log
	HTMLLog("\tNumOfFaces = %d;\tNumOfIndices = %d;\tNumOfVert = %d;\n", m_numFaces, m_numIndices, nx*ny);


	U16*  m_ind = new U16[m_numIndices];
    U16* ind = m_ind;
    for(i = 0; i < ny-1; i++)
    {
      for(j = 0; j < nx-1; j++)
      {
          // first triangle
          *ind++ = (U16)(i*nx + j);
          *ind++ = (U16)((i+1)*nx + j);
          *ind++ = (U16)((i+1)*nx+j+1);

          // second triangle      
          *ind++ = (U16)(i*nx + j);
          *ind++ = (U16)((i+1)*nx + j+1);
          *ind++ = (U16)(i*nx + j+1);
      }
    }




	//--- Vertex Buffer
    double dx = width / double(nx-1);
    double dy = height / double(ny-1);
    
    double ypos = -height * 0.5f;

    CUSTOMVERTEX* mesh_vert = new CUSTOMVERTEX[nx*ny];
    for(i = 0; i < ny; i++, ypos += dy)
    {
		double xpos = -width * 0.5f;
		for(j = 0; j < nx; j++, xpos += dx)
		{
			//Vertex initialization
			mesh_vert[j + i*nx].x = (float)xpos;
			mesh_vert[j + i*nx].y = -100.0f;
			mesh_vert[j + i*nx].z = (float)ypos;

			//normals
			mesh_vert[j + i*nx].nx = 0.0f;
			mesh_vert[j + i*nx].ny = 1.0f;
			mesh_vert[j + i*nx].nz = 0.0f;

	
			mesh_vert[j + i*nx].tu = (float)j;
			mesh_vert[j + i*nx].tv = (float)i;


		}
	}

    m_numVert = nx * ny;



	// Creating Vertex Buffer
    if( FAILED( pDirect3DDevice->CreateVertexBuffer( m_numVert * sizeof(CUSTOMVERTEX),
													0, 
													D3DFVF_CUSTOMVERTEX,
													D3DPOOL_DEFAULT, 
													&pBufferVertex, 
													NULL ) ) )
        return E_FAIL;
    VOID* pBV;
    if( FAILED( pBufferVertex->Lock( 0, 
									sizeof(mesh_vert), 
									(void**)&pBV, 
									0 ) ) ) 
        return E_FAIL;
    memcpy( pBV, mesh_vert, m_numVert*sizeof(CUSTOMVERTEX) );
    pBufferVertex->Unlock(); 



	// Creating Index Buffer
    if(FAILED(pDirect3DDevice->CreateIndexBuffer( m_numIndices* sizeof(U16), 
												0, 
												D3DFMT_INDEX16, 
												D3DPOOL_DEFAULT,
												&pBufferIndex, 
												NULL))) 
		return E_FAIL;
    VOID* pBI;
    pBufferIndex->Lock( 0, sizeof(m_ind) , (void**)&pBI, 0 );
    memcpy( pBI, m_ind, m_numIndices*sizeof(U16) );
    pBufferIndex->Unlock();	




	//--
	// Creating LOD index buffers

	// WARNING! need testing for memory failures
/*
	//lod 2
    if(FAILED(pDirect3DDevice->CreateIndexBuffer( m_numIndices/4* sizeof(U16), 
												0, 
												D3DFMT_INDEX16, 
												D3DPOOL_DEFAULT,
												&pBufferIndexLOD2, 
												NULL)))
		return E_FAIL;

	delete [] m_ind;
	m_ind = new U16[m_numIndices/4+2];
	ind = m_ind;
    for(i = 0; i < (ny-1)/2; i++)
    {
      for(j = 0; j < (nx-1)/2; j++)
      {
          // first triangle
          *ind++ = (U16)(2*i*nx + 2*j);
          *ind++ = (U16)(2*(i+1)*nx + 2*j);
          *ind++ = (U16)(2*(i+1)*nx+2*(j+1));

          // second triangle      
          *ind++ = (U16)(2*i*nx + 2*j);
          *ind++ = (U16)(2*(i+1)*nx + 2*(j+1));
          *ind++ = (U16)(2*i*nx + 2*(j+1));
      }
    }
	

    pBufferIndexLOD2->Lock( 0, sizeof(m_ind) , (void**)&pBI, 0 );
    memcpy( pBI, m_ind, m_numIndices/4*sizeof(U16) );
    pBufferIndexLOD2->Unlock();	


	//lod 4
    if(FAILED(pDirect3DDevice->CreateIndexBuffer( m_numIndices/16* sizeof(U16), 
												0, 
												D3DFMT_INDEX16, 
												D3DPOOL_DEFAULT,
												&pBufferIndexLOD4, 
												NULL)))
		return E_FAIL;

	delete [] m_ind;
	m_ind = new U16[m_numIndices/16+4];
	ind = m_ind;
    for(i = 0; i < (ny-1)/4; i++)
    {
      for(j = 0; j < (nx-1)/4; j++)
      {
          // first triangle
          *ind++ = (U16)(4*i*nx + 4*j);
          *ind++ = (U16)(4*(i+1)*nx + 4*j);
          *ind++ = (U16)(4*(i+1)*nx+4*(j+1));

          // second triangle      
          *ind++ = (U16)(4*i*nx + 4*j);
          *ind++ = (U16)(4*(i+1)*nx + 4*(j+1));
          *ind++ = (U16)(4*i*nx + 4*(j+1));
      }
    }
	

    pBufferIndexLOD4->Lock( 0, sizeof(m_ind) , (void**)&pBI, 0 );
    memcpy( pBI, m_ind, m_numIndices/16*sizeof(U16) );
    pBufferIndexLOD4->Unlock();	


	//lod 8
    if(FAILED(pDirect3DDevice->CreateIndexBuffer( m_numIndices/64* sizeof(U16), 
												0, 
												D3DFMT_INDEX16, 
												D3DPOOL_DEFAULT,
												&pBufferIndexLOD8, 
												NULL)))
		return E_FAIL;

	delete [] m_ind;
	m_ind = new U16[m_numIndices/64+8];
	ind = m_ind;
    for(i = 0; i < (ny-1)/8; i++)
    {
      for(j = 0; j < (nx-1)/8; j++)
      {
          // first triangle
          *ind++ = (U16)(8*i*nx + 8*j);
          *ind++ = (U16)(8*(i+1)*nx + 8*j);
          *ind++ = (U16)(8*(i+1)*nx+8*(j+1));

          // second triangle      
          *ind++ = (U16)(8*i*nx + 8*j);
          *ind++ = (U16)(8*(i+1)*nx + 8*(j+1));
          *ind++ = (U16)(8*i*nx + 8*(j+1));
      }
    }
	

    pBufferIndexLOD8->Lock( 0, sizeof(m_ind) , (void**)&pBI, 0 );
    memcpy( pBI, m_ind, m_numIndices/64*sizeof(U16) );
    pBufferIndexLOD8->Unlock();	


	//lod 16
    if(FAILED(pDirect3DDevice->CreateIndexBuffer( m_numIndices/256* sizeof(U16), 
												0, 
												D3DFMT_INDEX16, 
												D3DPOOL_DEFAULT,
												&pBufferIndexLOD16, 
												NULL)))
		return E_FAIL;

	delete [] m_ind;
	m_ind = new U16[m_numIndices/256+16];
	ind = m_ind;
    for(i = 0; i < (ny-1)/16; i++)
    {
      for(j = 0; j < (nx-1)/16; j++)
      {
          // first triangle
          *ind++ = (U16)(16*i*nx + 16*j);
          *ind++ = (U16)(16*(i+1)*nx + 16*j);
          *ind++ = (U16)(16*(i+1)*nx + 16*(j+1));

          // second triangle      
          *ind++ = (U16)(16*i*nx + 16*j);
          *ind++ = (U16)(16*(i+1)*nx + 16*(j+1));
          *ind++ = (U16)(16*i*nx + 16*(j+1));
      }
    }
	

    pBufferIndexLOD16->Lock( 0, sizeof(m_ind) , (void**)&pBI, 0 );
    memcpy( pBI, m_ind, m_numIndices/256*sizeof(U16) );
    pBufferIndexLOD16->Unlock();	


	//--



	//getting descriptions for log
	D3DVERTEXBUFFER_DESC VDesc;
	pBufferVertex->GetDesc(&VDesc);
	HTMLLog("\tVertex buffer size: %d Kb", (int)VDesc.Size/1024);

	D3DINDEXBUFFER_DESC IDesc;
	pBufferIndex->GetDesc(&IDesc);
	HTMLLog("\tIndex buffer size: %d Kb", (int)IDesc.Size/1024);

	pBufferIndexLOD2->GetDesc(&IDesc);
	HTMLLog("\tLOD2 Index buffer size: %d Kb\n", (int)IDesc.Size/1024);

	pBufferIndexLOD4->GetDesc(&IDesc);
	HTMLLog("\tLOD4 Index buffer size: %d Kb\n", (int)IDesc.Size/1024);

	pBufferIndexLOD8->GetDesc(&IDesc);
	HTMLLog("\tLOD8 Index buffer size: %d Kb\n", (int)IDesc.Size/1024);

	pBufferIndexLOD16->GetDesc(&IDesc);
	HTMLLog("\tLOD16 Index buffer size: %d Kb\n", (int)IDesc.Size/1024);
*/

	delete [] m_ind;
	delete [] mesh_vert;

	return S_OK;
}


//-------------------------------------------------------------
// Name:
// Desc:
//-------------------------------------------------------------
HRESULT CTerrain::RenderLOD(int LOD, D3DXVECTOR2 A, D3DXVECTOR2 B, D3DXVECTOR2 C, D3DXVECTOR2 D)
{
	int i,j;
	int m,n;
	D3DXVECTOR2 L;

	
	//set lod-dependent bufferindices
	if(LOD == 1) pDirect3DDevice->SetIndices(pBufferIndex);
	else if(LOD == 2) pDirect3DDevice->SetIndices(pBufferIndexLOD2);
	else if(LOD == 4) pDirect3DDevice->SetIndices(pBufferIndexLOD4);
	else if(LOD == 8) pDirect3DDevice->SetIndices(pBufferIndexLOD8);
	else if(LOD == 16) pDirect3DDevice->SetIndices(pBufferIndexLOD16);
	else if(LOD == 0) pDirect3DDevice->SetIndices(pBufferIndex);	
	else HTMLLog("<font color=red>Error!: invalid LOD id</font>\n");
	

	//roundings
	if(LOD != 0)
	{
		A.x = MAX(A.x - (int)A.x%LOD - LOD, 0);
		B.x = MIN(B.x + (LOD - (int)B.x%LOD) + LOD, seg_x-1);
		A.y = MAX(A.y - (int)A.y%LOD - LOD, 0);
		B.y = MIN(B.y + (LOD - (int)B.y%LOD) + LOD, seg_y-1);
	}


	// DRAWING!
	m_pEffect->CommitChanges();
	if(LOD == 0)
	{
		if(FAILED(pDirect3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_numVert, 0, m_numFaces))) return E_FAIL;
	}

	else if(LOD == 1)
	{
		for( m = A.x; m < B.x; m++) //rows
			for( n = A.y; n < B.y; n++) //cols
			{
				i = m + n*(seg_x-1);
				if(FAILED(pDirect3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 6, i*6, 2))) return E_FAIL;
			}
	}
	
	//if LOD > 0
	else
	{
		//miscellanious
		L = D3DXVECTOR2(0,0);


		for( m = A.x; m < C.x; m++) //rows
			for( n = A.y; n < B.y; n++) //cols
			{
				if(m%LOD == L.x && n%LOD == L.y)
				{
					i = m/LOD + n/LOD*((seg_x-1)/LOD);
					if(FAILED(pDirect3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 6, i*6, 2))) return E_FAIL;
				}
			}

		for( m = C.x; m < D.x; m++) //rows
			for( n = A.y; n < C.y; n++) //cols
			{
				if(m%LOD == L.x && n%LOD == L.y)
				{
					i = m/LOD + n/LOD*((seg_x-1)/LOD);
					if(FAILED(pDirect3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 6, i*6, 2))) return E_FAIL;
				}
			}

		for( m = C.x; m < D.x; m++) //rows
			for( n = D.y; n < B.y; n++) //cols
			{
				if(m%LOD == L.x && n%LOD == L.y)
				{
					i = m/LOD + n/LOD*((seg_x-1)/LOD);
					if(FAILED(pDirect3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 6, i*6, 2))) return E_FAIL;
				}
			}

		for( m = D.x; m < B.x; m++) //rows
			for( n = A.y; n < B.y; n++) //cols
			{
				if(m%LOD == L.x && n%LOD == L.y)
				{
					i = m/LOD + n/LOD*((seg_x-1)/LOD);
					if(FAILED(pDirect3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 6, i*6, 2))) return E_FAIL;
				}
			}

	}


	return S_OK;
}


//-------------------------------------------------------------
// Name:
// Desc:
//-------------------------------------------------------------
HRESULT CTerrain::Render(D3DXMATRIX matView, D3DXMATRIX matProj, D3DXVECTOR2 coord, D3DXVECTOR2 campos)
{
	int i,j;
	int m,n;


	D3DXMATRIX MatrixWorld;
	D3DXMATRIX MatrixWorldD;
	D3DXMatrixTranslation( &MatrixWorld,  coordinate.x, coordinate.y, coordinate.z );
	m_pEffect->SetMatrix( "mViewProj", &(matView * matProj) );
	m_pEffect->SetMatrix( "mWorld", &MatrixWorld);


	if(type == UP) pDirect3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
	else if(type == DOWN) pDirect3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
	else pDirect3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );


	if(FAILED(pDirect3DDevice->SetStreamSource( 0, pBufferVertex, 0, sizeof(CUSTOMVERTEX) ))) return E_FAIL;
	if(FAILED(pDirect3DDevice->SetFVF( D3DFVF_CUSTOMVERTEX ))) return E_FAIL;
	if(FAILED(pDirect3DDevice->SetIndices(pBufferIndex))) return E_FAIL;


	//-- texture splatting drawing method
	//-----------------------------------

	// 1: texture set

	//texturemap
	if(FAILED(m_pEffect->SetTexture( "tAlphaMask", pTextureMap))) return E_FAIL;

	//base
	if(FAILED(m_pEffect->SetTexture( "tColorMap", texTable->ppTex[0]))) return E_FAIL;
	if(FAILED(m_pEffect->SetTexture( "tBumpMap", texTable->ppBumpTex[0]))) return E_FAIL;	

	//alpha
	if(FAILED(m_pEffect->SetTexture( "tSampler2", texTable->ppTex[1]))) return E_FAIL;
	if(FAILED(m_pEffect->SetTexture( "tSampler2b", texTable->ppBumpTex[1]))) return E_FAIL;	
	//red
	if(FAILED(m_pEffect->SetTexture( "tSampler3", texTable->ppTex[2]))) return E_FAIL;
	if(FAILED(m_pEffect->SetTexture( "tSampler3b", texTable->ppBumpTex[2]))) return E_FAIL;	
	//green
	if(FAILED(m_pEffect->SetTexture( "tSampler4", texTable->ppTex[3]))) return E_FAIL;
	if(FAILED(m_pEffect->SetTexture( "tSampler4b", texTable->ppBumpTex[3]))) return E_FAIL;	
	//blue
	if(FAILED(m_pEffect->SetTexture( "tSampler5", texTable->ppTex[4]))) return E_FAIL;
	if(FAILED(m_pEffect->SetTexture( "tSampler5b", texTable->ppBumpTex[4]))) return E_FAIL;	
	

	// 2: set additional shader variables
	m_pEffect->SetVector("vSegs", &D3DXVECTOR4(seg_x, seg_y, 0.f, 0.f) );


	// 3: draw!
	int LODarea = area/3;

	//render all
	RenderLOD(0, D3DXVECTOR2(0,0), D3DXVECTOR2(0,0), D3DXVECTOR2(0,0), D3DXVECTOR2(0,0) );

	/*
	RenderLOD(1, D3DXVECTOR2( MAX(coord.x-LODarea, 0),			MAX(coord.y-LODarea, 0) ),
				 D3DXVECTOR2( MIN(coord.x+LODarea, seg_x-1),	MIN(coord.y+LODarea, seg_y-1) ),
				 D3DXVECTOR2( 0, 0 ),
				 D3DXVECTOR2( 0, 0 ) );

	RenderLOD(2, D3DXVECTOR2( MAX(coord.x-3*LODarea, 0),       MAX(coord.y-3*LODarea, 0)       ),
				 D3DXVECTOR2( MIN(coord.x+3*LODarea, seg_x-1), MIN(coord.y+3*LODarea, seg_y-1) ),
				 D3DXVECTOR2( MAX(coord.x-LODarea, 0),         MAX(coord.y-LODarea, 0)         ),
				 D3DXVECTOR2( MIN(coord.x+LODarea, seg_x-1),   MIN(coord.y+LODarea, seg_y-1)   )
			 );

	
	RenderLOD(8, D3DXVECTOR2( MAX(coord.x-6*LODarea, 0),       MAX(coord.y-6*LODarea, 0)       ),
				 D3DXVECTOR2( MIN(coord.x+6*LODarea, seg_x-1), MIN(coord.y+6*LODarea, seg_y-1) ),
				 D3DXVECTOR2( MAX(coord.x-3*LODarea, 0),       MAX(coord.y-3*LODarea, 0)       ),
				 D3DXVECTOR2( MIN(coord.x+3*LODarea, seg_x-1), MIN(coord.y+3*LODarea, seg_y-1) )
			 );
	/*
	//for very distant lands
	RenderLOD(8, D3DXVECTOR2( MAX(coord.x-12*LODarea, 0),      MAX(coord.y-12*LODarea, 0)       ),
				 D3DXVECTOR2( MIN(coord.x+12*LODarea, seg_x-1),MIN(coord.y+12*LODarea, seg_y-1) ),
				 D3DXVECTOR2( MAX(coord.x-6*LODarea, 0),       MAX(coord.y-6*LODarea, 0)       ),
				 D3DXVECTOR2( MIN(coord.x+6*LODarea, seg_x-1), MIN(coord.y+6*LODarea, seg_y-1) )
			 );
	
	RenderLOD(16,D3DXVECTOR2( MAX(coord.x-20*LODarea, 0),      MAX(coord.y-20*LODarea, 0)       ),
				 D3DXVECTOR2( MIN(coord.x+20*LODarea, seg_x-1),MIN(coord.y+20*LODarea, seg_y-1) ),
				 D3DXVECTOR2( MAX(coord.x-12*LODarea, 0),       MAX(coord.y-12*LODarea, 0)       ),
				 D3DXVECTOR2( MIN(coord.x+12*LODarea, seg_x-1), MIN(coord.y+12*LODarea, seg_y-1) )
			 );
*/

	//restore
	pDirect3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW);

	return S_OK;

}


/*
	// 3: draw!
	for( m = MAX(0, coord.x-area); m < MIN(coord.x+area, seg_x-1); m++) //rows
		for( n = MAX(0, coord.y-area); n < MIN(coord.y+area, seg_y-1); n++) //cols
		{
			i = m + n*(seg_x-1);
			if(FAILED(pDirect3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 6, i*6, 2))) return E_FAIL;
		}
*/

// texture separated drawing method
/*
	for(j = 0; j < texTable->elementCount; j++)
	{
		//set texture
		if( msg != 1 )
		{
			if(FAILED(m_pEffect->SetTexture( "tColorMap", texTable->ppTex[j]))) return E_FAIL;
			if(FAILED(m_pEffect->SetTexture( "tBumpMap", texTable->ppBumpTex[j]))) return E_FAIL;
		}

		//all faces
		for( m = MAX(0, coord.x-area); m < MIN(coord.x+area, seg_x-1); m++) //rows
			for( n = MAX(0, coord.y-area); n < MIN(coord.y+area, seg_y-1); n++) //cols
			{
				i = m + n*(seg_x-1);
				if(colors[i] == texTable->colors[j])
				{
					if(FAILED(pDirect3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 6, i*6, 2))) return E_FAIL;					
				}
			}
	}
*/



	//simplify draw
/*
	if(FAILED(m_pEffect->SetTexture( "tColorMap", texTable->ppTex[0]))) return E_FAIL;
	if(FAILED(m_pEffect->SetTexture( "tBumpMap", texTable->ppBumpTex[0]))) return E_FAIL;
	if(FAILED(pDirect3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_numVert, 0, m_numFaces))) return E_FAIL;
*/


/*
	for( i = 0, j = 0; i < m_numFaces / 2; i++ )
	{

		//complicated if - test of area and LOD
		if( qu(i%(seg_x-1) - (int)coord.x) + qu(i/(seg_x-1) - (int)coord.y) < qu(area) )
		{

			//usual indexbuffer
			if(FAILED(pDirect3DDevice->SetIndices(pBufferIndex))) return E_FAIL;

			//set textures
			if( (msg != 1) &&
				(j == 0 || colors[i] != colors[j]) )
			{
				j = i;

				for( int ctr = 0; ctr < (int)texTable->elementCount; ctr++)
				{	
					if(texTable->colors[ctr] == colors[i])
					{
						if(FAILED(m_pEffect->SetTexture ( "tColorMap", texTable->ppTex[ctr]))) return E_FAIL;
						else if(FAILED(m_pEffect->SetTexture ( "tBumpMap", texTable->ppBumpTex[ctr]))) return E_FAIL;
						else break;
					}
				}
				if(ctr == (int)texTable->elementCount)
				{
					if(FAILED(m_pEffect->SetTexture( "tColorMap", texTable->ppTex[0]))) return E_FAIL;
					if(FAILED(m_pEffect->SetTexture( "tBumpMap", texTable->ppBumpTex[0]))) return E_FAIL;
				}
			}

			//draw
			if(FAILED(pDirect3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 6, i*6, 2))) return E_FAIL;

		}
	
		// LOD terrain
		
		else if( ((i%(seg_x-1)%2 == 0) && (i/(seg_x-1)%2 == 0)) )
		{
			int nx = i%(seg_x-1)/2;
			int ny = i/(seg_x-1)/2;


			//LOD indexbuffer
			if(FAILED(pDirect3DDevice->SetIndices(pBufferIndexLOD))) return E_FAIL;

			//set textures
			if( (msg != 1) &&
				(j == 0 || colors[i] != colors[j]) )
			{
				j = i;

				for( int ctr = 0; ctr < (int)texTable->elementCount; ctr++)
				{	
					if(texTable->colors[ctr] == colors[i])
					{
						if(FAILED(m_pEffect->SetTexture ( "tColorMap", texTable->ppTex[ctr]))) return E_FAIL;
						else if(FAILED(m_pEffect->SetTexture ( "tBumpMap", texTable->ppBumpTex[ctr]))) return E_FAIL;
						else break;
					}
				}
				if(ctr == (int)texTable->elementCount)
				{
					if(FAILED(m_pEffect->SetTexture( "tColorMap", texTable->ppTex[0]))) return E_FAIL;
					if(FAILED(m_pEffect->SetTexture( "tBumpMap", texTable->ppBumpTex[0]))) return E_FAIL;
				}
			}

			//draw
			if(FAILED(pDirect3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 6, (ny*((seg_x-1)/2)+nx)*6, 2))) return E_FAIL;
		}
		
	}
*/


//-------------------------------------------------------------
// Name:
// Desc:
//-------------------------------------------------------------
HRESULT CTerrain::Release()
{
	if( pBufferIndex  != NULL)
        pBufferIndex->Release(); 

    if( pBufferVertex  != NULL)
        pBufferVertex->Release();

	if( pBufferIndexLOD2 != NULL)
		pBufferIndexLOD2->Release();
	
	if( pBufferIndexLOD4 != NULL)
		pBufferIndexLOD4->Release();

	if( pBufferIndexLOD8 != NULL)
		pBufferIndexLOD8->Release();

	if( pBufferIndexLOD16 != NULL)
		pBufferIndexLOD16->Release();
	
	return S_OK;
}

