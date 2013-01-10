//------------------------------------------------------
// CLASS CMODEL
// Desc: using for loading and processing
//		 skinned meshes into program
//------------------------------------------------------

#include "cmodel.h"


//-----------------------------------------------------------------------------
// Name:
// Desc: little help function (have no actual use) 
//-----------------------------------------------------------------------------
HRESULT LogAnimSetInfo(AnimSetInfo* asInfo)
{
	char string[512];
	sprintf(string, "ASInfo logged: %d, %d, %d, %d, %d, %d, %d, %d. %s, %f, %s, %f\n",
					(int)asInfo->ap_bIsAnimPass,
					(int)asInfo->bAnimPeriodPassed,
					(int)asInfo->ap_BreakTime,
					(int)asInfo->ap_StartTime,
					(int)asInfo->ap_CurTime,
					(int)asInfo->ap_NumOfSet1,
					(int)asInfo->ap_NumOfSet2,
					(int)asInfo->ap_fTime,
					asInfo->AnimSetName,
					(float)asInfo->AnimTime,
					asInfo->StockAnimationSet,
					(float)asInfo->StockTime
					);
	LogPlease(string);

	return S_OK;
}


//-----------------------------------------------------------------------------
// Name: AllocateName()
// Desc: Allocates memory for a string to hold the name of a frame or mesh
//-----------------------------------------------------------------------------
HRESULT AllocateName( LPCSTR Name, LPSTR *pNewName )
{
    UINT cbLength;

    if( Name != NULL )
    {
        cbLength = (UINT)strlen(Name) + 1;
        *pNewName = new CHAR[cbLength];
        if (*pNewName == NULL)
            return E_OUTOFMEMORY;
        memcpy( *pNewName, Name, cbLength*sizeof(CHAR) );
    }
    else
    {
        *pNewName = NULL;
    }

    return S_OK;
}





//-----------------------------------------------------------------------------
// Name: CAllocateHierarchy::CreateFrame()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CAllocateHierarchy::CreateFrame(LPCSTR Name, LPD3DXFRAME *ppNewFrame)
{
    HRESULT hr = S_OK;
    D3DXFRAME_DERIVED *pFrame;

    *ppNewFrame = NULL;

    pFrame = new D3DXFRAME_DERIVED;
    if (pFrame == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

    hr = AllocateName(Name, &pFrame->Name);
    if (FAILED(hr))
        goto e_Exit;

    // initialize other data members of the frame
    D3DXMatrixIdentity(&pFrame->TransformationMatrix);
    D3DXMatrixIdentity(&pFrame->CombinedTransformationMatrix);

    pFrame->pMeshContainer = NULL;
    pFrame->pFrameSibling = NULL;
    pFrame->pFrameFirstChild = NULL;

    *ppNewFrame = pFrame;
    pFrame = NULL;
e_Exit:
    delete pFrame;
    return hr;
}



//-----------------------------------------------------------------------------
// Name: CAllocateHierarchy::CreateMeshContainer()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CAllocateHierarchy::CreateMeshContainer(THIS_ 
												LPCSTR Name, 
												CONST D3DXMESHDATA *pMeshData,
												CONST D3DXMATERIAL *pMaterials, 
												CONST D3DXEFFECTINSTANCE *pEffectInstances, 
												DWORD NumMaterials, 
												CONST DWORD *pAdjacency, 
												LPD3DXSKININFO pSkinInfo, 
												LPD3DXMESHCONTAINER *ppNewMeshContainer)
{
    HRESULT hr;
    D3DXMESHCONTAINER_DERIVED *pMeshContainer = NULL;
    UINT NumFaces;
    UINT iMaterial;
    DWORD iBone, cBones;
    LPDIRECT3DDEVICE9 pd3dDevice = NULL;

    LPD3DXMESH pMesh = NULL;

    *ppNewMeshContainer = NULL;



    // this sample does not handle patch meshes, so fail when one is found
    if (pMeshData->Type != D3DXMESHTYPE_MESH)
    {
        hr = E_FAIL;
        goto e_Exit;
    }


    // get the pMesh interface pointer out of the mesh data structure
    pMesh = pMeshData->pMesh;



    // this sample does not FVF compatible meshes, so fail when one is found
    if (pMesh->GetFVF() == 0)
    {
        hr = E_FAIL;
        goto e_Exit;
    }



    // allocate the overloaded structure to return as a D3DXMESHCONTAINER
    pMeshContainer = new D3DXMESHCONTAINER_DERIVED;
    if (pMeshContainer == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }
    memset(pMeshContainer, 0, sizeof(D3DXMESHCONTAINER_DERIVED));


    // make sure and copy the name.  All memory as input belongs to caller, interfaces can be addref'd though
    hr = AllocateName(Name, &pMeshContainer->Name);
    if (FAILED(hr))
        goto e_Exit;        

    pMesh->GetDevice(&pd3dDevice);
    NumFaces = pMesh->GetNumFaces();




    // if no normals are in the mesh, add them
    if (!(pMesh->GetFVF() & D3DFVF_NORMAL))
    {
        pMeshContainer->MeshData.Type = D3DXMESHTYPE_MESH;

        // clone the mesh to make room for the normals
        hr = pMesh->CloneMeshFVF( pMesh->GetOptions(), 
                                    pMesh->GetFVF() | D3DFVF_NORMAL, 
                                    pd3dDevice, &pMeshContainer->MeshData.pMesh );
        if (FAILED(hr))
            goto e_Exit;

        // get the new pMesh pointer back out of the mesh container to use
        // NOTE: we do not release pMesh because we do not have a reference to it yet
        pMesh = pMeshContainer->MeshData.pMesh;

        // now generate the normals for the pmesh
        D3DXComputeNormals( pMesh, NULL );
    }
    else  // if no normals, just add a reference to the mesh for the mesh container
    {
        pMeshContainer->MeshData.pMesh = pMesh;
        pMeshContainer->MeshData.Type = D3DXMESHTYPE_MESH;

        pMesh->AddRef();
    }
        




    // allocate memory to contain the material information.  This sample uses
    //   the D3D9 materials and texture names instead of the EffectInstance style materials
    pMeshContainer->NumMaterials = MAX(1, NumMaterials);
    pMeshContainer->pMaterials = new D3DXMATERIAL[pMeshContainer->NumMaterials];
    
	pMeshContainer->ppTextures = new LPDIRECT3DTEXTURE9[pMeshContainer->NumMaterials];
	pMeshContainer->ppBumpTextures = new LPDIRECT3DTEXTURE9[pMeshContainer->NumMaterials];
	

    pMeshContainer->pAdjacency = new DWORD[NumFaces*3];
    if ((pMeshContainer->pAdjacency == NULL) || (pMeshContainer->pMaterials == NULL))
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

    memcpy(pMeshContainer->pAdjacency, pAdjacency, sizeof(DWORD) * NumFaces*3);
    memset(pMeshContainer->ppTextures, 0, sizeof(LPDIRECT3DTEXTURE9) * pMeshContainer->NumMaterials);
	memset(pMeshContainer->ppBumpTextures, 0, sizeof(LPDIRECT3DTEXTURE9) * pMeshContainer->NumMaterials);


    // if materials provided, copy them
    if (NumMaterials > 0)            
    {

        memcpy(pMeshContainer->pMaterials, pMaterials, sizeof(D3DXMATERIAL) * NumMaterials);

        for (iMaterial = 0; iMaterial < NumMaterials; iMaterial++)
        {
            if (pMeshContainer->pMaterials[iMaterial].pTextureFilename != NULL)
            {
                //TCHAR strTexturePath[MAX_PATH] = _T("data/textures");
                //DXUtil_FindMediaFileCb( strTexturePath, sizeof(strTexturePath), pMeshContainer->pMaterials[iMaterial].pTextureFilename );
                char strTexturePath[512];
				WCHAR wstrTexturePath[512];
				
				//init texture
				sprintf(strTexturePath, "data/textures/%s", pMeshContainer->pMaterials[iMaterial].pTextureFilename);
				MultiByteToWideChar( CP_ACP, 0, strTexturePath, -1, wstrTexturePath, 512 );
				if( FAILED( D3DXCreateTextureFromFile( pd3dDevice, wstrTexturePath, 
                                                        &pMeshContainer->ppTextures[iMaterial] ) ) )
				{
					HTMLLog("\t!!!failed to create texture %s\n", strTexturePath);
                    pMeshContainer->ppTextures[iMaterial] = NULL;
				}

				//init bump texture
				sprintf(strTexturePath, "data/textures/n_%s", pMeshContainer->pMaterials[iMaterial].pTextureFilename);
				MultiByteToWideChar( CP_ACP, 0, strTexturePath, -1, wstrTexturePath, 512 );
				if( FAILED( D3DXCreateTextureFromFile( pd3dDevice, wstrTexturePath, 
                                                        &pMeshContainer->ppBumpTextures[iMaterial] ) ) )
				{
					HTMLLog("\t!!!failed to create bump texture %s\n", strTexturePath);
					pMeshContainer->ppBumpTextures[iMaterial] = NULL;
				}
                 

                // don't remember a pointer into the dynamic memory, just forget the name after loading
                pMeshContainer->pMaterials[iMaterial].pTextureFilename = NULL;
            }
        }

    }
    else // if no materials provided, use a default one
    {

        pMeshContainer->pMaterials[0].pTextureFilename = NULL;
        memset(&pMeshContainer->pMaterials[0].MatD3D, 0, sizeof(D3DMATERIAL9));
        pMeshContainer->pMaterials[0].MatD3D.Diffuse.r = 0.5f;
        pMeshContainer->pMaterials[0].MatD3D.Diffuse.g = 0.5f;
        pMeshContainer->pMaterials[0].MatD3D.Diffuse.b = 0.5f;
        pMeshContainer->pMaterials[0].MatD3D.Specular = pMeshContainer->pMaterials[0].MatD3D.Diffuse;

    }



    // if there is skinning information, save off the required data and then setup for HW skinning
    if (pSkinInfo != NULL)
    {
        // first save off the SkinInfo and original mesh data
        pMeshContainer->pSkinInfo = pSkinInfo;
        pSkinInfo->AddRef();

        pMeshContainer->pOrigMesh = pMesh;
        pMesh->AddRef();

        // Will need an array of offset matrices to move the vertices from the figure space to the bone's space
        cBones = pSkinInfo->GetNumBones();
        pMeshContainer->pBoneOffsetMatrices = new D3DXMATRIX[cBones];
        if (pMeshContainer->pBoneOffsetMatrices == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto e_Exit;
        }


        // get each of the bone offset matrices so that we don't need to get them later
        for (iBone = 0; iBone < cBones; iBone++)
        {
            pMeshContainer->pBoneOffsetMatrices[iBone] = *(pMeshContainer->pSkinInfo->GetBoneOffsetMatrix(iBone));
        }

        // GenerateSkinnedMesh will take the general skinning information and transform it to a HW friendly version
        hr = cMod->GenerateSkinnedMesh(pMeshContainer);
        if (FAILED(hr))
            goto e_Exit;
    }



    *ppNewMeshContainer = pMeshContainer;
    pMeshContainer = NULL;



e_Exit:
    SAFE_RELEASE(pd3dDevice);

    // call Destroy function to properly clean up the memory allocated 
    if (pMeshContainer != NULL)
    {
        DestroyMeshContainer(pMeshContainer);
    }

    return hr;
}




//-----------------------------------------------------------------------------
// Name: CAllocateHierarchy::DestroyFrame()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CAllocateHierarchy::DestroyFrame(LPD3DXFRAME pFrameToFree) 
{
    SAFE_DELETE_ARRAY( pFrameToFree->Name );
    SAFE_DELETE( pFrameToFree );
    return S_OK; 
}




//-----------------------------------------------------------------------------
// Name: CAllocateHierarchy::DestroyMeshContainer()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CAllocateHierarchy::DestroyMeshContainer(LPD3DXMESHCONTAINER pMeshContainerBase)
{
    UINT iMaterial;
    D3DXMESHCONTAINER_DERIVED *pMeshContainer = (D3DXMESHCONTAINER_DERIVED*)pMeshContainerBase;

    SAFE_DELETE_ARRAY( pMeshContainer->Name );
    SAFE_DELETE_ARRAY( pMeshContainer->pAdjacency );
    SAFE_DELETE_ARRAY( pMeshContainer->pMaterials );
    SAFE_DELETE_ARRAY( pMeshContainer->pBoneOffsetMatrices );

    // release all the allocated textures
    if (pMeshContainer->ppTextures != NULL)
    {
        for (iMaterial = 0; iMaterial < pMeshContainer->NumMaterials; iMaterial++)
        {
            SAFE_RELEASE( pMeshContainer->ppTextures[iMaterial] );
        }
    }

    if (pMeshContainer->ppBumpTextures != NULL)
    {
        for (iMaterial = 0; iMaterial < pMeshContainer->NumMaterials; iMaterial++)
        {
            SAFE_RELEASE( pMeshContainer->ppBumpTextures[iMaterial] );
        }
    }


    SAFE_DELETE_ARRAY( pMeshContainer->ppTextures );
	SAFE_DELETE_ARRAY( pMeshContainer->ppBumpTextures )

    SAFE_DELETE_ARRAY( pMeshContainer->ppBoneMatrixPtrs );
    SAFE_RELEASE( pMeshContainer->pBoneCombinationBuf );
    SAFE_RELEASE( pMeshContainer->MeshData.pMesh );
    SAFE_RELEASE( pMeshContainer->pSkinInfo );
    SAFE_RELEASE( pMeshContainer->pOrigMesh );
    SAFE_DELETE( pMeshContainer );
    return S_OK;
}








CModel::CModel()
{

	//base
	m_pFrameRoot = NULL;
	m_pAnimController = NULL;
	m_pBoneMatrices = NULL;
	m_pBoneMatricesLocal = NULL;
	m_NumBoneMatricesMax = 0;

	//cm = NULL;


	//bools
	ASI.ap_bIsAnimPass = FALSE;
	bAttached = FALSE;
	bAttacher = FALSE;
	dublicate = FALSE;

	bFree = FALSE;
	
	//construction stuff
	numOfCm = 0;


	//message
	msg = 0;


	//physx
	object = NULL;
	controller = NULL;

	height = -999.f;

}





BOOL						CModel::IsAnimPass() { return ASI.ap_bIsAnimPass; }
//int							CModel::GetNumOfCM() { return numOfCm; }
HRESULT						CModel::SetCoordinate(D3DXVECTOR3 newCoord) { coordinate = D3DXVECTOR3(newCoord); return S_OK; }
HRESULT						CModel::SetAngle(D3DXVECTOR3 newAngle) {angle = D3DXVECTOR3(newAngle); return S_OK; }
LPD3DXANIMATIONCONTROLLER	CModel::GetAnimController(void) { return m_pAnimController; }
D3DXMATRIXA16*				CModel::GetMatWorld() { return &matWorld; }
HRESULT						CModel::SetMatWorld(D3DXMATRIXA16* MatWorldIn) { matWorld = *MatWorldIn; return S_OK; }




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CModel::Init(LPDIRECT3DDEVICE9		g_pd3dDevice,
					 D3DCAPS9				g_d3dCaps,
					 LPCWSTR				strMeshPath,
					 LPD3DXEFFECT			g_pEffect,
					 CModel*				dublicateModel)
{
	HRESULT    hr;
	int i;
	int iBone;

    //init log report
	char string[256];
	HTMLLog("\n...Mesh initializing: id:%d %s ; Type = ", id, meshPath);
	
	if(MType == BASE) LogPlease("BASE");
	else if(MType == CLONE) LogPlease("CLONE");
	else LogPlease("OTHER");
	if(dublicate) LogPlease(" DUBLICATE");



	//id3ddevice9 ptr
	m_pd3dDevice = g_pd3dDevice;
	m_d3dCaps = g_d3dCaps;


	//setting effect pointer
	m_pEffect = g_pEffect;


	//skinned mesh
	if( !dublicate ) //then initialize
	{

		CAllocateHierarchy Alloc(this);	
		hr = D3DXLoadMeshHierarchyFromX(strMeshPath,
										D3DXMESH_MANAGED, 
										m_pd3dDevice, 
										&Alloc, 
										NULL, 
										&m_pFrameRoot, 
										&m_pAnimController);
		if (FAILED(hr))
			return hr;

		hr = SetupBoneMatrixPointers(m_pFrameRoot);
		if (FAILED(hr))
			return hr;

	}
	else //if dublicate then just copy pointers
	{

		m_pFrameRoot = dublicateModel->m_pFrameRoot;
		m_pAnimController = dublicateModel->m_pAnimController;
		m_pBoneMatrices = dublicateModel->m_pBoneMatrices;
		m_NumBoneMatricesMax = dublicateModel->m_NumBoneMatricesMax;

	}


	hr = D3DXFrameCalculateBoundingSphere(m_pFrameRoot, &m_vObjectCenter, &m_fObjectRadius);
	if (FAILED(hr))
		return hr;




	LPD3DXSKININFO skininfo = FindFirstSkinInfo(m_pFrameRoot);

	// ordinary
	if( skininfo == NULL )
	{
		type = ORDINARY;
		LogPlease(" ORDINARY \n");

		numFrames = GetNumOfFrames(m_pFrameRoot);
		m_pBoneMatricesLocal = new D3DXMATRIXA16[ numFrames ];
		m_pBoneMatricesLocalNames = new char*[ numFrames ];


		HTMLLog("Num of Bones - 0, Num of Frames - %d\n", numFrames); 

		//then fill massive with remaining frames
		LPD3DXFRAME pFrame;
		for( i=0; i < numFrames; i++)
		{
			LPD3DXFRAME pFrame = GetFrameByNum(m_pFrameRoot, i);

			m_pBoneMatricesLocalNames[i] = new char[256];
			strcpy( m_pBoneMatricesLocalNames[i], pFrame->Name );
			HTMLLog( "\t\tFrame %d: %s \n", i, m_pBoneMatricesLocalNames[i]);
			iBone++;
		}
	}

	// skinned
	else
	{
		type = SKINNED;
		LogPlease(" SKINNED \n");

		numFrames = GetNumOfFrames(m_pFrameRoot);
		m_pBoneMatricesLocal = new D3DXMATRIXA16[ numFrames ];
		m_pBoneMatricesLocalNames = new char*[ numFrames ];


		HTMLLog("Num of Bones - %d, Num of Frames - %d\n", skininfo->GetNumBones(), numFrames);

		//filling first with bones
		for( iBone=0; iBone < skininfo->GetNumBones(); iBone++ )
		{	
			m_pBoneMatricesLocalNames[iBone] = new char[256];
			strcpy( m_pBoneMatricesLocalNames[iBone], skininfo->GetBoneName(iBone) );
			HTMLLog( "\t\tBone %d: %s \n", iBone, m_pBoneMatricesLocalNames[iBone]); 
		}

		//then fill massive with remaining frames
		LPD3DXFRAME pFrame;
		for( i=0; i < numFrames; i++)
		{
			LPD3DXFRAME pFrame = GetFrameByNum(m_pFrameRoot, i);

			if( GetBoneNumberByName(pFrame->Name) == -1 )
			{
				m_pBoneMatricesLocalNames[iBone] = new char[256];
				strcpy( m_pBoneMatricesLocalNames[iBone], pFrame->Name );
				HTMLLog( "\t\tBone %d: %s \n", iBone, m_pBoneMatricesLocalNames[iBone]);
				iBone++;
			}
			
		}


	}
	

	//this will be removed
	//additional set
	ASI.AnimTime = 0;

	return S_OK;
}



//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT	CModel::InitPhysX(NxPhysicsSDK* gPhysicsSDK, NxScene* gScene, NxControllerManager*	gManager, char flag)
{
	int i;
	LogPlease("\t\t<kbd>...Init PhysX()...</kbd>\n");

	//checkups
	if( gPhysicsSDK != NULL ) mPhysicsSDK = gPhysicsSDK;
	if( gScene != NULL ) mScene = gScene;
	if( gManager != NULL ) mManager = gManager;


	if( flag == OT_CHARACTER )
	{
		CreatePhysXBipedObject();
		
		HTMLLog("\t\t<kbd>Creating physX objects for OT_CHARACTER</kbd> %f\n", height);
		controller = new MyCharacterController(		  mManager,
													  mScene,  
													  NxVec3(coordinate.x, coordinate.y, coordinate.z), 
													  40.f, 
													  130.f );
		object = controller->GetCharacterActor();
		SetActorCollisionGroup( object, GROUP_CONTROLLER );
		physxtype = FT_CHARACTER;

		usrdata.type = UI_SKINMESH;
		usrdata.id = id;
		usrdata.bHit = FALSE;

		object->userData = &usrdata;
		for( i = 0; i < NumOfBiped; i++ )
		{
			biped[i]->userData = &usrdata;
		}
	}
	else if( flag == OT_OBJECT )
	{
		HTMLLog("\t\t<kbd>Creating physX objects for OT_OBJECT</kbd>\n");
		CreatePhysXObject();

		usrdata.type = UI_OBJECT;
		usrdata.id = id;
		object->userData = &usrdata;
	}
	else if( flag == OT_WEAPON )
	{
		HTMLLog("\t\t<kbd>Creating physX objects for OT_WEAPON</kbd>\n");
		CreatePhysXObject();

		usrdata.type = UI_WEAPON;
		usrdata.id = id;
		usrdata.attacherid = &attacherId;
		object->userData = &usrdata;
	}

	CModelUserData* userdata = (CModelUserData*)object->userData;
	HTMLLog("type %d, id - %d\n", (int)userdata->type, (int)userdata->id); 

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
int CModel::GetBoneNumberByName(LPCSTR boneName)
{
	int i;

	LPD3DXSKININFO pSkinInfo = FindFirstSkinInfo(m_pFrameRoot);
	if(pSkinInfo == NULL) { LogPlease("CModel::GetBoneNumberByName() error: can't find skin info\n"); return -1; }

	for(i=0; i < (int)pSkinInfo->GetNumBones(); i++)
		if(!strcmp(boneName, pSkinInfo->GetBoneName(i))) return i;


	return -1;
}


//-----------------------------------------------------------------------------
// Name: 
// Desc: sudden free function =)
//-----------------------------------------------------------------------------
HRESULT CModel::SetFree(bool bFreeFlag)
{
	int i;
	if( bFreeFlag == TRUE )
	{
		bFree = TRUE;

		//update bipeds
		for( i = 0; i < NumOfBiped; i++ ) biped[i]->clearBodyFlag( NX_BF_KINEMATIC );
		
		//for( i = 0; i < NumOfJoint; i++ ) HTMLLog("%d - %s\n", i, jointNames[i]);
		//update joints
		for( i = 0; i < NumOfJoint; i++ )
		{
			D3DXVECTOR3 pOutScale;
			D3DXVECTOR3 pOutTranslation;
			D3DXQUATERNION pOutRotation;
			int frameindex;

			frameindex = GetFrameIndexByName( jointNames[i] );
			if( frameindex == -1 ) HTMLLog("Error! cannot find %s\n", jointNames[i] );

			D3DXMatrixDecompose( &pOutScale, &pOutRotation, &pOutTranslation, &m_pBoneMatricesLocal[ frameindex ] );
			//pOutTranslation += coordinate;

			//HTMLLog("Anchored: %f %f %f\n", pOutTranslation.x, pOutTranslation.y, pOutTranslation.z);
			joint[i]->setGlobalAnchor( DxVec3ToNxVec3( pOutTranslation ) );
		}
	}
	else
	{
		bFree = FALSE;
	}

	return S_OK;
}



//-----------------------------------------------------------------------------
// Name: Attach block
// Desc:
//-----------------------------------------------------------------------------
HRESULT CModel::Detach()
{
	bAttacher = FALSE;
	bAttached = FALSE;
	return S_OK;
}


HRESULT CModel::Attach(CModel* cm, LPCSTR boneName)
{
	int i;

	if(type != SKINNED || cm->type != ORDINARY) return E_FAIL;
	
	/*
	D3DXFRAME_DERIVED* pFrame = (D3DXFRAME_DERIVED*)D3DXFrameFind(m_pFrameRoot, boneName);
	if(pFrame == NULL)
	{
		LogPlease("Attach error: unable to find bone with specified name\n");
		return E_FAIL;
	}
	*/

	//ordinary constants
	for(i=0; i < numFrames; i++)
	{
		if( !strcmp(m_pBoneMatricesLocalNames[i], boneName) )
		{
			cm->a_TransformMatrix = &m_pBoneMatricesLocal[i];
			break;
		}
	}
	if(i == numFrames) LogPlease("Attach error: cannot find specified frame\n");
	
	cm->bAttached = TRUE;
	cm->attacherId = id;

	//skinned constants
	bAttacher = TRUE;
	attachedId = cm->id;


	return E_FAIL;
}



//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CModel::SetAnimControllerTime(double time)
{
	if( //animcontroller check
	   ASI.ap_bIsAnimPass == TRUE
	   ) return S_OK;

	if (m_pAnimController == NULL) return E_FAIL;

	m_pAnimController->SetTrackPosition(0, time);
    m_pAnimController->AdvanceTime(0, NULL);
	ASI.AnimTime = time;

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
double CModel::GetAnimControllerTime()
{
	if( m_pAnimController != NULL ) return m_pAnimController->GetTime();
	else return NULL;
}


//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CModel::SetTrackAnimationSetByName(LPCSTR AnimationSetName,
										   int TrackNum, BOOL TrackEnable, float TrackSpeed, float TrackWeight,
										   float PriorityBlend)
{
	//vars
	DWORD i; 

	for( i=0; i< m_pAnimController->GetNumAnimationSets(); i++)
	{
		LPD3DXANIMATIONSET AnimSet;
		m_pAnimController->GetAnimationSet(i, &AnimSet);
		if(!stricmp(AnimationSetName, AnimSet->GetName()) )
		{
			m_pAnimController->SetTrackAnimationSet(TrackNum, AnimSet);
			m_pAnimController->SetTrackEnable(TrackNum, TrackEnable);
			m_pAnimController->SetTrackSpeed(TrackNum, TrackSpeed); 
			m_pAnimController->SetTrackWeight(TrackNum, TrackWeight);

			m_pAnimController->SetPriorityBlend(PriorityBlend);
			break;
		}
	}

	if( i == m_pAnimController->GetNumAnimationSets() ) return E_FAIL;
	else return S_OK;
}


//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
char* CModel::GetTrackAnimationSetName( int TrackNum )
{
	if(!m_pAnimController) return NULL;

	LPD3DXANIMATIONSET AnimSet;
	m_pAnimController->GetTrackAnimationSet(TrackNum, &AnimSet);
	
	return (char*)AnimSet->GetName(); 
}



//-----------------------------------------------------------------------------
// Name:
// Desc: equivavelent for ordinary meshes
//-----------------------------------------------------------------------------
HRESULT CModel::StoreFrameMatrices(CModel* modelIn)
{

	UINT iBone;
	D3DXFRAME_DERIVED *pInFrame;

	if( modelIn == NULL ) modelIn = this;

	//warning!
	if( modelIn->type != ORDINARY || type != ORDINARY ) { LogPlease("StoreFrameMatrices(): error: skinned meshes\n"); exit(1); }

	
	//now there are a couple of fames, which are not in skininfo, so we will add them manually
	for(iBone = 0; iBone < numFrames; iBone++)
	{
		D3DXFRAME_DERIVED *pFrame = (D3DXFRAME_DERIVED*)D3DXFrameFind(modelIn->m_pFrameRoot, m_pBoneMatricesLocalNames[iBone]);
		if( pFrame == NULL ) { HTMLLog(" StoreFrameMatrices() - some error occurs...\n"); return E_FAIL; }
		m_pBoneMatricesLocal[iBone] = pFrame->CombinedTransformationMatrix;
	}
	//HTMLLog("%d: %d %d\n", id, cBones, m_NumBoneMatricesMax);


	return S_OK;
}



//-----------------------------------------------------------------------------
// Name:
// Desc: if there are more then 1 skininfo, func will not work properly
//-----------------------------------------------------------------------------
HRESULT CModel::StoreBoneMatrices(CModel* modelIn)
{

	UINT iBone, cBones;
	D3DXFRAME_DERIVED *pInFrame;

	if( modelIn == NULL ) modelIn = this;

	//warning!
	if( modelIn->type != SKINNED || type != SKINNED ) { LogPlease("StoreBoneMatrices(): error: non-skinned meshes\n"); exit(1); }

	
	D3DXMESHCONTAINER_DERIVED* pInMeshContainer =
		(D3DXMESHCONTAINER_DERIVED*)FindFirstMeshContainerWithSkinInfo(modelIn->m_pFrameRoot);
	if(pInMeshContainer == NULL) { LogPlease("StoreBoneMatrices(): impossible error!"); exit(1); }



	//equaling
	cBones = pInMeshContainer->pSkinInfo->GetNumBones();
	for (iBone = 0; iBone < cBones; iBone++)
    {
		m_pBoneMatricesLocal[iBone] = *pInMeshContainer->ppBoneMatrixPtrs[iBone];
    }

	//now there are a couple of fames, which are not in skininfo, so we will add them manually
	for(; iBone < numFrames; iBone++)
	{
		D3DXFRAME_DERIVED *pFrame = (D3DXFRAME_DERIVED*)D3DXFrameFind(modelIn->m_pFrameRoot, m_pBoneMatricesLocalNames[iBone]);
		if( pFrame == NULL ) { HTMLLog(" StoreBoneMatrices() - some error occurs...\n"); return E_FAIL; }
		m_pBoneMatricesLocal[iBone] = pFrame->CombinedTransformationMatrix;
	}
	//HTMLLog("%d: %d %d\n", id, cBones, m_NumBoneMatricesMax);


	return S_OK;
}


//-----------------------------------------------------------------------------
// Name:
// Desc: if there are more then 1 skininfo, func will not work properly
//-----------------------------------------------------------------------------
HRESULT CModel::CopyBonesState(CModel* modelIn, D3DXMATRIX g_matView, D3DXMATRIX g_matProj)
{
	m_matView = g_matView;
	m_matProj = g_matProj;


	UINT iBone, cBones;
	D3DXFRAME_DERIVED *pInFrame;
	D3DXFRAME_DERIVED *pOutFrame;


	//warning!
	if(modelIn->type != SKINNED) { LogPlease("CopyBonesState error: non-skinned IN mesh\n"); exit(1); }
	if(type != SKINNED) { LogPlease("CopyBonesState error: non-skinned OUT mesh\n"); exit(1); }


	D3DXMESHCONTAINER_DERIVED* pOutMeshContainer =
		(D3DXMESHCONTAINER_DERIVED*)FindFirstMeshContainerWithSkinInfo(m_pFrameRoot);
	if(pOutMeshContainer == NULL) { LogPlease("Code error!\n"); exit(1); }
	
	D3DXMESHCONTAINER_DERIVED* pInMeshContainer =
		(D3DXMESHCONTAINER_DERIVED*)FindFirstMeshContainerWithSkinInfo(modelIn->m_pFrameRoot);
	if(pInMeshContainer == NULL) { LogPlease("Code error!\n"); exit(1); }


	//equaling
	/*
	cBones = pOutMeshContainer->pSkinInfo->GetNumBones();
	for (iBone = 0; iBone < cBones; iBone++)
    {
		pOutFrame = (D3DXFRAME_DERIVED*)D3DXFrameFind(m_pFrameRoot, pOutMeshContainer->pSkinInfo->GetBoneName(iBone));
        if (pOutFrame == NULL)	return E_FAIL;
		
		int bonenumber = modelIn->GetBoneNumberByName( pOutMeshContainer->pSkinInfo->GetBoneName(iBone) );
		
		if(bonenumber == -1)
		{
			pOutFrame->CombinedTransformationMatrix = modelIn->m_pBoneMatricesLocal[ GetFrameIndexByName( (char*)pOutMeshContainer->pSkinInfo->GetBoneName(iBone) )];
		}
		else pOutFrame->CombinedTransformationMatrix = modelIn->m_pBoneMatricesLocal[ bonenumber ];
    }

	for( ; iBone < modelIn->GetNumOfFrames(modelIn->m_pFrameRoot); iBone++)
	{
	}
	*/

	for(iBone = 0; iBone < numFrames; iBone++)
	{
		int index = modelIn->GetFrameIndexByName( m_pBoneMatricesLocalNames[iBone] );
		if(index == -1) 
		{ //HTMLLog("Missing bone drom parent: %s", m_pBoneMatricesLocalNames[iBone] ); 
		}
		else
		{
			m_pBoneMatricesLocal[iBone] = modelIn->m_pBoneMatricesLocal[index];
		}

	}

	//+++
	//if( modelIn->matWorld != NULL ) matWorld = modelIn->matWorld;


	return S_OK;
}



//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CModel::GetModelState(AnimSetInfo* animInfoIn)
{
		
	ASI.AnimSetName = animInfoIn->AnimSetName;
	ASI.AnimTime = animInfoIn->AnimTime;
	ASI.bAnimPeriodPassed = animInfoIn->bAnimPeriodPassed;

	ASI.StockTime = animInfoIn->StockTime;
	ASI.StockAnimationSet = animInfoIn->StockAnimationSet;


	//additional checkup - no analog in CModel* version
	if(MType == BASE)
	{
		SetTrackAnimationSetByName( ASI.AnimSetName, 0, TRUE, 1.f, 1.f, 1.f);
		m_pAnimController->SetTrackPosition(0, ASI.AnimTime);
		m_pAnimController->AdvanceTime( 0, NULL );
	}


	ASI.ap_bIsAnimPass = animInfoIn->ap_bIsAnimPass;

	ASI.ap_BreakTime = animInfoIn->ap_BreakTime;
	ASI.ap_CurTime = animInfoIn->ap_CurTime;
	ASI.ap_fTime = animInfoIn->ap_fTime;
	ASI.ap_StartTime = animInfoIn->ap_StartTime;
	ASI.ap_NumOfSet1 = animInfoIn->ap_NumOfSet1;
	ASI.ap_NumOfSet2 = animInfoIn->ap_NumOfSet2;


	return S_OK;
} 


//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CModel::SetModelState(AnimSetInfo* animInfoOut)
{

	
	animInfoOut->AnimSetName = ASI.AnimSetName;
	animInfoOut->AnimTime = ASI.AnimTime;
	animInfoOut->bAnimPeriodPassed = ASI.bAnimPeriodPassed;
	
	animInfoOut->StockAnimationSet = ASI.StockAnimationSet;
	animInfoOut->StockTime = ASI.StockTime;


	animInfoOut->ap_bIsAnimPass = ASI.ap_bIsAnimPass;

	animInfoOut->ap_BreakTime = ASI.ap_BreakTime;
	animInfoOut->ap_CurTime = ASI.ap_CurTime;
	animInfoOut->ap_fTime = ASI.ap_fTime;
	animInfoOut->ap_StartTime = ASI.ap_StartTime;
	animInfoOut->ap_NumOfSet1 = ASI.ap_NumOfSet1;
	animInfoOut->ap_NumOfSet2 = ASI.ap_NumOfSet2;


	return S_OK;


}


//-----------------------------------------------------------------------------
// Name:
// Desc: function must be called from CLONE object! modelIn must be BASE
//-----------------------------------------------------------------------------
HRESULT CModel::GetModelState(CModel* modelIn)
{
	//check if desc is properly set
	if(modelIn->MType != BASE || MType != CLONE) { LogPlease("GetModelState(CModel*) error!\n"); return E_FAIL; }

	//matrices
	if( modelIn->matWorld != NULL ) matWorld = modelIn->matWorld;
		
	ASI.AnimSetName = modelIn->GetTrackAnimationSetName(0);
	//ASI.AnimTime = modelIn->GetAnimControllerTime();
	ASI.AnimTime = modelIn->ASI.AnimTime;
	ASI.bAnimPeriodPassed = modelIn->ASI.bAnimPeriodPassed;

	ASI.StockTime = modelIn->ASI.StockTime;
	ASI.StockAnimationSet = modelIn->ASI.StockAnimationSet;



	ASI.ap_bIsAnimPass = modelIn->ASI.ap_bIsAnimPass;

	ASI.ap_BreakTime = modelIn->ASI.ap_BreakTime;
	ASI.ap_CurTime = modelIn->ASI.ap_CurTime;
	ASI.ap_fTime = modelIn->ASI.ap_fTime;
	ASI.ap_StartTime = modelIn->ASI.ap_StartTime;
	ASI.ap_NumOfSet1 = modelIn->ASI.ap_NumOfSet1;
	ASI.ap_NumOfSet2 = modelIn->ASI.ap_NumOfSet2;


	return S_OK;
}


//-----------------------------------------------------------------------------
// Name:
// Desc: function must be called from CLONE object! modelOut must be BASE
//-----------------------------------------------------------------------------
HRESULT CModel::SetModelState(CModel* modelOut)
{
	//check if desc is properly set
	if(modelOut->MType != BASE || MType != CLONE) { LogPlease("GetModelState() error!\n"); return E_FAIL; }


	//matrices
	if( matWorld != NULL ) modelOut->matWorld = matWorld;

	modelOut->SetTrackAnimationSetByName( ASI.AnimSetName, 0, TRUE, 1.f, 1.f, 1.f);
	modelOut->m_pAnimController->ResetTime();
	//modelOut->m_pAnimController->SetTrackPosition(0, ASI.AnimTime);
	modelOut->m_pAnimController->AdvanceTime( ASI.AnimTime, NULL );
	modelOut->m_pAnimController->SetTrackPosition(0, ASI.AnimTime);
	modelOut->ASI.bAnimPeriodPassed = ASI.bAnimPeriodPassed;
	
	modelOut->ASI.StockTime = ASI.StockTime;
	modelOut->ASI.StockAnimationSet = ASI.StockAnimationSet;


	modelOut->ASI.ap_bIsAnimPass = ASI.ap_bIsAnimPass;

	modelOut->ASI.ap_BreakTime = ASI.ap_BreakTime;
	modelOut->ASI.ap_CurTime = ASI.ap_CurTime;
	modelOut->ASI.ap_fTime = ASI.ap_fTime;
	modelOut->ASI.ap_StartTime = ASI.ap_StartTime;
	modelOut->ASI.ap_NumOfSet1 = ASI.ap_NumOfSet1;
	modelOut->ASI.ap_NumOfSet2 = ASI.ap_NumOfSet2;


	return S_OK;


}


//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CModel::CopyAnimationState(CModel* modelIn)
{

	//matrices
	if( modelIn->matWorld != NULL ) matWorld = modelIn->matWorld;
		
	SetTrackAnimationSetByName( modelIn->GetTrackAnimationSetName(0), 0, TRUE, 1.f, 1.f, 1.f);
	m_pAnimController->SetTrackPosition(0, modelIn->GetAnimControllerTime());
	m_pAnimController->AdvanceTime( 0, NULL );
	ASI.AnimTime = modelIn->GetAnimControllerTime();


	//if animpass
	if(modelIn->ASI.ap_bIsAnimPass == TRUE)
	{
		ASI.ap_bIsAnimPass = TRUE;

		ASI.ap_BreakTime = modelIn->ASI.ap_BreakTime;
		ASI.ap_CurTime = modelIn->ASI.ap_CurTime;
		ASI.ap_fTime = modelIn->ASI.ap_fTime;
		ASI.ap_StartTime = modelIn->ASI.ap_StartTime;
		ASI.ap_NumOfSet1 = modelIn->ASI.ap_NumOfSet1;
		ASI.ap_NumOfSet2 = modelIn->ASI.ap_NumOfSet2;
	}

	else //simple animation
	{
		ASI.ap_bIsAnimPass = FALSE;
	}


	return S_OK;
}


//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CModel::UpdateMatrices(D3DXMATRIX g_matView, D3DXMATRIX g_matProj)
{
	m_matView = g_matView;
	m_matProj = g_matProj;


	//update matrices
	if( ASI.ap_bIsAnimPass == TRUE)
	{
		UpdateFrameMatricesAnimPass(m_pFrameRoot, &matWorld);
	}
	else
	{
		UpdateFrameMatrices(m_pFrameRoot, &matWorld);
	}


	//check
	if( m_pAnimController && MType != CLONE ) ASI.AnimTime = m_pAnimController->GetTime();

	return S_OK;
}



float CModel::GetHeight() { return GetBipedByName("Bip01_Head")->getGlobalPosition().y - GetBipedByName("Bip01_R_Foot")->getGlobalPosition().y; }

//-----------------------------------------------------------------------------
// Name: UpdateMatricesPhysX
// Desc: model gets matrix and coordinates from it's physX object
//-----------------------------------------------------------------------------
HRESULT CModel::UpdateBipedsPhysX()
{
	int i;
	D3DXMATRIXA16 physx_matrix;
	D3DXMATRIXA16 pmatrix; //result matrix
	NxVec3 nxvec3;

	D3DXVECTOR3 pOutScale;
	D3DXVECTOR3 pOutTranslation;
	D3DXQUATERNION pOutRotation;
	D3DXMATRIX matTransRot;

	if( bFree == FALSE )
	{
		for( i = 0; i < NumOfBiped; i++)
		{
			D3DXMatrixDecompose( &pOutScale, &pOutRotation, &pOutTranslation, bipedMatrix[i] );
			D3DXMatrixTransformation( &matTransRot, NULL, NULL, NULL, NULL, &pOutRotation, &pOutTranslation );

			NxMat34 matrix;
			matrix.setColumnMajor44( matTransRot.m );
			biped[i]->moveGlobalPose( matrix );
		}
	}
	else //if free
	{
		//special func for update
		UpdateBipedsPhysXByFrame( m_pFrameRoot, 0 );

		/* // new style
		for( i = 0; i < numFrames; i++ )
		{
			if( !strncmp( m_pBoneMatricesLocalNames[i], "Bip", 3) )
			{
				char name[256];
				sprintf( name, "CBP_%s", m_pBoneMatricesLocalNames[i]+6 );
				int bipindex = GetBipedIndexByName( name );
				//HTMLLog("\ngot biped index %d:%s for %d:%s", bipindex, name, i, m_pBoneMatricesLocalNames[i]);

				if( bipindex == -1 && i > 0 ) m_pBoneMatricesLocal[i] = m_pBoneMatricesLocal[ i-1 ];
				else
				{
					biped[ bipindex ]->getGlobalPose().getColumnMajor44( pmatrix );
					m_pBoneMatricesLocal[ i ] = pmatrix;
				}
			}
		}
		*/
		/* //old style
		for( i = 0; i < NumOfBiped; i++)
		{
			//matrix
			biped[i]->getGlobalPose().getColumnMajor44( pmatrix );

			//turn bip01 into cbp
			char name[256];
			sprintf(name, "Bip01_%s", bipedNames[i]+4);
			int frindex = GetFrameIndexByName( name );

			if( frindex == -1 ) HTMLLog("Error! cannot find frame with specified name: %s\n", name);
			else m_pBoneMatricesLocal[ frindex ] = pmatrix;
			//SetMatWorld( &pmatrix );
		}*/
	}

	return S_OK;
}



//-----------------------------------------------------------------------------
// Name: UpdateMatricesPhysX
// Desc: model gets matrix and coordinates from it's physX object
//-----------------------------------------------------------------------------
HRESULT CModel::UpdateMatricesPhysX( NxMat34* outmatrix, NxVec3* invector )
{

	D3DXMATRIXA16 physx_matrix;
	D3DXMATRIXA16 pmatrix; //result matrix
	NxVec3 nxvec3;

	if( outmatrix == NULL ) outmatrix = &object->getGlobalPose();
	if( invector == NULL ) invector = &object->getGlobalPosition();


	if( physxtype == FT_COL ) //from animation to physx
	{
		D3DXVECTOR3 pOutScale;
		D3DXVECTOR3 pOutTranslation;
		D3DXQUATERNION pOutRotation;
		D3DXMATRIX matTransRot;

		D3DXMatrixDecompose( &pOutScale, &pOutRotation, &pOutTranslation, objectMatrix );
		D3DXVECTOR3 boxdisplace;
		D3DXMatrixTransformation( &matTransRot, NULL, NULL, NULL, NULL, &pOutRotation, &pOutTranslation );

		NxMat34 matrix;
		matrix.setColumnMajor44( matTransRot.m );
		object->moveGlobalPose( matrix );
	}

	else //DOL || COL || CHARACTER
		 //from physx to animation
	{
		//coordinates
		nxvec3 = *invector;
		coordinate = D3DXVECTOR3( nxvec3.x, nxvec3.y, nxvec3.z);


		//matrix
		outmatrix->getColumnMajor44( pmatrix );

		// difference block - diff is vector between center of the physX model
		// (which is 0,0,0) and center of the model (or height of the model / 2 )
		// difference sets manually (yet)
		if( physxtype == FT_CHARACTER )
		{
			D3DXVECTOR3 diff = D3DXVECTOR3(0.f, -105.f, 0.f);
			float lowpoint = GetBipedByName("CBP_R_Foot")->getGlobalPosition().y;
			if( lowpoint > GetBipedByName("CBP_L_Foot")->getGlobalPosition().y ) lowpoint = GetBipedByName("CBP_L_Foot")->getGlobalPosition().y;
			lowpoint -= 5.f;
			float highpoint = GetBipedByName("CBP_Head")->getGlobalPosition().y + 5.f;

			//if( height == -999.f && fabs(highpoint - lowpoint) > 0.1f ) height = fabs(highpoint - lowpoint);
			//if( id == 0 )HTMLLog("%f\n", highpoint - lowpoint);
			//D3DXVECTOR3 diff = D3DXVECTOR3(0,0,0);
			diff -= D3DXVECTOR3( 0, (128.f - fabs(highpoint - lowpoint))*0.5f, 0);

			D3DXMatrixTranslation(&physx_matrix, diff.x, diff.y, diff.z);
			pmatrix = physx_matrix * pmatrix;
		}

		SetMatWorld( &pmatrix );
	}

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CModel::Update(double m_fElapsedTime)
{

	//calculating matrices
	D3DXMatrixTranslation( &matWorld, 
						   coordinate.x,
				           coordinate.y,
						   coordinate.z );
	D3DXMatrixRotationYawPitchRoll( &matRotation, angle.y, angle.x, angle.z);	 

	matWorld = matRotation * matWorld;



	//period test and animation mover (the one and only)
	if( m_pAnimController && MType != CLONE )
	{

		//1st step - simple period test
		LPD3DXANIMATIONSET AnimSet;
		m_pAnimController->GetTrackAnimationSet( 0, &AnimSet );

		if( m_pAnimController->GetTime() + m_fElapsedTime >= AnimSet->GetPeriod() && IsAnimPass() == FALSE)
		{
			ASI.bAnimPeriodPassed = TRUE;
		}
		else ASI.bAnimPeriodPassed = FALSE;


		//important thing!!! without this one last frame will be fake
		if( ASI.bAnimPeriodPassed == TRUE && OnceAnimation() == TRUE )
		{
			//do nothing actually
			//m_pAnimController->SetTrackPosition( 0, m_pAnimController->GetTime() - m_fElapsedTime );
			//m_pAnimController->ResetTime();
			m_pAnimController->AdvanceTime( 0, NULL );
		}
		else
		{
			//progress
			//m_pAnimController->SetTrackPosition( 0, m_pAnimController->GetTime() + m_fElapsedTime );
			m_pAnimController->AdvanceTime( m_fElapsedTime, NULL );
		}

		//set AnimTime constant
		ASI.AnimTime = m_pAnimController->GetTime();
		
	}



	return S_OK;
}



//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CModel::Render()
{
	//HTMLLog("--------------------------\nModel render! %d : %s\n", id, meshPath);
	DrawFrame(m_pFrameRoot);
	return S_OK;
}



//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CModel::Release()
{
	if( !dublicate )
	{
		CAllocateHierarchy Alloc(this);

		D3DXFrameDestroy(m_pFrameRoot, &Alloc);

		SAFE_RELEASE(m_pAnimController);
	}

	delete [] m_pBoneMatricesLocal;

	//don't work for some reason - check it later
	//if( controller != NULL ) { delete controller; controller = NULL; }

	return S_OK;
}


//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
LPD3DXMESHCONTAINER CModel::FindFirstMeshContainerWithSkinInfo(LPD3DXFRAME pFrame)
{
	LPD3DXMESHCONTAINER pFrameRet;
	D3DXMESHCONTAINER_DERIVED* pMeshContainer;

    pMeshContainer = (D3DXMESHCONTAINER_DERIVED*)pFrame->pMeshContainer;

    while (pMeshContainer != NULL)
    {
        if( pMeshContainer->pSkinInfo != NULL) return pMeshContainer;
        pMeshContainer = (D3DXMESHCONTAINER_DERIVED*)pMeshContainer->pNextMeshContainer;
    }

    if (pFrame->pFrameSibling != NULL)
    {
        pFrameRet = FindFirstMeshContainerWithSkinInfo(pFrame->pFrameSibling);
		if(pFrameRet != NULL) return pFrameRet;
    }

    if (pFrame->pFrameFirstChild != NULL)
    {
        pFrameRet = FindFirstMeshContainerWithSkinInfo(pFrame->pFrameFirstChild);
		if(pFrameRet != NULL) return pFrameRet;
    }
	

	return NULL;
}


//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
LPD3DXSKININFO CModel::FindFirstSkinInfo(LPD3DXFRAME pFrame)
{
	LPD3DXSKININFO pFrameRet;
	D3DXMESHCONTAINER_DERIVED* pMeshContainer;

    pMeshContainer = (D3DXMESHCONTAINER_DERIVED*)pFrame->pMeshContainer;

    while (pMeshContainer != NULL)
    {
        if( pMeshContainer->pSkinInfo != NULL) return pMeshContainer->pSkinInfo;
        pMeshContainer = (D3DXMESHCONTAINER_DERIVED*)pMeshContainer->pNextMeshContainer;
    }

    if (pFrame->pFrameSibling != NULL)
    {
        pFrameRet = FindFirstSkinInfo(pFrame->pFrameSibling);
		if(pFrameRet != NULL) return pFrameRet;
    }

    if (pFrame->pFrameFirstChild != NULL)
    {
        pFrameRet = FindFirstSkinInfo(pFrame->pFrameFirstChild);
		if(pFrameRet != NULL) return pFrameRet;
    }
	

	return NULL;
}

//-----------------------------------------------------------------------------
// Name: GenerateSkinnedMesh()
// Desc: Called either by CreateMeshContainer when loading a skin mesh, or when 
//       changing methods.  This function uses the pSkinInfo of the mesh 
//       container to generate the desired drawable mesh and bone combination 
//       table.
//-----------------------------------------------------------------------------
HRESULT CModel::GenerateSkinnedMesh(D3DXMESHCONTAINER_DERIVED *pMeshContainer)
{

    HRESULT hr = S_OK;

    if (pMeshContainer->pSkinInfo == NULL)
        return hr;

    SAFE_RELEASE( pMeshContainer->MeshData.pMesh );
    SAFE_RELEASE( pMeshContainer->pBoneCombinationBuf );

    
	//indexed skinning

		// Get palette size
        // First 9 constants are used for other data.  Each 4x3 matrix takes up 3 constants.
        // (96 - 9) /3 i.e. Maximum constant count - used constants 
        UINT MaxMatrices = 26; 
        pMeshContainer->NumPaletteEntries = MIN(MaxMatrices, pMeshContainer->pSkinInfo->GetNumBones());

        DWORD Flags = D3DXMESHOPT_VERTEXCACHE;
        if (m_d3dCaps.VertexShaderVersion >= D3DVS_VERSION(1, 1))
        {
            pMeshContainer->UseSoftwareVP = false;
            Flags |= D3DXMESH_MANAGED;
        }
        else
        {
            pMeshContainer->UseSoftwareVP = true;
            //g_bUseSoftwareVP = true;
            Flags |= D3DXMESH_SYSTEMMEM;
        }

        SAFE_RELEASE(pMeshContainer->MeshData.pMesh);

        hr = pMeshContainer->pSkinInfo->ConvertToIndexedBlendedMesh
                                                (
                                                pMeshContainer->pOrigMesh,
                                                Flags, 
                                                pMeshContainer->NumPaletteEntries, 
                                                pMeshContainer->pAdjacency, 
                                                NULL, NULL, NULL,             
                                                &pMeshContainer->NumInfl,
                                                &pMeshContainer->NumAttributeGroups, 
                                                &pMeshContainer->pBoneCombinationBuf, 
                                                &pMeshContainer->MeshData.pMesh);
        if (FAILED(hr))
            return hr;
    

		//-- ADDITION: HLSL
		
		// FVF has to match our declarator. Vertex shaders are not as forgiving as FF pipeline
        DWORD NewFVF = (pMeshContainer->MeshData.pMesh->GetFVF() & D3DFVF_POSITION_MASK) | D3DFVF_NORMAL | D3DFVF_TEX1 | D3DFVF_LASTBETA_UBYTE4;
        if (NewFVF != pMeshContainer->MeshData.pMesh->GetFVF())
        {
            LPD3DXMESH pMesh;
            hr = pMeshContainer->MeshData.pMesh->CloneMeshFVF(pMeshContainer->MeshData.pMesh->GetOptions(), NewFVF, m_pd3dDevice, &pMesh);
            if (!FAILED(hr))
            {
                pMeshContainer->MeshData.pMesh->Release();
                pMeshContainer->MeshData.pMesh = pMesh;
                pMesh = NULL;
            }
        }
		
        D3DVERTEXELEMENT9 pDecl[MAX_FVF_DECL_SIZE];
        LPD3DVERTEXELEMENT9 pDeclCur;
        hr = pMeshContainer->MeshData.pMesh->GetDeclaration(pDecl);
        if (FAILED(hr)) return hr;
		

        // the vertex shader is expecting to interpret the UBYTE4 as a D3DCOLOR, so update the type 
        //   NOTE: this cannot be done with CloneMesh, that would convert the UBYTE4 data to float and then to D3DCOLOR
        //          this is more of a "cast" operation
        pDeclCur = pDecl;
        while (pDeclCur->Stream != 0xff)
        {
            if ((pDeclCur->Usage == D3DDECLUSAGE_BLENDINDICES) && (pDeclCur->UsageIndex == 0))
                pDeclCur->Type = D3DDECLTYPE_D3DCOLOR;
            pDeclCur++;
        }
		
        hr = pMeshContainer->MeshData.pMesh->UpdateSemantics(pDecl);
        if (FAILED(hr)) return hr;
		
        // allocate a buffer for bone matrices, but only if another mesh has not allocated one of the same size or larger
        if( m_NumBoneMatricesMax < pMeshContainer->pSkinInfo->GetNumBones() )
        {
            m_NumBoneMatricesMax = pMeshContainer->pSkinInfo->GetNumBones();
			
            // Allocate space for blend matrices
            delete[] m_pBoneMatrices; 
            m_pBoneMatrices  = new D3DXMATRIXA16[m_NumBoneMatricesMax];
            if( m_pBoneMatrices == NULL )
            {
                hr = E_OUTOFMEMORY;
                return hr;
            }
        }


		return hr;
}




//-----------------------------------------------------------------------------
// Name: DrawMeshContainer()
// Desc: Called to render a mesh in the hierarchy
//-----------------------------------------------------------------------------
void CModel::DrawMeshContainer(LPD3DXMESHCONTAINER pMeshContainerBase, LPD3DXFRAME pFrameBase)
{
    D3DXMESHCONTAINER_DERIVED *pMeshContainer = (D3DXMESHCONTAINER_DERIVED*)pMeshContainerBase;
    D3DXFRAME_DERIVED *pFrame = (D3DXFRAME_DERIVED*)pFrameBase;
    UINT iMaterial;
    UINT iAttrib;
    LPD3DXBONECOMBINATION pBoneComb;


    UINT iMatrixIndex;
    UINT iPaletteEntry;
    D3DXMATRIXA16 matTemp;


    // first check for skinning
    if (pMeshContainer->pSkinInfo != NULL)
    {

		// if hw doesn't support indexed vertex processing, switch to software vertex processing
		if (pMeshContainer->UseSoftwareVP)
		{
			m_pd3dDevice->SetSoftwareVertexProcessing(TRUE);
		}
			
        pBoneComb = reinterpret_cast<LPD3DXBONECOMBINATION>( pMeshContainer->pBoneCombinationBuf->GetBufferPointer() );
        for (iAttrib = 0; iAttrib < pMeshContainer->NumAttributeGroups; iAttrib++)
        { 

			// first calculate all the world matrices
			for (iPaletteEntry = 0; iPaletteEntry < pMeshContainer->NumPaletteEntries; ++iPaletteEntry)
			{
				iMatrixIndex = pBoneComb[iAttrib].BoneId[iPaletteEntry];
				if (iMatrixIndex != UINT_MAX)
				{
					//D3DXMatrixMultiply(&m_pBoneMatrices[iPaletteEntry], &pMeshContainer->pBoneOffsetMatrices[iMatrixIndex], pMeshContainer->ppBoneMatrixPtrs[iMatrixIndex]);	
					D3DXMatrixMultiply(&m_pBoneMatrices[iPaletteEntry], &pMeshContainer->pBoneOffsetMatrices[iMatrixIndex], &m_pBoneMatricesLocal[iMatrixIndex]);	
				
				}
			}
			//HTMLLog("That was matrixmultiply cycle %s: <b>iAttrib</b>: %d; <b>iPaletteEntry</b>: %d\n", pFrame->Name, (int)iAttrib, (int)iPaletteEntry);


			//then transmit them to shader
			m_pEffect->SetMatrixArray( "mWorldMatrixArray", m_pBoneMatrices, pMeshContainer->NumPaletteEntries);


			// setup the material of the mesh subset - REMEMBER to use the original pre-skinning attribute id to get the correct material id
			//m_pd3dDevice->SetTexture( 0, pMeshContainer->ppTextures[pBoneComb[iAttrib].AttribId] );

			if( msg != DONTDRAWTEXTURES )
			{
				m_pEffect->SetTexture( "tColorMap", pMeshContainer->ppTextures[pBoneComb[iAttrib].AttribId]);
				
				if( msg != DONTDRAWBUMP )
					m_pEffect->SetTexture( "tBumpMap", pMeshContainer->ppBumpTextures[pBoneComb[iAttrib].AttribId]);
			}

			// Set CurNumBones to select the correct vertex shader for the number of bones
			m_pEffect->SetInt( "CurNumBones", pMeshContainer->NumInfl -1);
			m_pEffect->SetMatrix( "mViewProj", &(m_matView * m_matProj) );
			
			m_pEffect->CommitChanges();
			pMeshContainer->MeshData.pMesh->DrawSubset( iAttrib );
			
		}
		
		// remember to reset back to hw vertex processing if software was required
		if (pMeshContainer->UseSoftwareVP)
		{
			m_pd3dDevice->SetSoftwareVertexProcessing(FALSE);
		}

    } //pSkinInfo != NULL



    else  // standard mesh, just draw it after setting material properties
    {
			//m_pd3dDevice->SetTransform(D3DTS_WORLD, &pFrame->CombinedTransformationMatrix);
			m_pEffect->SetMatrix( "mViewProj", &(m_matView * m_matProj) );


			//get proper local matrix
			//m_pEffect->SetMatrix( "mWorld", &pFrame->CombinedTransformationMatrix );
			m_pEffect->SetMatrix( "mWorld", &m_pBoneMatricesLocal[ GetFrameIndexByName(pFrame->Name) ] );

			for (iMaterial = 0; iMaterial < pMeshContainer->NumMaterials; iMaterial++)
			{
				if( msg != DONTDRAWTEXTURES )
				{
					m_pEffect->SetTexture("tColorMap", pMeshContainer->ppTextures[iMaterial]);
					if ( msg != DONTDRAWBUMP )
						m_pEffect->SetTexture("tBumpMap", pMeshContainer->ppBumpTextures[iMaterial]);
				}

				m_pEffect->CommitChanges();
				pMeshContainer->MeshData.pMesh->DrawSubset(iMaterial);
			}

    }
}


//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
LPD3DXFRAME CModel::GetFrameByNum(LPD3DXFRAME pFrame, int Num, BOOL firsttime)
{
	if( firsttime == TRUE ) frameCounter = -1;

	LPD3DXFRAME res;

	if( pFrame->Name != NULL && pFrame->Name[0] != '\0' ) frameCounter++;
	if( frameCounter == Num ) return pFrame;

	if( pFrame->pFrameSibling != NULL ) { res = GetFrameByNum( pFrame->pFrameSibling, Num, FALSE); if( frameCounter == Num ) return res; }
	if( pFrame->pFrameFirstChild != NULL ) { res = GetFrameByNum( pFrame->pFrameFirstChild, Num, FALSE); if( frameCounter == Num ) return res; }

	return NULL;
}


//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
NxActor* CModel::GetBipedByName(char *name)
{
	for(int i=0; i < NumOfBiped; i++)
	{
		if(!strcmp(name, bipedNames[i])) return biped[i];
	}

	//HTMLLog("haven't found... sorry\n");
	return NULL;
}



//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
int CModel::GetBipedIndexByName(char* name)
{
	for(int i=0; i < NumOfBiped; i++)
	{
		if(!strcmp(name, bipedNames[i])) return i;
	}

	return -1;
}

//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
int CModel::GetFrameIndexByName(char* name)
{
	for(int i=0; i < numFrames; i++)
	{
		if(!strcmp(name, m_pBoneMatricesLocalNames[i])) return i;
	}

	return -1;
}

//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
int CModel::GetNumOfFrames(LPD3DXFRAME pFrame)
{
	int c = 0;

	//namenull check - altered since 2003 SDK
	if( pFrame->Name != NULL && pFrame->Name[0] != '\0' ) { c++; }

	if( pFrame->pFrameSibling != NULL ) c += GetNumOfFrames( pFrame->pFrameSibling );
	if( pFrame->pFrameFirstChild != NULL ) c += GetNumOfFrames( pFrame->pFrameFirstChild );

	return c;
}


//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
int CModel::UpdateBipedsPhysXByFrame( LPD3DXFRAME pFrame, int parentindex, BOOL firsttime )
{
	D3DXMATRIXA16 pmatrix; //result matrix
	int iterstorage;
	if( firsttime == TRUE ) frameCounter = -1;

	//namenull check - altered since 2003 SDK
	if( pFrame->Name != NULL && pFrame->Name[0] != '\0' )
	{ 
		frameCounter++;

		if( !strncmp( m_pBoneMatricesLocalNames[frameCounter], "Bip", 3) )
		{
			char name[256];
			sprintf( name, "CBP_%s", m_pBoneMatricesLocalNames[frameCounter]+6 );
			int bipindex = GetBipedIndexByName( name );
			//HTMLLog("\ngot biped index %d:%s for %d:%s", bipindex, name, i, m_pBoneMatricesLocalNames[i]);

			if( bipindex == -1 ) m_pBoneMatricesLocal[frameCounter] = m_pBoneMatricesLocal[ parentindex ];
			else
			{
				biped[ bipindex ]->getGlobalPose().getColumnMajor44( pmatrix );
				m_pBoneMatricesLocal[ frameCounter ] = pmatrix;
			}
		}
	}

	iterstorage = frameCounter;
	if( pFrame->pFrameSibling != NULL ) UpdateBipedsPhysXByFrame( pFrame->pFrameSibling, iterstorage, FALSE );
	if( pFrame->pFrameFirstChild != NULL ) UpdateBipedsPhysXByFrame( pFrame->pFrameFirstChild, iterstorage, FALSE );

	return 0;
}

//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
void CModel::CreatePhysXBipedObject(void)
{
	int i;
	
	/*
	HTMLLog("\n------------------------------\n");
	if( id == 0 ) LogFrameHierarchy(m_pFrameRoot, 0);
	HTMLLog("\n------------------------------\n");
	*/

	//allocating memory
	NumOfBiped = GetNumBipeds( m_pFrameRoot );
	HTMLLog("\t\tCreating Biped System with %d bipeds\n", NumOfBiped);

	biped = new NxActor *[NumOfBiped];
	bipedMatrix = new D3DXMATRIX *[NumOfBiped];
	bipedNames = new char *[NumOfBiped];
	for(i=0;i<NumOfBiped;i++) bipedNames[i] = new char[256];

	CurBiped = 0;
	CreatePhysXBipedObjectFrame( m_pFrameRoot );


	//joints
	NumOfJoint = GetNumJoints( m_pFrameRoot );
	HTMLLog("\t\tCreating joints %d\n", NumOfJoint);

	joint = new NxRevoluteJoint* [NumOfJoint];
	jointNames = new char *[NumOfJoint];
	for(i=0;i<NumOfJoint;i++) jointNames[i] = new char[256];

	CurJoint = 0;
	//CreatePhysXJointFrame( m_pFrameRoot );



	// important stuff
	for( i = 0; i < NumOfBiped; i++ )
	{
		SetActorCollisionGroup( biped[i], GROUP_BIPED );
	}
	
	return;
}


//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
void CModel::CreatePhysXObject(void)
{
	physxtype = ScanFramesForKeywords(m_pFrameRoot);

	if( physxtype != FT_NONE )
	{
		NxActorDesc* ActorDesc = new NxActorDesc();

		CreatePhysXObjectFrame(m_pFrameRoot, ActorDesc);
		HTMLLog("\tObject hierarchy created successfully\n");


		if( physxtype == FT_SOL)
		{
			HTMLLog("\t\ttype: FT_SOL\n");
			ActorDesc->body = NULL;
		}
		else if( physxtype == FT_DOL )
		{
			HTMLLog("\t\ttype: FT_DOL\n");
			NxBodyDesc	bodyDesc;
			ActorDesc->body = &bodyDesc;
			ActorDesc->density = 10600.f;
		}
		else if( physxtype == FT_COL )
		{
			HTMLLog("\t\ttype: FT_COL\n");
			NxBodyDesc	bodyDesc;
			bodyDesc.flags = NX_BF_KINEMATIC;
			ActorDesc->body = &bodyDesc;
			ActorDesc->density = 600.f;
		}
		else
		{
			HTMLLog("Unknown collision type! exitiong...\n");
			return;
		}

		ActorDesc->globalPose.t = DxVec3ToNxVec3( coordinate );
		object = mScene->createActor(*ActorDesc);

		if( object )
		{
			int NumOfShapes = (int)object->getNbShapes();
			HTMLLog("\t\tcreated %d shapes\n", NumOfShapes);
		}
		else HTMLLog("\t\t<b>creation of object FAILED!</b>\n");

		//groups
		if( physxtype == FT_SOL)
		{
			SetActorCollisionGroup(object, GROUP_COLLIDABLE_NON_PUSHABLE);
		}
		else if( physxtype == FT_DOL )
		{
			SetActorCollisionGroup(object, GROUP_COLLIDABLE_NON_PUSHABLE);
			object->setAngularDamping(0.4f);
			object->setLinearDamping(0.1f);
		}
		else if( physxtype == FT_COL )
		{
			SetActorCollisionGroup(object, GROUP_WEAPONS);
		}
		else if( physxtype == FT_CHARACTER ) //which is impossible
		{
		}
	}

	return;
}

//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
void CModel::CreatePhysXObjectBoxMeshContainer(LPD3DXMESHCONTAINER pMeshContainerBase, LPD3DXFRAME pFrameBase, NxActorDesc* ActorDesc)
{
	HTMLLog("\t\t<kbd>Creating physX object with name %s</kbd>\n", pFrameBase->Name);

    D3DXMESHCONTAINER_DERIVED *pMeshContainer = (D3DXMESHCONTAINER_DERIVED*)pMeshContainerBase;
    D3DXFRAME_DERIVED *pFrame = (D3DXFRAME_DERIVED*)pFrameBase;
	int i;

	//store
	objectMatrix = &pFrame->CombinedTransformationMatrix;

	char* pBuff;

	int NumVert;
	int NumInd;
	int NumFaces;

	if(FAILED(  pMeshContainer->MeshData.pMesh->LockVertexBuffer(D3DLOCK_READONLY, (LPVOID*)&pBuff)  ))
		LogPlease("CreatePhysXObject(): Cannot lock vertex buffer\n");
	NumVert = pMeshContainer->MeshData.pMesh->GetNumVertices();
	NumInd = pMeshContainer->MeshData.pMesh->GetNumFaces()*3;
	NumFaces = pMeshContainer->MeshData.pMesh->GetNumFaces();

	char stride = pMeshContainer->MeshData.pMesh->GetNumBytesPerVertex();
	//HTMLLog("compairing: %d - %d\n", (int)stride, sizeof(D3DXVECTOR3));
	
	//here's the translation we wanted
	//Create pointer for vertices
    D3DXVECTOR3* verts = new D3DXVECTOR3[8];
	int jj = 0;
	int j;

    for(i = 0; i < NumVert; i++)
    {
		D3DXVECTOR3 vecptr = *(D3DXVECTOR3*)(pBuff + stride*i);
		for(j=0; j < jj; j++)
		{
			if(verts[j] == vecptr) break;
		}
		if(j == jj)
		{
			verts[j] = vecptr;
			jj++;
			//HTMLLog("%d  - (%d, %d, %d)\n", j, (int)verts[j].x, (int)verts[j].y, (int)verts[j].z);
		}

    }
	
	pMeshContainer->MeshData.pMesh->UnlockVertexBuffer();
	pBuff = NULL;
	LogPlease("\t\tTranslation complete...\n");

	// now we'll get parameters for box object
	// !!!!!
	// we know, that X and Z coordinates of object are symmetrical
	// ( it means, that X1 == -X2 )
	// but Y coordinate is not centered!
	// !!!!!

	D3DXVECTOR3 bmin = D3DXVECTOR3( 99999.f, 99999.f, 99999.f);
	D3DXVECTOR3 bmax = D3DXVECTOR3(-99999.f,-99999.f,-99999.f);

	//computing Bounding Box
	for(i=0; i<8; i++)
	{
		if(verts[i].x > bmax.x) bmax.x = verts[i].x;
		if(verts[i].x < bmin.x) bmin.x = verts[i].x;
		if(verts[i].y > bmax.y) bmax.y = verts[i].y;
		if(verts[i].y < bmin.y) bmin.y = verts[i].y;
		if(verts[i].z > bmax.z) bmax.z = verts[i].z;
		if(verts[i].z < bmin.z) bmin.z = verts[i].z;
	}
	

	D3DXVECTOR3 bminscaled;
	D3DXVECTOR3 bmaxscaled;

	D3DXVECTOR3 pOutScale;
	D3DXVECTOR3 pOutTranslation;
	D3DXQUATERNION pOutRotation;
	D3DXMATRIX matRot;
	D3DXMATRIX matScale;
	D3DXMATRIX matTrans;

	D3DXMatrixDecompose( &pOutScale, &pOutRotation, &pOutTranslation, &pFrame->TransformationMatrix );
	D3DXMatrixScaling( &matScale, pOutScale.x, pOutScale.y, pOutScale.z);
	D3DXVec3TransformCoord( &bminscaled, &bmin, &matScale );
	D3DXVec3TransformCoord( &bmaxscaled, &bmax, &matScale );

	//creating box
	D3DXVECTOR3	boxsize;
	D3DXVECTOR3 boxdisplace;

	boxsize = (bmaxscaled - bminscaled)/2;
	boxdisplace = D3DXVECTOR3( 0, bminscaled.y + boxsize.y, 0);
	HTMLLog("\t\t<kbd>Generated Box: (%d %d %d); y displacement - %d\n</kbd>", (int)boxsize.x, (int)boxsize.y, (int)boxsize.z, (int)boxdisplace.y);

	//Box Shape
	NxBoxShapeDesc* boxShape = new NxBoxShapeDesc();
	boxShape->dimensions	 = DxVec3ToNxVec3( boxsize );

	D3DXMatrixRotationQuaternion( &matRot, &pOutRotation );
	boxShape->localPose.setColumnMajor44( matRot );

	D3DXVec3TransformCoord( &boxdisplace, &boxdisplace, &matRot);
	
	if( physxtype != FT_COL ) boxdisplace += pOutTranslation;

	//no need in this for kinematic, because it gets the same matrix from animation
	boxShape->localPose.t	 = DxVec3ToNxVec3( boxdisplace );

	ActorDesc->shapes.pushBack( boxShape );

	//add trigger
	if( physxtype  == FT_COL )
	{
		NxBoxShapeDesc* boxShape2 = new NxBoxShapeDesc();
		boxShape2->dimensions	 = DxVec3ToNxVec3( boxsize );
		boxShape2->localPose.setColumnMajor44( matRot );
		boxShape2->localPose.t	 = DxVec3ToNxVec3( boxdisplace );
		boxShape2->shapeFlags = NX_TRIGGER_ENABLE;
		ActorDesc->shapes.pushBack( boxShape2 );
	}

	return;
}



//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
void CModel::CreatePhysXBipedObjectMeshContainer(LPD3DXMESHCONTAINER pMeshContainerBase, LPD3DXFRAME pFrameBase)
{
	HTMLLog("\t\t<kbd>Creating physX %d biped object with name %s</kbd>...", CurBiped, pFrameBase->Name);

    D3DXMESHCONTAINER_DERIVED *pMeshContainer = (D3DXMESHCONTAINER_DERIVED*)pMeshContainerBase;
    D3DXFRAME_DERIVED *pFrame = (D3DXFRAME_DERIVED*)pFrameBase;
	int i;

	//reparse the name
	//char* parser = pFrame->Name + 4;
	//char name[256];
	//sprintf( name, "Bip01_%s", parser);
	bipedMatrix[CurBiped] = &m_pBoneMatricesLocal[ GetFrameIndexByName( pFrame->Name ) ];
	sprintf( bipedNames[CurBiped], "%s", pFrame->Name );
	//HTMLLog("  %s  ", bipedNames[CurBiped]);
	
	char* pBuff;
	int NumVert;
	int NumInd;
	int NumFaces;

	if(FAILED(  pMeshContainer->MeshData.pMesh->LockVertexBuffer(D3DLOCK_READONLY, (LPVOID*)&pBuff)  ))
		LogPlease("CreatePhysXObject(): Cannot lock vertex buffer\n");
	NumVert = pMeshContainer->MeshData.pMesh->GetNumVertices();
	NumInd = pMeshContainer->MeshData.pMesh->GetNumFaces()*3;
	NumFaces = pMeshContainer->MeshData.pMesh->GetNumFaces();

	DWORD stride = pMeshContainer->MeshData.pMesh->GetNumBytesPerVertex();
	
	D3DXVECTOR3 bmin = D3DXVECTOR3( 99999.f, 99999.f, 99999.f);
	D3DXVECTOR3 bmax = D3DXVECTOR3(-99999.f,-99999.f,-99999.f);

    for(i = 0; i < NumVert; i++)
    {
		D3DXVECTOR3 vecptr = *(D3DXVECTOR3*)(pBuff + stride*i);
		if( vecptr.x > bmax.x ) bmax.x = vecptr.x;
		if( vecptr.y > bmax.y ) bmax.y = vecptr.y;
		if( vecptr.z > bmax.z ) bmax.z = vecptr.z;
		if( vecptr.x < bmin.x ) bmin.x = vecptr.x;
		if( vecptr.y < bmin.y ) bmin.y = vecptr.y;
		if( vecptr.z < bmin.z ) bmin.z = vecptr.z;
		//HTMLLog("%f %f %f\n", vecptr.x, vecptr.y, vecptr.z);
    }

	pMeshContainer->MeshData.pMesh->UnlockVertexBuffer();
	pBuff = NULL;
	//LogPlease("\t\tTranslation complete...\n");


	D3DXVECTOR3 bminscaled;
	D3DXVECTOR3 bmaxscaled;

	D3DXVECTOR3 pOutScale;
	D3DXVECTOR3 pOutTranslation;
	D3DXQUATERNION pOutRotation;
	D3DXMATRIX matRot;
	D3DXMATRIX matScale;
	D3DXMATRIX matTrans;

	D3DXMatrixDecompose( &pOutScale, &pOutRotation, &pOutTranslation, &pFrame->TransformationMatrix );
	D3DXMatrixScaling( &matScale, pOutScale.x, pOutScale.y, pOutScale.z);
	D3DXVec3TransformCoord( &bminscaled, &bmin, &matScale );
	D3DXVec3TransformCoord( &bmaxscaled, &bmax, &matScale );
	//if swap
	if( bminscaled.x > bmaxscaled.x ) { float helper = bminscaled.x; bminscaled.x = bmaxscaled.x; bmaxscaled.x = helper; }
	if( bminscaled.y > bmaxscaled.y ) { float helper = bminscaled.y; bminscaled.y = bmaxscaled.y; bmaxscaled.y = helper; }
	if( bminscaled.z > bmaxscaled.z ) { float helper = bminscaled.z; bminscaled.z = bmaxscaled.z; bmaxscaled.z = helper; }

	//creating box
	D3DXVECTOR3	boxsize;
	//D3DXVECTOR3 newboxsize;
	D3DXVECTOR3 boxdisplace;

	//strange operation =)
	boxsize = (bmaxscaled - bminscaled)/2;
	boxdisplace = D3DXVECTOR3( 0, bminscaled.y + boxsize.y, 0);

	//HTMLLog("\t translation: (%f %f %f) \t", pOutTranslation.x, pOutTranslation.y, pOutTranslation.z );
	//HTMLLog("\t\t<kbd>Generated Box: (%f %f %f); y displacement - %f\n</kbd>", boxsize.x, boxsize.y, boxsize.z, boxdisplace.y);

	//Box Shape
	
	NxBoxShapeDesc* boxShape = new NxBoxShapeDesc();
	boxShape->dimensions	 = DxVec3ToNxVec3( boxsize );
	boxShape->localPose.t	 = DxVec3ToNxVec3( boxdisplace );
	
/*
	NxCapsuleShapeDesc* sphereShape = new NxCapsuleShapeDesc();
	sphereShape->height = height;
	sphereShape->radius = radius;
	sphereShape->localPose.t	 = DxVec3ToNxVec3( boxdisplace );
*/
	NxBodyDesc*  bodyDesc = new NxBodyDesc();
    bodyDesc->flags = NX_BF_KINEMATIC;

	NxActorDesc* ActorDesc = new NxActorDesc();
	ActorDesc->shapes.pushBack( boxShape );
    ActorDesc->body = bodyDesc;
	ActorDesc->density = 600.f;
    ActorDesc->globalPose.t = DxVec3ToNxVec3( coordinate );

    biped[CurBiped] = mScene->createActor(*ActorDesc);
	if( !biped[CurBiped] ) HTMLLog("<b>...FAILED!\n</b>");

	biped[CurBiped]->setAngularDamping( 0.6f );
	biped[CurBiped]->setLinearDamping( 0.8f );
	SetActorCollisionGroup(biped[CurBiped], GROUP_BIPED);
	
	
	CurBiped++;
	HTMLLog(" ...<kbd>success!\n</kbd>");


	return;
}


//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
void CModel::CreatePhysXJointMeshContainer(LPD3DXMESHCONTAINER pMeshContainerBase, LPD3DXFRAME pFrameBase)
{
	HTMLLog("\t\t<kbd>Creating physX %d joint with name %s</kbd>...\n", CurJoint, pFrameBase->Name);

    D3DXMESHCONTAINER_DERIVED *pMeshContainer = (D3DXMESHCONTAINER_DERIVED*)pMeshContainerBase;
    D3DXFRAME_DERIVED *pFrame = (D3DXFRAME_DERIVED*)pFrameBase;
	int i,j;

	//store
	sprintf( jointNames[CurJoint], "%s", pFrame->Name );
	//HTMLLog("  %s  ", jointNames[CurJoint]);

	D3DXVECTOR3 pOutScale;
	D3DXVECTOR3 pOutTranslation;
	D3DXQUATERNION pOutRotation;
	
	D3DXVECTOR3 axis;
	char type;


	type = REVOLUTE;
	axis = D3DXVECTOR3( 0, 0, 1 );

	//extract names
	char* parser = pFrame->Name;
	char name1[256], name2[256];

	while( *parser != '_' ) parser++;
	parser++;
	for( j = 0; *parser != '*'; j++, parser++ ) name1[j] = *parser;
	name1[j] = '\0';
	parser += 2;
	for( j = 0; *parser != '*'; j++, parser++ ) name2[j] = *parser;
	name2[j] = '\0';
	parser++;


	//coordinate
	D3DXMatrixDecompose( &pOutScale, &pOutRotation, &pOutTranslation, &pFrame->TransformationMatrix );

	NxActor *act1, *act2;
	act1 = GetBipedByName( name1 );
	act2 = GetBipedByName( name2 );
	if( act1 == NULL || act2 == NULL ) HTMLLog("\nError: cannot find biped with names: %s %s\n", name1, name2 );

	//nessesary part - we have to remember, that anchor is global
	pOutTranslation += coordinate;

	if( type == REVOLUTE ) 
	{
		joint[CurJoint] = 
			CreateRevoluteJoint(act1, act2, DxVec3ToNxVec3( pOutTranslation ), NxVec3(0,0,1), mScene);
		if( joint[CurJoint] == NULL ) HTMLLog("<b>\nFailed to create JOINT\n</b>");
	}


	CurJoint++;
	HTMLLog("\t\t\tParse results: name1 %s, name2 %s, coord: (%d, %d, %d), %d\n",
			name1, name2, (int)pOutTranslation.x, (int)pOutTranslation.y, (int)pOutTranslation.z, (int)type );
	HTMLLog("\t\t...<kbd>success!\n</kbd>");


	return;
}


//-----------------------------------------------------------------------------
// Name:
// Desc: 
//-----------------------------------------------------------------------------
void CModel::CreatePhysXObjectFrame(LPD3DXFRAME pFrame, NxActorDesc* ActorDesc)
{

    LPD3DXMESHCONTAINER pMeshContainer;

    pMeshContainer = pFrame->pMeshContainer;
    while (pMeshContainer != NULL)
    {
		//important check!
		//don't draw frames with no name or biped and collision frames
		if( pFrame->Name != NULL && pFrame->Name[0] != '\0' &&
			( !strncmp( pFrame->Name, "SOL", 3) || !strncmp( pFrame->Name, "COL", 3)  || !strncmp( pFrame->Name, "DOL", 3) ) 
		  )
		
			CreatePhysXObjectBoxMeshContainer(pMeshContainer, pFrame, ActorDesc);

        pMeshContainer = pMeshContainer->pNextMeshContainer;
    }

    if (pFrame->pFrameSibling != NULL)
    {
		CreatePhysXObjectFrame(pFrame->pFrameSibling, ActorDesc);
    }

    if (pFrame->pFrameFirstChild != NULL)
    {
		CreatePhysXObjectFrame(pFrame->pFrameFirstChild, ActorDesc);
    }
}


//-----------------------------------------------------------------------------
// Name:
// Desc: very bad function!!! but it works (for now)
//       based on assume, that every Bip has child with container, which store his mesh
//-----------------------------------------------------------------------------
void CModel::CreatePhysXBipedObjectFrame( LPD3DXFRAME pFrame )
{
	//important check!
	if( pFrame->Name != NULL && !strncmp( pFrame->Name, "CBP", 3) && pFrame->pMeshContainer != NULL
		//pFrame->pFrameFirstChild != NULL && pFrame->pFrameFirstChild->pMeshContainer != NULL
	  )
	{
		CreatePhysXBipedObjectMeshContainer(pFrame->pMeshContainer, pFrame);
	}

    if (pFrame->pFrameSibling != NULL)
    {
		CreatePhysXBipedObjectFrame(pFrame->pFrameSibling);
    }

    if (pFrame->pFrameFirstChild != NULL)
    {
		CreatePhysXBipedObjectFrame(pFrame->pFrameFirstChild);
    }
}

//-----------------------------------------------------------------------------
// Name:
// Desc: very bad function!!! but it works (for now)
//       based on assume, that every Bip has child with container, which store his mesh
//-----------------------------------------------------------------------------
void CModel::CreatePhysXJointFrame( LPD3DXFRAME pFrame )
{
	//important check!
	if( pFrame->Name != NULL && !strncmp( pFrame->Name, "JNT", 3) && pFrame->pMeshContainer != NULL
		//pFrame->pFrameFirstChild != NULL && pFrame->pFrameFirstChild->pMeshContainer != NULL
	  )
	{
		CreatePhysXJointMeshContainer(pFrame->pMeshContainer, pFrame);
	}

    if (pFrame->pFrameSibling != NULL)
    {
		CreatePhysXJointFrame(pFrame->pFrameSibling);
    }

    if (pFrame->pFrameFirstChild != NULL)
    {
		CreatePhysXJointFrame(pFrame->pFrameFirstChild);
    }
}


//-----------------------------------------------------------------------------
// Name:
// Desc: 
//-----------------------------------------------------------------------------
#define TABGENERATION for(int i=0; i<generation; i++) HTMLLog("\t");
void CModel::LogFrameHierarchy(LPD3DXFRAME pFrame, int generation)
{
	TABGENERATION;
	if( pFrame->Name == NULL || pFrame->Name[0] == '\0' )
		HTMLLog("Frame: Noname, ");
	else HTMLLog("Frame: %s, ", pFrame->Name, pFrame->Name);
	
	if(pFrame->pMeshContainer == NULL) HTMLLog("meshcontainer == NULL\n");
	else HTMLLog("meshcontainer != NULL\n");

	if (pFrame->pFrameFirstChild != NULL)
		LogFrameHierarchy( pFrame->pFrameFirstChild, generation+1 );

	if (pFrame->pFrameSibling != NULL)
		LogFrameHierarchy( pFrame->pFrameSibling, generation );


	
	return;
}

//-----------------------------------------------------------------------------
// Name:
// Desc: not just bipeds, but material bipeds!
//-----------------------------------------------------------------------------
int CModel::GetNumJoints(LPD3DXFRAME pFrame)
{
	int result = 0;

	if( pFrame->Name != NULL && !strncmp( pFrame->Name, "JNT", 3) && pFrame->pMeshContainer != NULL
		//&& pFrame->pFrameFirstChild != NULL && pFrame->pFrameFirstChild->pMeshContainer != NULL
	  )	result++;

	if (pFrame->pFrameSibling != NULL)
		result += GetNumJoints( pFrame->pFrameSibling );

	if (pFrame->pFrameFirstChild != NULL)
		result += GetNumJoints( pFrame->pFrameFirstChild );
	
	return result;
}



//-----------------------------------------------------------------------------
// Name:
// Desc: not just bipeds, but material bipeds!
//-----------------------------------------------------------------------------
int CModel::GetNumBipeds(LPD3DXFRAME pFrame)
{
	int result = 0;

	if( pFrame->Name != NULL && !strncmp( pFrame->Name, "CBP", 3) && pFrame->pMeshContainer != NULL
		//&& pFrame->pFrameFirstChild != NULL && pFrame->pFrameFirstChild->pMeshContainer != NULL
	  )	result++;

	if (pFrame->pFrameSibling != NULL)
		result += GetNumBipeds( pFrame->pFrameSibling );

	if (pFrame->pFrameFirstChild != NULL)
		result += GetNumBipeds( pFrame->pFrameFirstChild );
	
	return result;
}

//-----------------------------------------------------------------------------
// Name:
// Desc: defines type of physx object
//-----------------------------------------------------------------------------
char CModel::ScanFramesForKeywords(LPD3DXFRAME pFrame)
{
	char result;

	if( pFrame->Name != NULL && !strncmp( pFrame->Name, "COL", 3) )
	{
		return FT_COL;
	}
	else if ( pFrame->Name != NULL &&  !strncmp( pFrame->Name, "SOL", 3) )
	{
		return FT_SOL;
	}
	else if ( pFrame->Name != NULL &&  !strncmp( pFrame->Name, "DOL", 3) )
	{
		return FT_DOL;
	}
	else
	{
		if (pFrame->pFrameSibling != NULL)
		{
			result = ScanFramesForKeywords( pFrame->pFrameSibling );
			if( result != FT_NONE ) return result;
		}

		if (pFrame->pFrameFirstChild != NULL)
		{
			result = ScanFramesForKeywords( pFrame->pFrameFirstChild );
			if( result != FT_NONE ) return result;
		}
	}
		
	return FT_NONE;
}



#define DONOTDRAW(str) ( !strncmp( str, "Bip", 3) || !strncmp( str, "COL", 3) || !strncmp( str, "SOL", 3) || !strncmp( str, "DOL", 3) || !strncmp( str, "CBP", 3) || !strncmp( str, "JNT", 3) )
//-----------------------------------------------------------------------------
// Name: DrawFrame()
// Desc: Called to render a frame in the hierarchy
//       do not draw frames with "Bip" and "Col" begins
//-----------------------------------------------------------------------------
void CModel::DrawFrame(LPD3DXFRAME pFrame)
{
    LPD3DXMESHCONTAINER pMeshContainer;

    pMeshContainer = pFrame->pMeshContainer;
    while (pMeshContainer != NULL)
    {
		//important check!
		//don't draw frames with no name or biped and collision frames
		if( pFrame->Name != NULL && pFrame->Name[0] != '\0' &&
			DONOTDRAW( pFrame->Name  )
		  );
		else
			DrawMeshContainer(pMeshContainer, pFrame);

        pMeshContainer = pMeshContainer->pNextMeshContainer;
    }

    if (pFrame->pFrameSibling != NULL)
    {
        DrawFrame(pFrame->pFrameSibling);
    }

    if (pFrame->pFrameFirstChild != NULL)
    {
		//important check!
		if( pFrame->Name != NULL && pFrame->Name[0] != '\0' &&
			DONOTDRAW( pFrame->Name  )
		  );
		else
			DrawFrame(pFrame->pFrameFirstChild);
    }
}



//-----------------------------------------------------------------------------
// Name: SetupBoneMatrixPointersOnMesh()
// Desc: Called to setup the pointers for a given bone to its transformation matrix
//-----------------------------------------------------------------------------
HRESULT CModel::SetupBoneMatrixPointersOnMesh(LPD3DXMESHCONTAINER pMeshContainerBase)
{
    UINT iBone, cBones;
    D3DXFRAME_DERIVED *pFrame;

    D3DXMESHCONTAINER_DERIVED *pMeshContainer = (D3DXMESHCONTAINER_DERIVED*)pMeshContainerBase;

    // if there is a skinmesh, then setup the bone matrices
    if (pMeshContainer->pSkinInfo != NULL)
    {
        cBones = pMeshContainer->pSkinInfo->GetNumBones();

        pMeshContainer->ppBoneMatrixPtrs = new D3DXMATRIX*[cBones];
        if( pMeshContainer->ppBoneMatrixPtrs == NULL )
            return E_OUTOFMEMORY;

        for (iBone = 0; iBone < cBones; iBone++)
        {
            pFrame = (D3DXFRAME_DERIVED*)D3DXFrameFind(m_pFrameRoot, pMeshContainer->pSkinInfo->GetBoneName(iBone));
            if( pFrame == NULL )
                return E_FAIL;

            pMeshContainer->ppBoneMatrixPtrs[iBone] = &pFrame->CombinedTransformationMatrix;
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SetupBoneMatrixPointers()
// Desc: Called to setup the pointers for a given bone to its transformation matrix
//-----------------------------------------------------------------------------
HRESULT CModel::SetupBoneMatrixPointers(LPD3DXFRAME pFrame)
{
    HRESULT hr;

    if (pFrame->pMeshContainer != NULL)
    {
        hr = SetupBoneMatrixPointersOnMesh(pFrame->pMeshContainer);
        if (FAILED(hr))
            return hr;
    }

    if (pFrame->pFrameSibling != NULL)
    {
        hr = SetupBoneMatrixPointers(pFrame->pFrameSibling);
        if (FAILED(hr))
            return hr;
    }

    if (pFrame->pFrameFirstChild != NULL)
    {
        hr = SetupBoneMatrixPointers(pFrame->pFrameFirstChild);
        if (FAILED(hr))
            return hr;
    }

    return S_OK;
}




HRESULT CModel::SetStockAnimationSetByName(char* name, float time) { ASI.StockAnimationSet = name; ASI.StockTime = time; return S_OK; }

//-----------------------------------------------------------------------------
// Name: PassAnimationStock()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CModel::PassAnimationStock()
{
	PassAnimation( ASI.StockAnimationSet, ASI.StockTime );
	
	return S_OK;
}



//-----------------------------------------------------------------------------
// Name: PassAnimation()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CModel::PassAnimation(LPCSTR Name2, double fTime)
{

	//1st check - already passing
	if(ASI.ap_bIsAnimPass == TRUE) return E_FAIL;
	
	LPCSTR OldAnimSetName = ASI.AnimSetName;
	//ASI.AnimSetName = (char*)Name2;


	if( m_pAnimController == NULL && MType == BASE ) //error
		return E_FAIL; 

	else if(MType == CLONE) //clone
	{	
		//setting variables
		ASI.ap_StartTime = (float)timeGetTime();
		ASI.ap_BreakTime = ASI.AnimTime;
		ASI.ap_fTime = fTime;


		LPCSTR Name1 = OldAnimSetName;

		//search for animation sets
		int NumSets = ParentModel->m_pAnimController->GetNumAnimationSets();
		for(int i=0; i< NumSets; i++)
		{

			LPD3DXANIMATIONSET AnimSet;
			ParentModel->m_pAnimController->GetAnimationSet(i, &AnimSet);
			if(!stricmp(Name1, AnimSet->GetName()) )
			{
				ASI.ap_NumOfSet1 = i;
			}
			if(!stricmp(Name2, AnimSet->GetName()) )
			{
				ASI.ap_NumOfSet2 = i;
			}
		}

		//2nd check
		if( ASI.ap_NumOfSet1 != ASI.ap_NumOfSet2 )
		{
			ASI.ap_bIsAnimPass = TRUE;
			ASI.bAnimPeriodPassed = FALSE;
		}
		else //if ap_NumOfSet1 == ap_NumOfSet2
			 //we need to play animation from the beginnning
		{
			if( OnceAnimation() == TRUE )
				ASI.AnimTime = 0;
		}

	}
	/*
	else //MType = base
	{

		//setting variables
		ASI.ap_StartTime = (float)timeGetTime();
		ASI.ap_BreakTime = m_pAnimController->GetTime();
		ASI.ap_fTime = fTime;


		LPCSTR Name1;
		LPD3DXANIMATIONSET CurSet;
		m_pAnimController->GetTrackAnimationSet( 0, &CurSet );
		Name1 = CurSet->GetName();


		//search for animation sets
		for(DWORD i=0; i< m_pAnimController->GetNumAnimationSets(); i++)
		{
			LPD3DXANIMATIONSET AnimSet;
			m_pAnimController->GetAnimationSet(i, &AnimSet);
			if(!stricmp(Name1, AnimSet->GetName()) )
			{
				ASI.ap_NumOfSet1 = i;
			}
			if(!stricmp(Name2, AnimSet->GetName()) )
			{
				ASI.ap_NumOfSet2 = i;
			}
		}


		//2nd check
		if( ASI.ap_NumOfSet1 != ASI.ap_NumOfSet2 )
		{
			ASI.ap_bIsAnimPass = TRUE;
			ASI.bAnimPeriodPassed = FALSE;
		}
		else //if ap_NumOfSet1 == ap_NumOfSet2
		{
			if( OnceAnimation() == TRUE )
			{
				m_pAnimController->ResetTime();
				m_pAnimController->AdvanceTime(0,NULL);
			}
		}

	}*/


	return S_OK;
}





//-----------------------------------------------------------------------------
// Name: UpdateFrameMatricesAnimPass()
// Desc:
//-----------------------------------------------------------------------------
void CModel::UpdateFrameMatricesAnimPass(LPD3DXFRAME pFrameBase, LPD3DXMATRIX pParentMatrix)
{
	//first check
	if(m_pAnimController == NULL) { //HTMLLog("Error! UpdateFrameMatricesAnimPass(): animcontroller == NULL, object %d", this->id );
									return; } //error!


    D3DXFRAME_DERIVED *pFrame = (D3DXFRAME_DERIVED*)pFrameBase;
	D3DXMATRIX matrix1, matrix2;
	

	//setting up the matrices
	LPD3DXANIMATIONSET AnimSet;

	m_pAnimController->GetAnimationSet(ASI.ap_NumOfSet1, &AnimSet);
	m_pAnimController->SetTrackAnimationSet(0, AnimSet);
	m_pAnimController->SetTrackPosition(0, ASI.ap_BreakTime);
	m_pAnimController->AdvanceTime(0, NULL);
	matrix1 = pFrame->TransformationMatrix;

	m_pAnimController->GetAnimationSet(ASI.ap_NumOfSet2, &AnimSet);
	m_pAnimController->SetTrackAnimationSet(0, AnimSet);
	m_pAnimController->SetTrackPosition(0,0);
	m_pAnimController->AdvanceTime(0, NULL);
	matrix2 = pFrame->TransformationMatrix;



	ASI.ap_CurTime = (float)timeGetTime();
	double Scalar = (ASI.ap_CurTime - ASI.ap_StartTime) / (ASI.ap_fTime * 1000.f);

	D3DXMATRIX TransformationMatrix;
	


	//decomposing matrices
	
	D3DXQUATERNION decomp1rot, decomp2rot, quatRes;
	D3DXVECTOR3 vecTrans1, vecTrans2, transRes;
	D3DXVECTOR3 vecScale1, vecScale2;
	D3DXMatrixDecompose(&vecScale1, &decomp1rot, &vecTrans1, &matrix1);
	D3DXMatrixDecompose(&vecScale2, &decomp2rot, &vecTrans2, &matrix2);
	D3DXQuaternionSlerp( &quatRes, &decomp1rot, &decomp2rot, (float)Scalar );

	transRes = vecTrans1 + (vecTrans2-vecTrans1)*Scalar;

	D3DXMatrixTransformation( &TransformationMatrix, NULL, NULL, NULL, NULL, &quatRes, &transRes );
	
	
	//base cycle
	pFrame->CombinedTransformationMatrix = TransformationMatrix;
	if (pParentMatrix != NULL)
		pFrame->CombinedTransformationMatrix *= (*pParentMatrix);



    if (pFrame->pFrameSibling != NULL)
    {
        UpdateFrameMatricesAnimPass(pFrame->pFrameSibling, pParentMatrix);
    }

    if (pFrame->pFrameFirstChild != NULL)
    {
        UpdateFrameMatricesAnimPass(pFrame->pFrameFirstChild, &pFrame->CombinedTransformationMatrix);
    }

	if(ASI.ap_CurTime > ASI.ap_StartTime + ASI.ap_fTime*1000.f)
	{
		m_pAnimController->GetAnimationSet(ASI.ap_NumOfSet2, &AnimSet);
		m_pAnimController->SetTrackAnimationSet(0, AnimSet);
		m_pAnimController->ResetTime();
		m_pAnimController->AdvanceTime(0, NULL);
		ASI.AnimTime = 0;

		ASI.ap_bIsAnimPass = FALSE;
	}
}


//-----------------------------------------------------------------------------
// Name: UpdateAttachmentMatrix
// Desc:
//-----------------------------------------------------------------------------
void CModel::UpdateAttachmentMatrix()
{
	CurrentAttachmentTransformMatrix = * a_TransformMatrix;
}

//-----------------------------------------------------------------------------
// Name: UpdateFrameMatrices()
// Desc:
//-----------------------------------------------------------------------------
void CModel::UpdateFrameMatrices(LPD3DXFRAME pFrameBase, LPD3DXMATRIX pParentMatrix)
{
    D3DXFRAME_DERIVED *pFrame = (D3DXFRAME_DERIVED*)pFrameBase;

	//attach
	if( bAttached == TRUE )
	{
		pFrame->CombinedTransformationMatrix = pFrame->TransformationMatrix * (CurrentAttachmentTransformMatrix);
		//pFrame->CombinedTransformationMatrix *= (*a_OffsetMatrix) * (*a_TransformMatrix);
	}
	else
	{
		pFrame->CombinedTransformationMatrix = pFrame->TransformationMatrix;
		if (pParentMatrix != NULL)
			pFrame->CombinedTransformationMatrix *= (*pParentMatrix);
	}	



    if (pFrame->pFrameSibling != NULL)
    {
        UpdateFrameMatrices(pFrame->pFrameSibling, pParentMatrix);
    }

    if (pFrame->pFrameFirstChild != NULL)
    {
        UpdateFrameMatrices(pFrame->pFrameFirstChild, &pFrame->CombinedTransformationMatrix);
    }
}


