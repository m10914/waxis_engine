//-----------------------------------------------------------------------------
// File: SkinnedMesh.cpp
//
// Desc: DirectX window application created by the DirectX AppWizard
//-----------------------------------------------------------------------------
#include "SkinnedMesh.h"


#define ANGANIM 0.07f
#define SPEED 110.0f




//-----------------------------------------------------------------------------
// Global access to the app (needed for the global WndProc())
//-----------------------------------------------------------------------------
CMyD3DApplication* g_pApp  = NULL;
HINSTANCE          g_hInst = NULL;
float			   g_fElapsedTime;



//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point to the program. Initializes everything, and goes into a
//       message-processing loop. Idle time is used to render the scene.
//-----------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
    CMyD3DApplication d3dApp;

    g_pApp  = &d3dApp;
    g_hInst = hInst;

    InitCommonControls();
    if( FAILED( d3dApp.Create( hInst ) ) )
        return 0;

    return d3dApp.Run();
}



//-----------------------------------------------------------------------------
// Name: PHYSX BLOCK
// Desc: PhysX Stuff
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InitNx()
{
	gAllocator = new UserAllocator;


	// Initialize PhysicsSDK
	//NxPhysicsSDKDesc desc;
	//NxSDKCreateError errorCode = NXCE_NO_ERROR;
	gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, gAllocator);//, new ErrorStream(), desc, &errorCode);
	if(gPhysicsSDK == NULL) 
	{
		HTMLLog("\nSDK create error .\nUnable to initialize the PhysX SDK.\n\n");
		return E_FAIL;
	}
		

	gPhysicsSDK->setParameter(NX_SKIN_WIDTH, 0.005f);

	// Create a scene
	NxSceneDesc sceneDesc;
	sceneDesc.userContactReport = &gContactReport;
	sceneDesc.gravity = NxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.simType = NX_SIMULATION_SW;
	gScene = gPhysicsSDK->createScene(sceneDesc);
	if(gScene == NULL) 
	{
		HTMLLog("\nError: Unable to create a PhysX scene, exiting.\n\n");
		return false;
	}

	// Set default material
	NxMaterial* defaultMaterial = gScene->getMaterialFromIndex(0);
	defaultMaterial->setRestitution(0.0f);
	defaultMaterial->setStaticFriction(0.5f);
	defaultMaterial->setDynamicFriction(0.5f);


	//init char controller
	gManager = NxCreateControllerManager(gAllocator);

	//trigger callback
	gScene->setUserTriggerReport( &myTriggerCallback );

	//actor pair flags (???)

	//for ragdoll stuff
	gScene->setGroupCollisionFlag( GROUP_CONTROLLER, GROUP_BIPED, false);
	//gScene->setGroupCollisionFlag( GROUP_BIPED, GROUP_BIPED, false);

	return S_OK;
}


HRESULT CMyD3DApplication::StartNx()
{
	gScene->simulate( 100*m_fElapsedTime ); //milliseconds to seconds
	return S_OK;
}


HRESULT CMyD3DApplication::FetchNx()
{
	gScene->flushStream();
	gScene->fetchResults(NX_RIGID_BODY_FINISHED, true);

	return S_OK;
}


HRESULT CMyD3DApplication::ReleaseNx()
{
	if(gPhysicsSDK != NULL)
	{
		if( gManager ) NxReleaseControllerManager(gManager);

		if(gScene != NULL) gPhysicsSDK->releaseScene(*gScene);
		gScene = NULL;

		NxReleasePhysicsSDK(gPhysicsSDK);
		gPhysicsSDK = NULL;

	}

	return S_OK;
}





//-----------------------------------------------------------------------------
// Name: CMyD3DApplication()
// Desc: Application constructor.   Paired with ~CMyD3DApplication()
//       Member variables should be initialized to a known state here.  
//       The application window has not yet been created and no Direct3D device 
//       has been created, so any initialization that depends on a window or 
//       Direct3D should be deferred to a later stage. 
//-----------------------------------------------------------------------------
CMyD3DApplication::CMyD3DApplication()
{
	//vars
	int i;


	//log initialization
	InitLog();
	

	//application constructor
	m_bStartFullscreen = TRUE;
	m_bWindowed = false;
	m_bShowCursorWhenFullscreen	= true;
    
	m_bOneTimeRender = true;

	//timing
	TimeQ = 10000;
	WorldTime = 12.0f;

	m_d3dEnumeration.AppUsesDepthBuffer   = TRUE;
	m_d3dEnumeration.AppUsesDepthBuffer   = TRUE;
    m_d3dEnumeration.AppUsesMixedVP = TRUE;
	m_d3dEnumeration.AppMinDepthBits = 16;
	m_d3dEnumeration.AppMinStencilBits = 4;



	//engine stuff
	
	FILE* file = fopen("config.ini", "r");


	FindWord(file, "[ViewDistanceObject]");
	VIEWDISTANCE_OBJECT = atoi(GetWord(file));

	FindWord(file, "[ViewDistanceTerrain]");
	VIEWDISTANCE_TERRAIN = atoi(GetWord(file));


	FindWord(file, "[Resolution]");
	SCREENWIDTH = atoi(GetWord(file));
	SCREENHEIGHT = atoi(GetWord(file));

	bShadowsEnable = TRUE;
	bBoundingBoxes = FALSE;

	fclose(file);

	//-----

	en_box = NULL;

	input = NULL;
	camera = NULL;
	terrain = NULL;
	ParticleSystem = NULL;

	pic = NULL;


    // Create a D3D font using d3dfont.cpp
    m_pFont                     = new CD3DFont( _T("Arial Unicode MS"), 12, D3DFONT_BOLD );
    m_bLoadingApp               = TRUE;
   
	
	//-- music stuff
	
	//wav
	m_pMusicManager = NULL;
    //m_pSound = NULL;
	
	//mp3
	m_pBackgroundMusic = NULL;
	
	//--


	// PhysX
	gPhysicsSDK = NULL;
	gScene = NULL;
	gAllocator = NULL;

	gManager = NULL;
	//CapsuleController = NULL;



	//console chat menu
	
	bConsole = FALSE;
	consoleTime = timeGetTime();
	consoleSymNum = 0;
	consoleStr[0] = '\0';

	for( i=0; i < CONSOLEHISTORYNUM; i++) consoleHistory[i][0] = '\0';
	consoleHistoryCur = CONSOLEHISTORYNUM-1;

	bChat = FALSE;
	chatTime = timeGetTime();
	chatSymNum = 0;
	chatStr[0] = '\0';

	for( i=0; i < CHATHISTORYNUM; i++) chatHistory[i][0] = '\0';
	chatHistoryCur = CHATHISTORYNUM-1;

	EnterTime = timeGetTime();


	bMenu = TRUE;
	menuTime = timeGetTime();
	curmenu = MAINMENU;
}




//-----------------------------------------------------------------------------
// Name: ~CMyD3DApplication()
// Desc: Application destructor.  Paired with CMyD3DApplication()
//-----------------------------------------------------------------------------
CMyD3DApplication::~CMyD3DApplication()
{
}




//-----------------------------------------------------------------------------
// Name: OneTimeSceneInit()
// Desc: Paired with FinalCleanup().
//       The window has been created and the IDirect3D9 interface has been
//       created, but the device has not been created yet.  Here you can
//       perform application-related initialization and cleanup that does
//       not depend on a device.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::OneTimeSceneInit()
{
	//Display mode settings
	//---------------------
    D3DDISPLAYMODE* pdm = &m_d3dSettings.Fullscreen_DisplayMode;

	pdm->Width = SCREENWIDTH;
	pdm->Height = SCREENHEIGHT;
	pdm->RefreshRate = 60;



    // Drawing loading status message until app finishes loading
    SendMessage( m_hWnd, WM_PAINT, 0, 0 );


    // Initialize audio
    InitAudio( m_hWnd );


    m_bLoadingApp = FALSE;

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
int CMyD3DApplication::GetCModelIndexByID( vector<CModel*> cm, int NumOfcm, int id)
{
	int i;
	for( i = 0; i < NumOfcm; i++ )
	{
		if( cm[i]->id == id ) break;
	}

	if(i == NumOfcm) return -1;
	else return i;
}

//-----------------------------------------------------------------------------
// Name: InitEffect()
// Desc: Initialize DirectX effect
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InitEffect(void)
{
	LPD3DXBUFFER pBufferErrors = NULL;
	if(FAILED( D3DXCreateEffectFromFile( m_pd3dDevice,
										 L"data/shaders/ShadowsTransform.fx",
										 NULL, NULL, 
										 D3DXFX_DONOTSAVESTATE | D3DXSHADER_OPTIMIZATION_LEVEL3,
										 NULL,
										 &m_pEffect, &pBufferErrors ) ))
	{
		HTMLLog(" InitEffect() error:\n %s \n",(char*)pBufferErrors->GetBufferPointer());
		return E_FAIL;
	}

	HRESULT hr = m_pEffect->ValidateTechnique( m_pEffect->GetTechnique(0) );

	return S_OK;
}


//-----------------------------------------------------------------------------
// Name: InitAudio()
// Desc: Initialize DirectX audio objects
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InitAudio( HWND hWnd )
{
    HRESULT hr;

    // Create the music manager class, used to create the sounds
    m_pMusicManager = new CMusicManager();
    if( FAILED( hr = m_pMusicManager->Initialize( hWnd ) ) )
        LogPlease( "Error: m_pMusicManager->Initialize" );


	

	//directshow
	m_pBackgroundMusic = new CMp3MusicManager;
	m_pBackgroundMusic->Init();
	m_pBackgroundMusic->PlayFile("data/music/mesmeria.mp3", FALSE);

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ConfirmDevice()
// Desc: Called during device initialization, this code checks the display device
//       for some minimum set of capabilities
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::ConfirmDevice( D3DCAPS9* pCaps, DWORD dwBehavior,
                                          D3DFORMAT Format )
{
	if(!Format) return E_FAIL;

	// Always request MIXED or SOFTWARE vertex processing do deal with fall backs
    if ((dwBehavior & D3DCREATE_PUREDEVICE) || (dwBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING))
        return E_FAIL;

    // If using mixed, make sure the hardware can do vertex blending
    if( ( dwBehavior & D3DCREATE_MIXED_VERTEXPROCESSING ) &&
        pCaps->MaxVertexBlendMatrices < 2 )
    {
        return E_FAIL;
    }

	return S_OK;
}



// -- snow particles
	// particles transformation functions	
	D3DXVECTOR3 snow_f1( D3DXVECTOR3 emitpos) //birth point
	{
		return D3DXVECTOR3( emitpos.x + rand()%5000-2500,
							emitpos.y + rand()%1000,
							emitpos.z + rand()%5000-2500);
	}

	D3DXVECTOR3 snow_f2( void) // start speed
	{
		return D3DXVECTOR3( 0,
							-300,
							0 );
	}

	D3DXVECTOR3 snow_f3( D3DXVECTOR3 vel) // time speed
	{
		return D3DXVECTOR3(vel.x + rand()%3 - 1,
						   vel.y - 55.5f*g_fElapsedTime,
						   vel.z + rand()%3 - 1);
	}


	D3DXVECTOR3 snow_f4( D3DXVECTOR3 emitpos, D3DXVECTOR3 oldpos, D3DXVECTOR3 vel) //time pos
	{
		return D3DXVECTOR3( oldpos.x + vel.x*g_fElapsedTime,
							oldpos.y + vel.y*g_fElapsedTime,
							oldpos.z + vel.z*g_fElapsedTime );
	}
//


// -- blood particles
	// particles transformation functions	
	D3DXVECTOR3 blood_f1( D3DXVECTOR3 emitpos) //birth point
	{
		return D3DXVECTOR3( emitpos.x,
							emitpos.y,
							emitpos.z);
	}

	D3DXVECTOR3 blood_f2( void) // start speed
	{
		return D3DXVECTOR3( rand()%80,
							rand()%10,
							rand()%80 );
	}

	D3DXVECTOR3 blood_f3( D3DXVECTOR3 vel) // time speed
	{
		return D3DXVECTOR3(vel.x,
						   vel.y - 355.5f*g_fElapsedTime,
						   vel.z);
	}


	D3DXVECTOR3 blood_f4( D3DXVECTOR3 emitpos, D3DXVECTOR3 oldpos, D3DXVECTOR3 vel) //time pos
	{
		return D3DXVECTOR3( oldpos.x + vel.x*g_fElapsedTime,
							oldpos.y + vel.y*g_fElapsedTime,
							oldpos.z + vel.z*g_fElapsedTime );
	}
//


// -- fire particles
	// particles transformation functions	
	D3DXVECTOR3 fire_f1( D3DXVECTOR3 emitpos) //birth point
	{
		return D3DXVECTOR3( emitpos.x,
							emitpos.y,
							emitpos.z);
	}

	D3DXVECTOR3 fire_f2( void) // start speed
	{
		return D3DXVECTOR3( rand()%10,
							rand()%20,
							rand()%10 );
	}

	D3DXVECTOR3 fire_f3( D3DXVECTOR3 vel) // time speed
	{
		return D3DXVECTOR3(vel.x,
						   vel.y + 15.5f*g_fElapsedTime,
						   vel.z);
	}


	D3DXVECTOR3 fire_f4( D3DXVECTOR3 emitpos, D3DXVECTOR3 oldpos, D3DXVECTOR3 vel) //time pos
	{
		return D3DXVECTOR3( oldpos.x + vel.x*g_fElapsedTime,
							oldpos.y + vel.y*g_fElapsedTime,
							oldpos.z + vel.z*g_fElapsedTime );
	}
//


// -- magic particles
	// particles transformation functions	
	D3DXVECTOR3 magic_f1( D3DXVECTOR3 emitpos) //birth point
	{
		int x = rand();
		return D3DXVECTOR3( emitpos.x + 40*cos( (float)x ),
							emitpos.y + rand()%60,
							emitpos.z + 40*sin( (float)x ) );
	}

	D3DXVECTOR3 magic_f2( void) // start speed
	{
		return D3DXVECTOR3( 0,
							rand()%30,
							0);
	}

	D3DXVECTOR3 magic_f3( D3DXVECTOR3 vel) // time speed
	{
		return D3DXVECTOR3(vel.x,
						   vel.y + 15.5f*g_fElapsedTime,
						   vel.z);
	}


	D3DXVECTOR3 magic_f4( D3DXVECTOR3 emitpos, D3DXVECTOR3 oldpos, D3DXVECTOR3 vel) //time pos
	{
		return D3DXVECTOR3( oldpos.x + vel.x*g_fElapsedTime,
							oldpos.y + vel.y*g_fElapsedTime,
							oldpos.z + vel.z*g_fElapsedTime );
	}
//




//-----------------------------------------------------------------------------
// Name: InitDeviceObjects()
// Desc: Paired with DeleteDeviceObjects()
//       The device has been created.  Resources that are not lost on
//       Reset() can be created here -- resources in D3DPOOL_MANAGED,
//       D3DPOOL_SCRATCH, or D3DPOOL_SYSTEMMEM.  Image surfaces created via
//       CreateImageSurface are never lost and can be created here.  Vertex
//       shaders and pixel shaders can also be created here as they are not
//       lost on Reset().
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InitDeviceObjects()
{
	//variables
	int i,j;
	WCHAR wstring[512];
	


	// Init the font
    m_pFont->InitDeviceObjects( m_pd3dDevice );

	// Init the Effect
	if(FAILED( InitEffect() )) LogPlease("application::InitEffect - FAILED");

	//camera
	camera = new CCamera;


	//----- PHYSX LOAD
	HTMLLog("initializing physx... ");
	if(SUCCEEDED(InitNx())) HTMLLog("... success!\n");
	




	//----------------------------------------------
	//----- LEVEL LOAD -----------------------------
	//----------------------------------------------


	//open level file descriptor
	FILE* file = fopen("data/levels/testlevel.lvl","r");


	//---- models initialization ----



	//objects
	FindWord(file, "[Objects]");

	NumOfObjects = atoi(GetWord(file));
	//Objects = new CModel[NumOfObjects];
	
	HTMLLog("\n-= Init Objects :: %d =-\n", NumOfObjects);

	for(i=0; i < NumOfObjects; i++)
	{
		Objects.push_back(NULL);
		Objects[i] = new CModel();
		Objects[i]->id = i;

		Objects[i]->meshPath = GetWord(file);
		
		for(j=0; j<i; j++) if( !stricmp(Objects[i]->meshPath, Objects[j]->meshPath) ) break;
		if(i==j) Objects[i]->dublicate = FALSE;
		else Objects[i]->dublicate = TRUE;

		MultiByteToWideChar( CP_ACP, 0, Objects[i]->meshPath, -1, wstring, 512 );
		Objects[i]->Init(m_pd3dDevice, m_d3dCaps, wstring, m_pEffect, Objects[j]);

		Objects[i]->SetCoordinate( ConstructVec3(GetWord(file)) );
		Objects[i]->SetAngle( ConstructVec3(GetWord(file)) );

		Objects[i]->InitPhysX(gPhysicsSDK, gScene, gManager, OT_OBJECT);
		//Objects[i].GenerateCollisionModel();
	}



	//Animations
	FindWord(file, "[Animations]");

	NumOfAnimations = atoi(GetWord(file));

	//Animations = new CMyModel[NumOfAnimations];

	HTMLLog("\n-= Init Animations :: %d =-\n", NumOfAnimations);

	for(i=0; i < NumOfAnimations; i++)
	{
		Animations.push_back(NULL);
		Animations[i] = new CMyModel();
		Animations[i]->id = i;
		Animations[i]->MAnimType = atoi(GetWord(file));

		//MAnimType must be = i;
		if(Animations[i]->MAnimType != i) HTMLLog("\n\n<b>Error! Animation type ain't right!</b>\n\n");

		Animations[i]->MType = BASE;
		Animations[i]->ParentModel = NULL;

		Animations[i]->meshPath = GetWord(file);
		Animations[i]->dublicate = FALSE;

		MultiByteToWideChar( CP_ACP, 0, Animations[i]->meshPath, -1, wstring, 512 );
		Animations[i]->Init(m_pd3dDevice, m_d3dCaps, wstring, m_pEffect, Animations[i]);

	}



	//SkinMeshes
	FindWord(file, "[SkinMeshes]");

	NumOfSkinMesh = atoi(GetWord(file));
	

	HTMLLog("\n-= Init SkinMeshes & Heads :: %d =-\n", NumOfSkinMesh);

	for(i=0; i < NumOfSkinMesh; i++)
	{
		SkinMesh.push_back(NULL);
		SkinMesh[i] = new CMyModel();
		Heads.push_back(NULL);
		Heads[i] = new CModel();

		SkinMesh[i]->id = i;
		Heads[i]->id = i;

		//defining type of mesh (maybe no need to initialize)
		SkinMesh[i]->MAnimType = atoi(GetWord(file));


		SkinMesh[i]->MType = CLONE;
		SkinMesh[i]->ParentModel = Animations[ SkinMesh[i]->MAnimType ];


		SkinMesh[i]->meshPath = GetWord(file);
		for(j=0; j<i; j++) if( !stricmp(SkinMesh[i]->meshPath, SkinMesh[j]->meshPath) ) break;
		if(i==j) SkinMesh[i]->dublicate = FALSE;
		else SkinMesh[i]->dublicate = TRUE;

		Heads[i]->meshPath = GetWord(file);
		for(j=0; j<i; j++) if( !stricmp(Heads[i]->meshPath, Heads[j]->meshPath) ) break;
		if(i==j) Heads[i]->dublicate = FALSE;
		else Heads[i]->dublicate = TRUE;

		MultiByteToWideChar( CP_ACP, 0, SkinMesh[i]->meshPath, -1, wstring, 512 );
		SkinMesh[i]->Init(m_pd3dDevice, m_d3dCaps, wstring, m_pEffect, SkinMesh[j]);

		MultiByteToWideChar( CP_ACP, 0, Heads[i]->meshPath, -1, wstring, 512 );
		Heads[i]->Init(m_pd3dDevice, m_d3dCaps, wstring, m_pEffect, Heads[j]);

		SkinMesh[i]->speed = D3DXVECTOR3(0,0,0);
		SkinMesh[i]->SetCoordinate( ConstructVec3(GetWord(file)) );
		SkinMesh[i]->oldCoord = SkinMesh[i]->coordinate;
		SkinMesh[i]->SetAngle( D3DXVECTOR3(0,0,0) );


		SkinMesh[i]->InitPhysX(gPhysicsSDK, gScene, gManager, OT_CHARACTER);
	}



	//--temporary log
	HTMLLog("\n\t\t--<b>*SkinMesh and *Heads vectors has been created: %d of %d\n</b>", SkinMesh.size(), NumOfSkinMesh);
	for(i=0; i < NumOfSkinMesh; i++)
	{
		HTMLLog("\t\t--%d : id: %d; type: %d; meshPath %s\n", i, SkinMesh[i]->id, SkinMesh[i]->MType, SkinMesh[i]->meshPath );
	}


	//Weapons
	FindWord(file, "[Weapons]");

	NumOfWeapons = atoi(GetWord(file));
	//Weapons = new CModel[NumOfWeapons];
	
	HTMLLog("\n-= Init Weapons :: %d =-\n", NumOfWeapons);

	for(i=0; i < NumOfWeapons; i++)
	{
		Weapons.push_back(NULL);
		Weapons[i] = new CModel();
		Weapons[i]->id = i;

		Weapons[i]->meshPath = GetWord(file);
		for(j=0; j<i; j++) if( !stricmp(Weapons[i]->meshPath, Weapons[j]->meshPath) ) break;
		if(i==j) Weapons[i]->dublicate = FALSE;
		else Weapons[i]->dublicate = TRUE;

		MultiByteToWideChar( CP_ACP, 0, Weapons[i]->meshPath, -1, wstring, 512 );
		Weapons[i]->Init(m_pd3dDevice, m_d3dCaps, wstring, m_pEffect, Weapons[j]);

		Weapons[i]->SetCoordinate( ConstructVec3(GetWord(file)) );
		Weapons[i]->SetAngle( D3DXVECTOR3(0,0,0) );


		Weapons[i]->InitPhysX(gPhysicsSDK, gScene, gManager, OT_WEAPON);
	}


	//Armor
	FindWord(file, "[Armor]");

	NumOfArmor = atoi(GetWord(file));
	//Armor = new CModel[NumOfArmor];
	
	HTMLLog("\n-= Init Armor :: %d =-\n", NumOfArmor);

	for(i=0; i < NumOfArmor; i++)
	{
		Armor.push_back(NULL);
		Armor[i] = new CModel();
		Armor[i]->id = i;

		Armor[i]->meshPath = GetWord(file);
		for(j=0; j<i; j++) if( !stricmp(Armor[i]->meshPath, Armor[j]->meshPath) ) break;
		if(i==j) Armor[i]->dublicate = FALSE;
		else Armor[i]->dublicate = TRUE;

		MultiByteToWideChar( CP_ACP, 0, Armor[i]->meshPath, -1, wstring, 512 );
		Armor[i]->Init(m_pd3dDevice, m_d3dCaps, wstring, m_pEffect, Armor[j]);
	}




	//special
	/*for( i = 0; i < NumOfSkinMesh; i++ )
		for( j = 0; j < NumOfWeapons; j++ )
		{
			gScene->setActorPairFlags( *SkinMesh[i]->object, *Weapons[j]->object, //NX_IGNORE_PAIR ); 
																		  NX_NOTIFY_ON_START_TOUCH |   
                                                                          NX_NOTIFY_ON_TOUCH | 
                                                                          NX_NOTIFY_ON_END_TOUCH );
		}
	*/
	SkinMesh[0]->Attach(Weapons[0], "Bip01_Scubbard1");

	SkinMesh[1]->CurrentArmor = 0;

	SkinMesh[3]->Attach(Weapons[1], "Bip01_R_Hand");
	SkinMesh[3]->bIsWpnDrawned = TRUE;

	//humor
	
	//SkinMesh[2]->bFree = TRUE;
	//SkinMesh[2]->bDead = TRUE;
	//for( i = 0; i < SkinMesh[2]->NumOfBiped; i++ ) SkinMesh[2]->biped[i]->clearBodyFlag( NX_BF_KINEMATIC );

	//---------



	//---- terrain initialization ----

	texTable = new CTextureTable;
	texTable->CreateFromFile(m_pd3dDevice, "data/textures/terrain/snowtable.txt");


	FindWord(file,"[Terrain]");

	NumOfCTerrain = atoi( GetWord(file) );
	terrain = new CTerrain[NumOfCTerrain];

	sprintf(logstr, "\n-= Init Terrain :: %d =-\n", NumOfCTerrain); LogPlease(logstr);

	for(i=0; i < NumOfCTerrain; i++)
	{
		terrain[i].Init(GetString(file),
						m_pd3dDevice, texTable,
						m_pEffect);
		terrain[i].SetArea(VIEWDISTANCE_TERRAIN);
		terrain[i].InitPhysX(gPhysicsSDK, gScene);
	}


	//---------


	//---- Light initialization ----

	FindWord(file,"[Lights]");

	NumOfLights = atoi( GetWord(file) );
	dirlights = new DIRLIGHT[NumOfLights];

	for(i=0; i < NumOfLights; i++)
	{
		dirlights[i].Pos = ConstructVec3(GetWord(file));
		dirlights[i].Aim = ConstructVec3(GetWord(file));
	}

	//---------




	//skybox
	skybox = new CSkyBox;
	skybox->Init(m_pd3dDevice, m_pEffect, L"data/models/SkyDome.x");
	skybox->LoadTexture(L"data/textures/skybox/cloudnight.dds");

	//env color
	EnvColor = 0xeeeedd;



	//water
	HTMLLog("\n-= Init Water  =-\n");
	water = new CWater;
	water->Init(m_pd3dDevice, m_pEffect, D3DXVECTOR3(0,0,0), D3DXVECTOR2(200000,200000), D3DXVECTOR2(200,200) );



	//particles
	NumOfParticle = 4;
	ParticleSystem = new CParticleSystemParent[NumOfParticle];

	HTMLLog("\n-= Init ParticleSystems :: %d =-\n", NumOfParticle);


	//snow particle
		ParticleSystem[0].Init(L"data/textures/PAR_snow.dds",
								0.1f,
								m_pd3dDevice,
								m_pEffect,
								10000.0f,
								10000,
								snow_f1,
								snow_f2,
								snow_f3,
								snow_f4);
		ParticleSystem[0].SetIntensity(1600);


	//blood particle
		ParticleSystem[1].Init(L"data/textures/PAR_blood.dds",
								0.1f,
								m_pd3dDevice,
								m_pEffect,
								17000.0f,
								2000,
								blood_f1,
								blood_f2,
								blood_f3,
								blood_f4);
		ParticleSystem[1].SetIntensity(0);


	//fire particle
		ParticleSystem[2].Init(L"data/textures/PAR_fire.dds",
								0.1f,
								m_pd3dDevice,
								m_pEffect,
								80700.0f,
								100,
								fire_f1,
								fire_f2,
								fire_f3,
								fire_f4);
		ParticleSystem[2].SetIntensity(0);


	//magic particle
		ParticleSystem[3].Init(L"data/textures/PAR_magic.dds",
								0.1f,
								m_pd3dDevice,
								m_pEffect,
								30700.0f,
								1000,
								magic_f1,
								magic_f2,
								magic_f3,
								magic_f4);
		ParticleSystem[3].SetIntensity(0);



	//decals
	NumOfDecals = 1;
	Decals = new CDecal[NumOfDecals];
	HTMLLog("\n-= Init Decals :: %d =-\n", NumOfDecals);

	for(i = 0; i < NumOfDecals; i++)
	{
		Decals[i].Init("Dec01.dds", m_pd3dDevice, m_pEffect);
	}


	//create input
	input = new Input;
	input->CreateInput(g_hInst);
	input->CreateKeyboard(m_hWnd);
	input->CreateMouse(m_hWnd);



	//init level file
	fclose(file);



	//init shadow maps
	HTMLLog("\n-= Init ShadowMaps =-\n");	

	if(FAILED(m_pd3dDevice->CreateTexture(	 SHADOWMAPSIZE,
											 SHADOWMAPSIZE,
											 1,
											 D3DUSAGE_RENDERTARGET,
											 D3DFMT_R32F,
											 D3DPOOL_DEFAULT,
											 &g_pShadowMap,
											 NULL ))) return E_FAIL;
	g_pShadowMap->GetSurfaceLevel( 0, &g_pShadowSurf );


	if(FAILED(m_pd3dDevice->CreateDepthStencilSurface( SHADOWMAPSIZE,
													   SHADOWMAPSIZE,
													   D3DFMT_D16,
													   D3DMULTISAMPLE_NONE,
													   0,
													   TRUE,
													   &g_pShadowDepth,
													   NULL))) return E_FAIL;

	if(FAILED(m_pd3dDevice->CreateDepthStencilSurface( SHADOWMAPSIZE,
													   SHADOWMAPSIZE,
													   D3DFMT_D16,
													   D3DMULTISAMPLE_NONE,
													   0,
													   TRUE,
													   &g_pNewDepthRT,
													   NULL))) return E_FAIL;	






	//init GUI
	HTMLLog("\n-= Init GUI =-\n");
	NumOfPics = 6;
	pic = new CPic[NumOfPics];

	pic[0].Init(m_pd3dDevice, L"data/textures/gui/menu.dds", SCREENHEIGHT, SCREENWIDTH);
	pic[1].Init(m_pd3dDevice, L"data/textures/gui/healthbar.dds", SCREENHEIGHT/46, SCREENWIDTH/5);
	pic[2].Init(m_pd3dDevice, L"data/textures/gui/manabar.dds", SCREENHEIGHT/46, SCREENWIDTH/5);
	pic[3].Init(m_pd3dDevice, L"data/textures/gui/chatback.dds", 250, SCREENWIDTH * 0.35f);
	pic[4].Init(m_pd3dDevice, L"data/textures/gui/chatbackB.dds", 250, SCREENWIDTH * 0.35f);
	pic[5].Init(m_pd3dDevice, L"data/textures/gui/cross.dds", SCREENHEIGHT * 0.01f, SCREENWIDTH * 0.01f);

	

	//init engine stuff
	


	//end of engine stuff



	LogPlease("\n\n-= Initialization Successful! =-\n\n");
	LogPlease("\n\n-= Starting in-game Log =-\n\n");

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: Paired with InvalidateDeviceObjects()
//       The device exists, but may have just been Reset().  Resources in
//       D3DPOOL_DEFAULT and any other device state that persists during
//       rendering should be set here.  Render states, matrices, textures,
//       etc., that don't change during rendering can be set once here to
//       avoid redundant state setting during Render() or FrameMove().
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RestoreDeviceObjects()
{

    // Setup render state
    m_pd3dDevice->SetRenderState( D3DRS_LIGHTING,         TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE,     TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,          TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,         D3DCULL_CCW );
    m_pd3dDevice->SetRenderState( D3DRS_AMBIENT,          0x33333333 );
    m_pd3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );

    // Restore the font
    m_pFont->RestoreDeviceObjects();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::ProcessConsoleMsg(void)
{

	//parse
	int argc = 1, i;
	char** argv;
	char *c, *pc;

	for( c = consoleStr; *c != '\0'; c++ ) if( *c == ' ' ) argc++;
	
	argv = new char*[argc];
	for( i=0; i < argc; i++ ) argv[i] = new char[256];

	c = consoleStr;
	for( i=0; i < argc; i++ )
	{
		pc = argv[i];
		for(; *c != '\0' && *c != ' '; c++, pc++) *pc = *c;
		*pc = '\0';
		c++;
	}


	//PROCESS
	//-----

	//quit command
		 if( !strcmp( argv[0], "quit" ) )
	{
		//quit
		DeleteDeviceObjects();
		FinalCleanup();
		exit(0);
	}

	//kill & reanimate commands
	else if( !strcmp( argv[0], "kill" ) && argc == 2 )
	{
		SkinMesh[ atoi(argv[1]) ]->life = -100;
		SkinMesh[ atoi(argv[1]) ]->bDead = TRUE;
	}
	else if( !strcmp( argv[0], "reanimate" ) && argc == 2 )
	{
		SkinMesh[ atoi(argv[1]) ]->life = 100;
		SkinMesh[ atoi(argv[1]) ]->bDead = FALSE;
	}


	//teleport command
	else if( !strcmp( argv[0], "teleport" ) && argc == 4 )
	{
		SkinMesh[0]->coordinate.x = atoi(argv[1]);
		SkinMesh[0]->coordinate.y = atoi(argv[2]);
		SkinMesh[0]->coordinate.z = atoi(argv[3]);
		SkinMesh[0]->speed.y = 0;
	}

	//set time command
	else if( !strcmp( argv[0], "time" ) && argc == 2 )
	{
		WorldTime = atoi( argv[1] );
	}

	//insert command (two types)
	else if( !strcmp( argv[0], "insert" ) )
	{
		if(argc == 5)
		{
			AddMessageToConsole("adding actor...\n");
			AddActor( atoi(argv[1]), argv[2], argv[3], ConstructVec3(argv[4]) );
		}
		else if(argc == 2)
		{	
			AddMessageToConsole("adding actor...\n");
			if( !strcmp( argv[1], "bot" ) )
				AddActor( 0, "data/models/man01.x", "data/models/head01.x", D3DXVECTOR3(500,500,500) );
		}
		else
		{
			AddMessageToConsole("insert: missing arguments\n");
		}
	}


	//unknown command
	else
	{
		AddMessageToConsole("Unknown command");
	}



	HTMLLog("ConsoleCommand entered, argc %d, argv:", argc);
	for( i = 0; i < argc; i++ ) HTMLLog(" %s", argv[i]);
	HTMLLog("\n");

	return S_OK;
}


//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::ProcessChatMsg(void)
{
	//parse
	int argc = 1, i;
	char** argv;
	char *c, *pc;

	for( c = chatStr; *c != '\0'; c++ ) if( *c == ' ' ) argc++;
	
	argv = new char*[argc];
	for( i=0; i < argc; i++ ) argv[i] = new char[256];

	c = chatStr;
	for( i=0; i < argc; i++ )
	{
		pc = argv[i];
		for(; *c != '\0' && *c != ' '; c++, pc++) *pc = *c;
		*pc = '\0';
		c++;
	}



	// spellcasting
		 if( !strcmp( argv[0], "edro" ) )
	{

		i=0;
		if( !ONCEANIMATIONPERIOD )
		{
			SkinMesh[0]->PassAnimation("CAST1", 0.1f);
		}

	}

	else if( !strcmp( argv[0], "ifilatio" ) )
	{

		i=0;
		if( !ONCEANIMATIONPERIOD )
		{
			SkinMesh[0]->PassAnimation("CAST2", 0.1f);
			SkinMesh[0]->life += 40;
			if( SkinMesh[0]->life > 100 ) SkinMesh[0]->life = 100;
		}

	}

	else if( argc == 2 && !strcmp( argv[0], "vergan" ) && !strcmp( argv[1], "tat" ) )
	{

		i=0;
		if( !ONCEANIMATIONPERIOD )
		{
			SkinMesh[0]->PassAnimation("CAST3", 0.1f);
			
			
			for(int j=1; j < NumOfSkinMesh; j++)
			{
				SkinMesh[j]->life -= 40;
				SkinMesh[j]->bHurt = TRUE;
				if(SkinMesh[j]->life <= 0) SkinMesh[j]->bDead = TRUE;
			}
			
		}

	}

	HTMLLog("ChatCommand entered, argc %d, argv:", argc);
	for( i = 0; i < argc; i++ ) HTMLLog(" %s", argv[i]);
	HTMLLog("\n");

	return S_OK;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::ProcessMovement(CModel* cm)
{
	char* animName;
	CMyModel* CurSkinMesh = (CMyModel*)cm;

	//if passing - then no movement
	if( cm->ASI.ap_bIsAnimPass ) return S_OK;

	//if no animation - no movement
	if(cm->MType == BASE)
		animName = cm->GetTrackAnimationSetName(0);	
	else if(cm->MType == CLONE)
		animName = cm->ASI.AnimSetName;


	float cosy	=	float(SPEED * cos(CurSkinMesh->angle.y));
	float siny	=	float(SPEED * sin(CurSkinMesh->angle.y));	


	//simple movement
		 if( !strcmp( animName, "WALK" ) )
	{
		CurSkinMesh->speed.z = -cosy;
		CurSkinMesh->speed.x = -siny;
	}
	else if( !strcmp( animName, "RUN" ) || !strcmp( animName, "WPNRUN" ) /*|| !strcmp( animName, "StrikeRun" )*/ )
	{
		CurSkinMesh->speed.z = -2.5f * cosy;
		CurSkinMesh->speed.x = -2.5f * siny;
	}
	else if( !strcmp( animName, "BWD" ) )
	{
		CurSkinMesh->speed.z = 1.5f * cosy;
		CurSkinMesh->speed.x = 1.5f * siny;
	}

	else if( !strcmp( animName, "JUMP" ) || !strcmp( animName, "FLY" ) )
	{
		CurSkinMesh->speed.z = -2.0f * cosy;
		CurSkinMesh->speed.x = -2.0f * siny;
	}
	else if( !strcmp( animName, "SRFL" ) )
	{
		CurSkinMesh->speed.z = -1.4f * siny;
		CurSkinMesh->speed.x = 1.4f * cosy;
	}
	else if( !strcmp( animName, "SRFR" ) )
	{
		CurSkinMesh->speed.z = 1.4f * siny;
		CurSkinMesh->speed.x = -1.4f * cosy;
	}
	else if( !strcmp( animName, "WPNSRFLA" ) )
	{
		CurSkinMesh->speed.z = -1.4f * siny;
		CurSkinMesh->speed.x = 1.4f * cosy;
	}
	else if( !strcmp( animName, "WPNSRFRA" ) )
	{
		CurSkinMesh->speed.z = 1.4f * siny;
		CurSkinMesh->speed.x = -1.4f * cosy;
	}
	else if( !strcmp( animName, "WPNSRFLFA" ) )
	{
		CurSkinMesh->speed.z = 1.2f*(-siny) + 1.2f*(-cosy);
		CurSkinMesh->speed.x = 1.2f*( cosy) + 1.2f*(-siny);
	}
	else if( !strcmp( animName, "WPNSRFRFA" ) )
	{
		CurSkinMesh->speed.z = 1.2f*( siny) + 1.2f*(-cosy);
		CurSkinMesh->speed.x = 1.2f*(-cosy) + 1.2f*(-siny);
	}


	//dodges
	else if( !strcmp( animName, "DODGE" ) )
	{
		CurSkinMesh->speed.z = 1.5f * cosy;
		CurSkinMesh->speed.x = 1.5f * siny;
	}
	

	//magic
	/*
	else if( !strcmp( animName, "CAST1" ) )
	{
		CurSkinMesh->speed.z = -0.2f*cosy;
		CurSkinMesh->speed.x = -0.2f*siny;
	}
	*/


	//hurts
	else if( !strcmp( animName, "HURTLITTLE" ) )
	{
		CurSkinMesh->speed.z = 0.1f*cosy;
		CurSkinMesh->speed.x = 0.1f*siny;
	}
	else if( !strcmp( animName, "HURTMUCH" ) )
	{
		CurSkinMesh->speed.z = 0.1f*cosy;
		CurSkinMesh->speed.x = 0.1f*siny;
	}

	//attack fails
	else if( !strcmp( animName, "STRIKELFAIL" ) )
	{
		CurSkinMesh->speed.z = 0.6f*cosy;
		CurSkinMesh->speed.x = 0.6f*siny;
	}
	else if( !strcmp( animName, "STRIKERFAIL" ) )
	{
		CurSkinMesh->speed.z = 0.6f*cosy;
		CurSkinMesh->speed.x = 0.6f*siny;
	}
	else if( !strcmp( animName, "STRIKEUFAIL" ) )
	{
		CurSkinMesh->speed.z = 0.6f*cosy;
		CurSkinMesh->speed.x = 0.6f*siny;
	}
	else if( !strcmp( animName, "STRIKEDFAIL" ) )
	{
		CurSkinMesh->speed.z = 0.6f*cosy;
		CurSkinMesh->speed.x = 0.6f*siny;
	}


	//strikes	
	else if( !strcmp( animName, "STRIKEUA" ) )
	{
		CurSkinMesh->speed.z = -cosy * 0.6f;
		CurSkinMesh->speed.x = -siny * 0.6f;
	}
	else if( !strcmp( animName, "STRIKEDA" ) )
	{
		CurSkinMesh->speed.z = cosy * 0.6f;
		CurSkinMesh->speed.x = siny * 0.6f;
	}
	else if( !strcmp( animName, "STRIKELA" ) )
	{
		CurSkinMesh->speed.z = -cosy * 0.8f;
		CurSkinMesh->speed.x = -siny * 0.8f;
	}
	else if( !strcmp( animName, "STRIKERA" ) )
	{
		CurSkinMesh->speed.z = -cosy * 0.8f;
		CurSkinMesh->speed.x = -siny * 0.8f;
	}



	//zero
	else
	{
		CurSkinMesh->speed.z = 0;
		CurSkinMesh->speed.x = 0;
	}


	return S_OK;
}



//-----------------------------------------------------------------------------
// Name:
// Desc: sample of incoming string:
//		 "0 data/models/humanbody01.x data/models/head01.x (500,500,500)"
//-----------------------------------------------------------------------------

HRESULT CMyD3DApplication::AddActor(int inAnimType, char* inMeshPath, char* inHeadMeshPath, D3DXVECTOR3 inCoord)
{
	int i,j;
	WCHAR wstring[512];

	//stop timer!!!
	DXUtil_Timer( TIMER_STOP );

	NumOfSkinMesh++;
	//*SkinMesh.resize(NumOfSkinMesh);
	//*Heads.resize(NumOfSkinMesh);


	HTMLLog("\n\t\t--Adding additional actor: %d\n", NumOfSkinMesh);

		i = NumOfSkinMesh-1;
		SkinMesh.push_back( NULL );
		SkinMesh[i] = new CMyModel();
		Heads.push_back( NULL );
		Heads[i] = new CModel();
		//*SkinMesh[NumOfSkinMesh-1] = CMyModel();
		//*Heads[NumOfSkinMesh-1] = CModel();
		
		
		SkinMesh[i]->id = i;
		Heads[i]->id = i;

		//defining type of mesh (maybe no need to initialize)
		SkinMesh[i]->MAnimType = inAnimType;

		//for(j=0; j < i; j++) if(SkinMesh[i]->MAnimType == SkinMesh[j]->MAnimType) break;
		//if(j==i) SkinMesh[i]->MType = BASE;
		//else
		//{
		//	SkinMesh[i]->MType = CLONE;
		//	SkinMesh[i]->ParentModel = &(*SkinMesh[j]);
		//}
		SkinMesh[i]->MType = CLONE;
		SkinMesh[i]->ParentModel = Animations[ SkinMesh[i]->MAnimType ];

		//check if skin mesh already exists
		SkinMesh[i]->meshPath = new char[256];
		strcpy(SkinMesh[i]->meshPath, inMeshPath);
		for(j=0; j<i; j++) if( !stricmp(SkinMesh[i]->meshPath, SkinMesh[j]->meshPath) ) break;
		if(i==j) SkinMesh[i]->dublicate = FALSE;
		else SkinMesh[i]->dublicate = TRUE;

		Heads[i]->meshPath = new char[256];
		strcpy(Heads[i]->meshPath, inHeadMeshPath);
		for(j=0; j<i; j++) if( !stricmp(Heads[i]->meshPath, Heads[j]->meshPath) ) break;
		if(i==j) Heads[i]->dublicate = FALSE;
		else Heads[i]->dublicate = TRUE;


		MultiByteToWideChar( CP_ACP, 0, SkinMesh[i]->meshPath, -1, wstring, 512 );
		SkinMesh[i]->Init(m_pd3dDevice, m_d3dCaps, wstring, m_pEffect, SkinMesh[j]);

		MultiByteToWideChar( CP_ACP, 0, Heads[i]->meshPath, -1, wstring, 512 );
		Heads[i]->Init(m_pd3dDevice, m_d3dCaps, wstring, m_pEffect, Heads[j]);

		SkinMesh[i]->speed = D3DXVECTOR3(0,0,0);
		SkinMesh[i]->SetCoordinate( inCoord );
		SkinMesh[i]->oldCoord = SkinMesh[i]->coordinate;
		SkinMesh[i]->SetAngle( D3DXVECTOR3(0,0,0) );

		SkinMesh[i]->InitPhysX(gPhysicsSDK, gScene, gManager, OT_CHARACTER);


	HTMLLog("\t\t--actor added successfully\n");
	
	DXUtil_Timer( TIMER_START );

	return S_OK;
}



//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::DInputCallback()
{

	int i = 0; //for ONCEANIMATIONPERIOD macros


	//initialization
	if(input == NULL) return E_FAIL;

	input->pKeyboard->GetDeviceState( sizeof(input->keyboard), (LPVOID)&input->keyboard );
	input->pMouse->GetDeviceState( sizeof(DIMOUSESTATE), (LPVOID)&input->mouse );

	
	
	//rotation of character
	if( SkinMesh[0]->bDead == FALSE && !SkinMesh[0]->IsAttack())
	{
		if( SkinMesh[0]->bLocked == FALSE )
			SkinMesh[0]->angle.y += input->mouse.lX * ANG *m_fElapsedTime;
		else
		{
			SkinMesh[0]->AIFace( SkinMesh[ SkinMesh[0]->lockTarget ]->coordinate, m_fElapsedTime );
		}
	}



	// setting keyboard bools
	BOOL bReturn;
	if( KEYDOWN( input->keyboard, DIK_RETURN) && timeGetTime() - EnterTime > 200.f )
	{
		bReturn = TRUE;
		EnterTime = timeGetTime();
	}
	else bReturn = FALSE;

	BOOL bShift;
	if( KEYDOWN( input->keyboard, DIK_LSHIFT) )
		bShift = TRUE;
	else bShift = FALSE;

	BOOL bLCtrl;
	if( KEYDOWN( input->keyboard, DIK_LCONTROL) )
		bLCtrl = TRUE;
	else bLCtrl = FALSE;

	BOOL bRMB;
	if( MOUSEBUTTON( input->mouse, RIGHT_BUTTON) )
		bRMB = TRUE;
	else bRMB = FALSE;

	BOOL bLMB;
	if( MOUSEBUTTON( input->mouse, LEFT_BUTTON) )
		bLMB = TRUE;
	else bLMB = FALSE;

	BOOL bSpace;
	if( KEYDOWN( input->keyboard, DIK_SPACE) )
		bSpace = TRUE;
	else bSpace = FALSE;





	// MENU processing
	if(bMenu)
	{
		POINT curpos;
		GetCursorPos(&curpos);

		if( curpos.x < 0.7f*SCREENWIDTH && curpos.x > 0.6f*SCREENWIDTH ) selection =  (curpos.y - SCREENHEIGHT*2/3)/40;
		else selection = -255;


		if( MOUSEBUTTON( input->mouse, LEFT_BUTTON) )
			bMenuLMB = TRUE;
		else
		{
			if( bMenuLMB == TRUE )	MenuCallback();	
			bMenuLMB = FALSE;
		}
		
	}


	// natural reactions - unavoidable block
	// when character cannot move himself
	// 1. falldown & dead
	// 2. jump (in the air)
	// 3. hurt
	else if(SkinMesh[0]->bDead == TRUE)
	{
		//char* AnimSetName = SkinMesh[0]->GetTrackAnimationSetName(0);
		char* AnimSetName = SkinMesh[0]->ASI.AnimSetName;


		if( !strcmp(AnimSetName, "FALLDOWN") )
		{
			SkinMesh[0]->SetStockAnimationSetByName("DEAD", 0.1f);
		}
		else
		{
			SkinMesh[0]->PassAnimation("DEAD", 0.1f);
			SkinMesh[0]->SetStockAnimationSetByName("DEAD", 0.1f);
		}
	}

	else if( SkinMesh[0]->fFlyTime >= 0.8f ) //calibrated value
	{
		SkinMesh[0]->PassAnimation("FLY", 0.1f);
	}

	else if(SkinMesh[0]->bHurt == TRUE)
	{
		if( !ONCEANIMATIONHURTPERIOD ) // if animation is not hurt or period passed
									   // means that hurt animation will be applied when current animation is not hurt
		{

			i = rand()%3;
			if(i==0)
			{
				SkinMesh[0]->PassAnimation("HURTMUCH", 0.2f);
			}
			else if(i==1)
			{
				SkinMesh[0]->PassAnimation("HURTLITTLE", 0.2f);
			}
			else if(i==2)
			{
				SkinMesh[0]->PassAnimation("FALLDOWNGETUP", 0.2f);
			}

			if( SkinMesh[0]->bIsWpnDrawned ) SkinMesh[0]->SetStockAnimationSetByName("WPNSTANDA", 0.1f);
			else SkinMesh[0]->SetStockAnimationSetByName("STAND", 0.1f);

		}
		SkinMesh[0]->bHurt = FALSE;
	}

	else if( SkinMesh[0]->bFail == TRUE)
	{
		//fails
		if( SkinMesh[0]->IsAttack() == ATTACKLEFT )
		{
			SkinMesh[0]->PassAnimation("STRIKELFAIL", 0.1f);
			SkinMesh[0]->SetStockAnimationSetByName("WPNSTANDA", 0.1f);
		}
		else if( SkinMesh[0]->IsAttack() == ATTACKRIGHT )
		{
			SkinMesh[0]->PassAnimation("STRIKERFAIL", 0.1f);
			SkinMesh[0]->SetStockAnimationSetByName("WPNSTANDA", 0.1f);
		}
		else if( SkinMesh[0]->IsAttack() == ATTACKUP )
		{
			SkinMesh[0]->PassAnimation("STRIKEUFAIL", 0.1f);
			SkinMesh[0]->SetStockAnimationSetByName("WPNSTANDA", 0.1f);
		}
		else if( SkinMesh[0]->IsAttack() == ATTACKDOWN )
		{
			SkinMesh[0]->PassAnimation("STRIKEDFAIL", 0.1f);
			SkinMesh[0]->SetStockAnimationSetByName("WPNSTANDA", 0.1f);
		}

		SkinMesh[0]->bFail = FALSE;
	}


	//CONSOLE && CHAT
	else if( bConsole || bChat )
	{	

		// ROTATION OR DOING NOTHING
		//---

		if( !ONCEANIMATIONPERIOD )
		{

			//rotation
			if( input->mouse.lX * ANG *m_fElapsedTime > ANGANIM )
				SkinMesh[0]->PassAnimation("ROTR", 0.1f);
			else if ( input->mouse.lX * ANG *m_fElapsedTime < -ANGANIM )
				SkinMesh[0]->PassAnimation("ROTL", 0.1f);

			//no rotation
			else
			{
				if(SkinMesh[0]->OnceAnimation() != TRUE)
				{
					if( SkinMesh[0]->bIsWpnDrawned )
					{
						SkinMesh[0]->SetStockAnimationSetByName("WPNSTANDA", 0.1f);
					}
					else
					{
						SkinMesh[0]->SetStockAnimationSetByName("STAND", 0.1f);
					}
				}

				//!!pass animation stock
				SkinMesh[0]->PassAnimationStock();
			}

		}

		//---


	}



	//--------------------------------
	// User controls processing
	// list of actions:
	// 
	// 1. draw/hide weapon
	// 2. jump
	// 3. attack/block
	// 4. dodge
	// 5. movement - walk/run/strafe
	// 6. cast spells
	// 7. rotate
	//--------------------------------
	else
	{
		//special attachment stuff
		if( !ONCEANIMATIONPERIOD )
		{
			//getting name
			//char* AnimSetName = SkinMesh[0]->GetTrackAnimationSetName(0);
			char* AnimSetName = SkinMesh[0]->ASI.AnimSetName;


			//attachment
			if( !strcmp(AnimSetName, "WPNDRAW1H") )
			{
				SkinMesh[0]->Attach(Weapons[0], "Bip01_R_Hand");
				SkinMesh[0]->bIsWpnDrawned = TRUE;

				m_pMusicManager->Create3DSegmentFromFile( &SkinMesh[0]->m_pSound, L"data/sound/wpndraw.wav", TRUE, FALSE, NULL);
				if( SkinMesh[0]->m_pSound ) SkinMesh[0]->m_pSound->Play();
			}
			else if ( !strcmp(AnimSetName, "WPNHIDE1H") )
			{
				SkinMesh[0]->Attach(Weapons[0], "Bip01_Scubbard1");
				SkinMesh[0]->bIsWpnDrawned = FALSE;


				m_pMusicManager->Create3DSegmentFromFile( &SkinMesh[0]->m_pSound, L"data/sound/wpnhide.wav", TRUE, FALSE, NULL);
				if( SkinMesh[0]->m_pSound ) SkinMesh[0]->m_pSound->Play();
			}
		}



	// KEYBOARD & MOUSE
	//------

		BOOL bAnykeyDown = FALSE;



		// JUMP
		//--------------
		if( bSpace && !ONCEANIMATIONPERIOD && !SkinMesh[0]->bIsWpnDrawned && SkinMesh[0]->bVerticalCollisionUP )
		{
			SkinMesh[0]->PassAnimation("JUMP", 0.05f);

			SkinMesh[0]->speed.y = 500.f;

			bAnykeyDown = TRUE;
		}

		


		//attacks
		//-------
		
		if( (bLCtrl == TRUE || bLMB == TRUE) && !ONCEANIMATIONNOSTRIKEPERIOD && SkinMesh[0]->bIsWpnDrawned == TRUE)
		{
		

			//getting name
			//char* AnimSetName = SkinMesh[0]->GetTrackAnimationSetName(0);
			char* AnimSetName = SkinMesh[0]->ASI.AnimSetName;

			if( KEYDOWN( input->keyboard, DIK_W) )
			{
				if(  !strcmp(AnimSetName, "WPNSTANDA") ||
					 //!strcmp(AnimSetName, "STRIKELSTA") ||
					 !strcmp(AnimSetName, "STRIKERSTA") ||
					 //!strcmp(AnimSetName, "STRIKEDSTA") ||
					 !strcmp(AnimSetName, "STRIKEUSTA")
				  )
				{
					SkinMesh[0]->PassAnimation("STRIKEDA", 0.3f);
					SkinMesh[0]->SetStockAnimationSetByName("STRIKEDSTA", 0.1f);

					bAnykeyDown = TRUE;
				}
			}

			else if( KEYDOWN( input->keyboard, DIK_S) )
			{
				if(  !strcmp(AnimSetName, "WPNSTANDA") ||
					 //!strcmp(AnimSetName, "STRIKELSTA") ||
					 !strcmp(AnimSetName, "STRIKERSTA") ||
					 !strcmp(AnimSetName, "STRIKEDSTA")
					 //!strcmp(AnimSetName, "STRIKEUSTA")
				  )
				{
					SkinMesh[0]->PassAnimation("STRIKEUA", 0.3f);
					SkinMesh[0]->SetStockAnimationSetByName("STRIKEUSTA", 0.1f);

					bAnykeyDown = TRUE;
				}
			}

			else if( KEYDOWN( input->keyboard, DIK_A) )
			{
				if(  !strcmp(AnimSetName, "WPNSTANDA") ||
					 !strcmp(AnimSetName, "STRIKELSTA") ||
					 //!strcmp(AnimSetName, "STRIKERSTA") ||
					 !strcmp(AnimSetName, "STRIKEDSTA") 
					 //!strcmp(AnimSetName, "STRIKEUSTA")
				  )
				{
					SkinMesh[0]->PassAnimation("STRIKERA", 0.15f);
					SkinMesh[0]->SetStockAnimationSetByName("STRIKERSTA", 0.1f);

					bAnykeyDown = TRUE;
				}
			}

			else if( KEYDOWN( input->keyboard, DIK_D) )
			{
				if(  !strcmp(AnimSetName, "WPNSTANDA") ||
					 //!strcmp(AnimSetName, "STRIKELSTA") ||
					 !strcmp(AnimSetName, "STRIKERSTA") 
					 //!strcmp(AnimSetName, "STRIKEDSTA") ||
					 //!strcmp(AnimSetName, "STRIKEUSTA")
				  )
				{
					SkinMesh[0]->PassAnimation("STRIKELA", 0.2f);
					SkinMesh[0]->SetStockAnimationSetByName("STRIKELSTA", 0.1f);

					bAnykeyDown = TRUE;
				}
			}

			
		}


		
		//blocks
		//----------
		else if( bRMB && !ONCEANIMATIONNOSTRIKEPERIOD && SkinMesh[0]->bIsWpnDrawned == TRUE )
		{
		
			if( KEYDOWN( input->keyboard, DIK_W) )
			{
				SkinMesh[0]->PassAnimation("BLOCKUA", 0.05f);
				SkinMesh[0]->SetStockAnimationSetByName("WPNSTANDA", 0.1f);

				bAnykeyDown = TRUE;				
			}
			else if( KEYDOWN( input->keyboard, DIK_S) )
			{
				SkinMesh[0]->PassAnimation("BLOCKDA", 0.05f);
				SkinMesh[0]->SetStockAnimationSetByName("WPNSTANDA", 0.1f);

				bAnykeyDown = TRUE;
			}
			else if( KEYDOWN( input->keyboard, DIK_D) )
			{
				SkinMesh[0]->PassAnimation("BLOCKRA", 0.05f);
				SkinMesh[0]->SetStockAnimationSetByName("WPNSTANDA", 0.1f);

				bAnykeyDown = TRUE;
			}
			else if( KEYDOWN( input->keyboard, DIK_A) )
			{
				SkinMesh[0]->PassAnimation("BLOCKLA", 0.05f);
				SkinMesh[0]->SetStockAnimationSetByName("WPNSTANDA", 0.1f);

				bAnykeyDown = TRUE;
			}
		
		}	
		
		


		//movement
		//-------
		else if( !ONCEANIMATIONPERIOD )
		{
			if( KEYDOWN( input->keyboard, DIK_W) )
			{
				if( bShift )	SkinMesh[0]->PassAnimation("WALK", 0.2f);

				else 
				{
					if( SkinMesh[0]->bIsWpnDrawned )
					{
						if( KEYDOWN( input->keyboard, DIK_A) )
						{
							SkinMesh[0]->PassAnimation("WPNSRFLFA", 0.02f);
						}

						else if( KEYDOWN( input->keyboard, DIK_D) )
						{
							SkinMesh[0]->PassAnimation("WPNSRFRFA", 0.02f);
						}

						else SkinMesh[0]->PassAnimation("WPNRUN", 0.2f);
					}

					else	SkinMesh[0]->PassAnimation("RUN", 0.2f);
				}

				bAnykeyDown = TRUE;
			}

			else if( KEYDOWN( input->keyboard, DIK_S) )
			{
				if( SkinMesh[0]->bIsWpnDrawned )
				{
					SkinMesh[0]->PassAnimation("DODGE", 0.1f);
				}
				else
				{
					SkinMesh[0]->PassAnimation("BWD", 0.2f);
				}

				bAnykeyDown = TRUE;
			}

			else if( KEYDOWN( input->keyboard, DIK_A) )
			{

				if( SkinMesh[0]->bIsWpnDrawned ) SkinMesh[0]->PassAnimation("WPNSRFLA", 0.02f);
				else SkinMesh[0]->PassAnimation("SRFL", 0.2f);

				bAnykeyDown = TRUE;
			}

			else if( KEYDOWN( input->keyboard, DIK_D) )
			{
				if( SkinMesh[0]->bIsWpnDrawned ) SkinMesh[0]->PassAnimation("WPNSRFRA", 0.02f);
				else SkinMesh[0]->PassAnimation("SRFR", 0.2f);

				bAnykeyDown = TRUE;
			}
		}




		//miscellanious



		//spells
		/*
		// in future - depends on current spell
		if( KEYDOWN( input->keyboard, DIK_C) && !bLMB && !bLCtrl && !ONCEANIMATIONPERIOD )
		{
			SkinMesh[0]->PassAnimation("CAST1", 0.1f);
			bAnykeyDown = TRUE;
		}
		*/


		// wpndraw
		if( KEYDOWN( input->keyboard, DIK_F) && !ONCEANIMATIONPERIOD )
		{
			if(SkinMesh[0]->bIsWpnDrawned == FALSE)
			{
				SkinMesh[0]->PassAnimation("WPNDRAW1H", 0.2f);
				//SkinMesh[0]->bIsWpnDrawned = TRUE;
			}
			else
			{
				SkinMesh[0]->PassAnimation("WPNHIDE1H", 0.2f);
				//SkinMesh[0]->bIsWpnDrawned = FALSE;
			}

			bAnykeyDown = TRUE;
		}



		// lock or dislock
		if( KEYDOWN( input->keyboard, DIK_TAB) && !ONCEANIMATIONPERIOD &&
			(timeGetTime() - lockTime) > 200.f )
		{
			if(SkinMesh[0]->bLocked == FALSE)
			{
				lockTime = timeGetTime();
				SkinMesh[0]->bLocked = TRUE;
				SkinMesh[0]->lockTarget = 3;
			}
			else
			{
				lockTime = timeGetTime();
				SkinMesh[0]->bLocked = FALSE;
			}

			bAnykeyDown = TRUE;
		}



		// ROTATION OR DOING NOTHING
		//---

		if( !bAnykeyDown && !ONCEANIMATIONPERIOD )
		{

			//rotation
			if( input->mouse.lX * ANG *m_fElapsedTime > ANGANIM )
				SkinMesh[0]->PassAnimation("ROTR", 0.1f);
			else if ( input->mouse.lX * ANG *m_fElapsedTime < -ANGANIM )
				SkinMesh[0]->PassAnimation("ROTL", 0.1f);

			//no rotation
			else
			{
				if(SkinMesh[0]->OnceAnimation() != TRUE)
				{
					if( SkinMesh[0]->bIsWpnDrawned )
					{
						SkinMesh[0]->SetStockAnimationSetByName("WPNSTANDA", 0.1f);
					}
					else
					{
						SkinMesh[0]->SetStockAnimationSetByName("STAND", 0.1f);
					}
				}

				//!!pass animation stock
				SkinMesh[0]->PassAnimationStock();
			}

		}

		//---

	}
	//------
	//------





	//----
	// out of categories	
	

	/*
	if( KEYDOWN( input->keyboard, DIK_M) )
	{
		D3DXSaveTextureToFile("test.jpg", D3DXIFF_JPG, water->m_pEnvTex, NULL);
	}
	*/
		

	// music
	/*
	if( KEYDOWN( input->keyboard, DIK_R) )
	{
		m_pMusicManager->CreateSegmentFromFile( &m_pSound, "data/sound/roar.wav", TRUE, FALSE);
		if( m_pSound ) m_pSound->Play();
	}
	*/


	//test
	//if( KEYDOWN( input->keyboard, DIK_T) )
	//{
	//	SkinMesh[0]->CurrentArmor = 0;
	//}


	//change of view
	if( KEYDOWN( input->keyboard, DIK_F1) )
	{
		if(camera->mode == FIRSTPERSONVIEW) camera->mode = THIRDPERSONVIEW; else camera->mode = FIRSTPERSONVIEW;
	}

	// console
	if( KEYDOWN( input->keyboard, DIK_GRAVE) && !bChat )
	{
		if( timeGetTime() - consoleTime > 200.f )
		{
			if( bConsole ) bConsole = FALSE;
			else		   bConsole = TRUE;

			consoleTime = timeGetTime();
		}
	}


	// enter processing
	if( bReturn )
	{
		//console
		if( bConsole )
		{
			AddMessageToConsole(consoleStr);
			ProcessConsoleMsg();

			//renewing
			consoleSymNum = 0;
			consoleStr[0] = '\0';
		}

		//chat
		else if( timeGetTime() - chatTime > 200.f )
		{
			if( bChat )
			{
				AddMessageToChat(chatStr);
				ProcessChatMsg();


				bChat = FALSE;
				chatSymNum = 0;
				chatStr[0] = '\0';
			}
			else	bChat = TRUE;

			chatTime = timeGetTime();
		}

	}

/*	
	if( bConsole && bChat==FALSE )
	{	
		if( bReturn )
		{
			AddMessageToConsole(consoleStr);


			ProcessConsoleMsg();


			consoleSymNum = 0;
			consoleStr[0] = '\0';
			EnterTime = timeGetTime();
		}
	}
	

	//chat
	if( bReturn && !bConsole )
	{
		if( timeGetTime() - chatTime > 200.f )
		{
			if( bChat )
			{
				AddMessageToChat(chatStr);

				ProcessChatMsg();


				bChat = FALSE;
				chatSymNum = 0;
				chatStr[0] = '\0';
				EnterTime = timeGetTime();

			}
			else	bChat = TRUE;

			chatTime = timeGetTime();
		}
	}
*/
	/*
	if( KEYDOWN( input->keyboard, DIK_M) )
	{
		AddActor( 0, "data/models/humanbody01.x", "data/models/head01.x", D3DXVECTOR3(500,500,500) );
	}
	*/

	//menu
	if( KEYDOWN( input->keyboard, DIK_ESCAPE) )
	{
		if( timeGetTime() - menuTime > 200.f )
		{
			if(bMenu == FALSE) { DXUtil_Timer( TIMER_STOP ); bMenu = TRUE;}
			else if(bMenu == TRUE) { DXUtil_Timer( TIMER_START ); bMenu = FALSE;}
			
			menuTime = timeGetTime();
		}
	}




	//----
	//----



	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: AICallback
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::AICallback(CModel* cm)
{


	int i = cm->id;
	int j;
	CMyModel* CurSkinMesh = (CMyModel*)cm;


	// natural reactions - unavoidable block
	// when character cannot move himself
	if(CurSkinMesh->bDead == TRUE)
	{
		char* AnimSetName;
		if(CurSkinMesh->MType == BASE) AnimSetName = CurSkinMesh->GetTrackAnimationSetName(0);
		else if(CurSkinMesh->MType == CLONE) AnimSetName = CurSkinMesh->ASI.AnimSetName;

		if( !strcmp(AnimSetName, "FALLDOWN") )
		{
			CurSkinMesh->SetStockAnimationSetByName("DEAD", 0.1f);
		}
		else
		{
			CurSkinMesh->PassAnimation("DEAD", 0.1f);
			CurSkinMesh->SetStockAnimationSetByName("DEAD", 0.1f);
		}
	}
	else if(CurSkinMesh->bHurt == TRUE)
	{
		if( !ONCEANIMATIONHURTPERIOD )
		{
			j = rand()%3;
			if(j==0)
			{
				CurSkinMesh->PassAnimation("HURTMUCH", 0.2f);
			}
			else if(j==1)
			{
				CurSkinMesh->PassAnimation("HURTLITTLE", 0.2f);
			}
			else if(j==2)
			{
				CurSkinMesh->PassAnimation("FALLDOWNGETUP", 0.2f);
			}

			
			if( CurSkinMesh->bIsWpnDrawned ) CurSkinMesh->SetStockAnimationSetByName("WPNSTANDA", 0.1f);
			else CurSkinMesh->SetStockAnimationSetByName("STAND", 0.1f);

		}

		CurSkinMesh->bHurt = FALSE;
	}
	
	else if( CurSkinMesh->bFail == TRUE)
	{
		//fails
		if( CurSkinMesh->IsAttack() == ATTACKLEFT )
		{
			CurSkinMesh->PassAnimation("STRIKELFAIL", 0.1f);
			CurSkinMesh->SetStockAnimationSetByName("WPNSTANDA", 0.1f);
		}
		else if( CurSkinMesh->IsAttack() == ATTACKRIGHT )
		{
			CurSkinMesh->PassAnimation("STRIKERFAIL", 0.1f);
			CurSkinMesh->SetStockAnimationSetByName("WPNSTANDA", 0.1f);
		}
		else if( CurSkinMesh->IsAttack() == ATTACKUP )
		{
			CurSkinMesh->PassAnimation("STRIKEUFAIL", 0.1f);
			CurSkinMesh->SetStockAnimationSetByName("WPNSTANDA", 0.1f);
		}
		else if( CurSkinMesh->IsAttack() == ATTACKDOWN )
		{
			CurSkinMesh->PassAnimation("STRIKEDFAIL", 0.1f);
			CurSkinMesh->SetStockAnimationSetByName("WPNSTANDA", 0.1f);
		}

		CurSkinMesh->bFail = FALSE;
	}




	//---------------------------------------
	//			AI block
	//---------------------------------------

	else
	{


		//----------------------------------------
		//            ---
		//			   |
		//			   |
		//			   |
		//            ---
		//----------------------------------------
		//first one - runner & follower
		if(cm->id == 1)
		{

			CurSkinMesh->AIFace( SkinMesh[0]->coordinate, ANG*m_fElapsedTime );


			if( !ONCEANIMATIONPERIOD )
			{

				if(D3DXVec3Length( &(SkinMesh[0]->coordinate - CurSkinMesh->coordinate) ) > 120.f)
				{
					CurSkinMesh->PassAnimation("WALK", 0.2f);
				}
				else
				{
					j = rand()%2;
					if(j == 0)
					{
						CurSkinMesh->PassAnimation("TALK1", 0.2f);


						m_pMusicManager->Create3DSegmentFromFile( &CurSkinMesh->m_pSound, L"data/sound/getoutofhere.wav", TRUE, FALSE, NULL);
						CurSkinMesh->m_pSound->Play();
					}
					else if(j == 1)
					{
						CurSkinMesh->PassAnimation("TALK2", 0.2f);
					

						m_pMusicManager->Create3DSegmentFromFile( &CurSkinMesh->m_pSound, L"data/sound/itsnotmyproblems.wav", TRUE, FALSE, NULL);
						CurSkinMesh->m_pSound->Play();
					}
				}
			}
		}

		//----------------------------------------




		//----------------------------------------
		//            -----
		//			   | |
		//			   | |
		//			   | |
		//            -----
		//----------------------------------------
		//second - walker
		else if(cm->id == 2)
		{


			if( !ONCEANIMATIONPERIOD )
			{
				CurSkinMesh->PassAnimation("WALK", 0.2f);
				CurSkinMesh->angle.y += ANG * m_fElapsedTime;
			}


		}
		
		//----------------------------------------




		//----------------------------------------
		//            -------
		//			   | | |
		//			   | | |
		//			   | | |
		//            -------
		//----------------------------------------
		//third - attacker

		else if(cm->id == 3)
		{
			BOOL bAnykeyDown = FALSE;


			if(!CurSkinMesh->IsAttack())
				CurSkinMesh->AIFace( SkinMesh[0]->coordinate, ANG*m_fElapsedTime );			


			if( !ONCEANIMATIONPERIOD && CurSkinMesh->bIsWpnDrawned == TRUE )
			{

				float distance = D3DXVec3Length( &(SkinMesh[0]->coordinate - CurSkinMesh->coordinate) );
				

				
				// Defensive state
					if( CurSkinMesh->life < 50 && SkinMesh[0]->bDead == FALSE )
				{

						if( distance > 400.f ) CurSkinMesh->PassAnimation("WPNGRACEA", 0.2f);
						
						else
						{
							//block
							if( SkinMesh[0]->IsAttack() && !ONCEANIMATIONNOSTRIKEPERIOD && SkinMesh[i]->bIsWpnDrawned == TRUE)
							{
								CurSkinMesh->AIBlock( SkinMesh[0]->IsAttack() );
							}

							//or attack
							else if( distance < 150.f &&
									 (!ONCEANIMATIONNOSTRIKEPERIOD && SkinMesh[i]->bIsWpnDrawned == TRUE) )
							{
								bAnykeyDown = CurSkinMesh->AIRandomAttack();
							}
						}

				}


				//follow
				else if( distance > 160.f )
				{
					CurSkinMesh->PassAnimation("RUN", 0.2f);

					bAnykeyDown = TRUE;
				}

				else if( distance < 110.f )
				{

					//block
					if( SkinMesh[0]->IsAttack() && !ONCEANIMATIONNOSTRIKEPERIOD && SkinMesh[i]->bIsWpnDrawned == TRUE)
					{
						CurSkinMesh->AIBlock( SkinMesh[0]->IsAttack() );
					}

					else
					{
						CurSkinMesh->PassAnimation("DODGE", 0.2f);
					}

					bAnykeyDown = TRUE;
				}

				//attack
				else if( !ONCEANIMATIONNOSTRIKEPERIOD && SkinMesh[i]->bIsWpnDrawned == TRUE )
				{
					bAnykeyDown = CurSkinMesh->AIRandomAttack();
				}

			}

			if( bAnykeyDown == FALSE && !ONCEANIMATIONPERIOD )
			{
				if(CurSkinMesh->OnceAnimation() != TRUE)
				{
					if( CurSkinMesh->bIsWpnDrawned ) CurSkinMesh->SetStockAnimationSetByName("WPNSTANDA", 0.1f);
					else CurSkinMesh->SetStockAnimationSetByName("STAND", 0.1f);
				}

					//!!pass animation stock
				CurSkinMesh->PassAnimationStock();
			}


		}

		//----------------------------------------




		//----------------------------------------
		//            -------
		//			   | |  |
		//			   | |  |
		//			   |  \/
		//            -------
		//----------------------------------------
		//fourth - just standing and casting

		else
		{
			if( !ONCEANIMATIONPERIOD )
			{
				int n = rand()%5;
				
				if(n==0)
					CurSkinMesh->PassAnimation("CAST1", 0.2f);
				else if(n==1)
					CurSkinMesh->PassAnimation("CAST2", 0.2f);
				else if(n==2)
					CurSkinMesh->PassAnimation("CAST3", 0.2f);
				else if(n==3)
					CurSkinMesh->PassAnimation("TALK1", 0.2f);
				else if(n==4)
					CurSkinMesh->PassAnimation("TALK2", 0.2f);
			}

		}

		//----------------------------------------



	}	//end of AI block



	return S_OK;
}



//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::AddDecal( CollisionModel* cm, CTerrain* terr )
{


	int NumOfDecvert = 6;
	DECALVERTEX* decvert = new DECALVERTEX[NumOfDecvert];

	D3DXVECTOR3 CMCenter = cm->vMin + (cm->vMax - cm->vMin)*0.5f;

	D3DXVECTOR2 N = terr->GetNByXZ(CMCenter.x, CMCenter.z);
	CUSTOMVERTEX* Vert = terr->GetQuadByN( N );

	D3DXVECTOR3 normal = terr->GetNormalByXZ(CMCenter.x, CMCenter.z);

	D3DXVECTOR3 u1, d1; //terrain
	D3DXVECTOR3 u2, d2; //bbox
	D3DXVECTOR3 u3, d3;	//result

	u1 = D3DXVECTOR3( MAX(Vert[0].x, Vert[3].x), MAX(Vert[0].y, Vert[3].y), MAX(Vert[0].z, Vert[3].z) );
	d1 = D3DXVECTOR3( MIN(Vert[0].x, Vert[3].x), MIN(Vert[0].y, Vert[3].y), MIN(Vert[0].z, Vert[3].z) );

	u2 = cm->vMax;
	d2 = cm->vMin;


	u3 = D3DXVECTOR3(0,0,0);
	d3 = D3DXVECTOR3(0,0,0);


	// x - axis
	if(u1.x < u2.x && u1.x > d2.x)
	{
		if( d1.x < d2.x ) { u3.x = u1.x; d3.x = d2.x; }
		else { u3.x = u1.x; d3.x = d1.x; }
	}
	else if(d1.x < u2.x && d1.x > d2.x)
	{
		if( u1.x > u2.x ) { u3.x = u2.x, d3.x = d1.x; }
		else { u3.x = u1.x; d3.x = d1.x; }
	}
	else if( u1.x > u2.x && d1.x < d2.x) { d3.x = d2.x, u3.x = u2.x; }


	// z - axis
	if(u1.z < u2.z && u1.z > d2.z)
	{
		if( d1.z < d2.z ) { u3.z = u1.z; d3.z = d2.z; }
		else { u3.z = u1.z; d3.z = d1.z; }
	}
	else if(d1.z < u2.z && d1.z > d2.z)
	{
		if( u1.z > u2.z ) { u3.z = u2.z, d3.z = d1.z; }
		else { u3.z = u1.z; d3.z = d1.z; }
	}
	else if( u1.z > u2.z && d1.z < d2.z) { d3.z = d2.z, u3.z = u2.z; }


	//texcoords
	D3DXVECTOR3 uT, dT;
	dT.x = (d3.x - d2.x)/(u2.x - d2.x);
	uT.x = 1.f - (u2.x - u3.x)/(u2.x - d2.x);
	dT.z = (d3.z - d2.z)/(u2.z - d2.z);
	uT.z = 1.f - (u2.z - u3.z)/(u2.z - d2.z);

	d3 += normal*0.1f;
	u3 += normal*0.1f;

	decvert[0] = DECALVERTEX(d3.x, terr->GetYByQuadAndXZ(d3.x ,d3.z,Vert)+normal.y*0.1f, d3.z, normal.x, normal.y, normal.z, uT.x, uT.z);
	decvert[2] = DECALVERTEX(u3.x, terr->GetYByQuadAndXZ(u3.x ,d3.z,Vert)+normal.y*0.1f, d3.z, normal.x, normal.y, normal.z, dT.x, uT.z);
	decvert[1] = DECALVERTEX(d3.x, terr->GetYByQuadAndXZ(d3.x ,u3.z,Vert)+normal.y*0.1f, u3.z, normal.x, normal.y, normal.z, uT.x, dT.z);
	decvert[3] = DECALVERTEX(d3.x, decvert[1].y, u3.z, normal.x, normal.y, normal.z, uT.x, dT.z);
	decvert[5] = DECALVERTEX(u3.x, decvert[2].y, d3.z, normal.x, normal.y, normal.z, dT.x, uT.z);
	decvert[4] = DECALVERTEX(u3.x, terr->GetYByQuadAndXZ(u3.x, u3.z,Vert)+normal.y*0.1f, u3.z, normal.x, normal.y, normal.z, dT.x, dT.z);

	
	Decals[0].Add(2, decvert, 3000);



	delete [] Vert;
	return S_OK;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
D3DXVECTOR3 CMyD3DApplication::BoxVsBoxCollision(CollisionModel *cm1, CollisionModel *cm2)
{

	D3DXVECTOR3 d1,u1,d2,u2; 
	float dx, dy, dz;

	d1 = cm1->vMin;
	u1 = cm1->vMax;
	d2 = cm2->vMin;
	u2 = cm2->vMax;


	dx = 0;
	dy = 0;
	dz = 0;


	
	if( 
	( ( (u1.x<u2.x)&&(u1.x>d2.x) )
			  ||( (d1.x<u2.x)&&(d1.x>d2.x) )
						  ||( (u2.x<u1.x)&&(u2.x>d1.x) ) )&&
	( ( (u1.y<u2.y)&&(u1.y>d2.y) )
			  ||( (d1.y<u2.y)&&(d1.y>d2.y) )
						  ||( (u2.y<u1.y)&&(u2.y>d1.y) ) )&&
	( ( (u1.z<u2.z)&&(u1.z>d2.z) )
			  ||( (d1.z<u2.z)&&(d1.z>d2.z) )
						  ||( (u2.z<u1.z)&&(u2.z>d1.z) ) )	)
	{


		//axis ^ y
		if( ( u1.y > d2.y && d2.y > d1.y ) && ( u1.y > u2.y && u2.y > d1.y ) )
		{
			if( d2.y - d1.y > u1.y - u2.y ) dy = u1.y - d2.y;
			else dy = d1.y - u2.y;
		}
		else if( ( u2.y > d1.y && d1.y > d2.y ) && ( u2.y > u1.y && u1.y > d2.y ) )
		{
			if( d1.y - d2.y > u2.y - u1.y ) dy = d1.y - u2.y;
			else dy = u1.y - d2.y;
		}
		else if( u1.y > d2.y && d1.y < d2.y )
		{
			dy = u1.y - d2.y;
		}
		else if( u2.y < u1.y && u2.y > d1.y )
		{
			dy = d1.y - u2.y;
		}	



		//axis ^ x
		if( ( u1.x > d2.x && d2.x > d1.x ) && ( u1.x > u2.x && u2.x > d1.x ) )
		{
			if( d2.x - d1.x > u1.x - u2.x ) dx = u1.x - d2.x;
			else dx = d1.x - u2.x;
		}
		else if( ( u2.x > d1.x && d1.x > d2.x ) && ( u2.x > u1.x && u1.x > d2.x ) )
		{
			if( d1.x - d2.x > u2.x - u1.x ) dx = d1.x - u2.x;
			else dx = u1.x - d2.x;
		}
		else if( u1.x > d2.x && d1.x < d2.x )
		{
			dx = u1.x - d2.x;
		}
		else if( u2.x < u1.x && u2.x > d1.x )
		{
			dx = d1.x - u2.x;
		}



		//axis ^ z
		if( ( u1.z > d2.z && d2.z > d1.z ) && ( u1.z > u2.z && u2.z > d1.z ) )
		{
			if( d2.z - d1.z > u1.z - u2.z ) dz = u1.z - d2.z;
			else dz = d1.z - u2.z;
		}
		else if( ( u2.z > d1.z && d1.z > d2.y ) && ( u2.z > u1.z && u1.z > d2.z ) )
		{
			if( d1.z - d2.z > u2.z - u1.z ) dz = d1.z - u2.z;
			else dz = u1.z - d2.z;
		}
		else if( u1.z > d2.z && d1.z < d2.z )
		{
			dz = u1.z - d2.z;
		}
		else if( u2.z < u1.z && u2.z > d1.z )
		{
			dz = d1.z - u2.z;
		}

	
		if( dx == 0 ) dx = 999999.f;
		if( dy == 0 ) dy = 999999.f;
		if( dz == 0 ) dz = 999999.f;
		

			 if( fabs(dx) < fabs(dz) && fabs(dx) < fabs(dy))
		{
			dy = 0;
			dz = 0;
		}
		else if( fabs(dz) < fabs(dx) && fabs(dz) < fabs(dy) )
		{
			dx = 0;
			dy = 0;
		}
		else if( fabs(dy) < fabs(dx) && fabs(dy) < fabs(dz) )
		{
			dx = 0;
			dz = 0;
		}




	}//end of check


	return D3DXVECTOR3(-dx, -dy, -dz);
}

//-----------------------------------------------------------------------------
// Name: FrameMove()
// Desc: Called once per frame, the call is the entry point for animating
//       the scene.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::FrameMove()
{
	int i,j,k;

	//do physx!
	StartNx();
	FetchNx();


#ifdef TESTMODE
	LogPlease("\n<b>FrameMove: </b>\n");
#endif

	//set global elapsed time
	g_fElapsedTime = m_fElapsedTime;

	if(!bMenu) m_pd3dDevice->ShowCursor( false );



	//timing stuff
	WorldTime += m_fElapsedTime/360000 * TimeQ;
	if( WorldTime > 24.f ) WorldTime -= 24.f;


	if( m_pBackgroundMusic->IsPlaying() == TRUE ) //isfinished actually, true once per song
	{
		m_pBackgroundMusic->PlayFile( "data/music/mesmeria.mp3", TRUE );
	}



/*
	//---- COLLISION DETENTION ----
#ifdef TESTMODE
	LogPlease("Collision Detention block: \n");
#endif

	if(SkinMesh[0]->cm != NULL) //means it's not the first frame
	{

		//camera VS terrain

		//---------


		//weapons VS objects

		//---------


		//skin meshes VS terrain

		for( i=0; i < NumOfSkinMesh; i++)
		{
			if( SkinMesh[i]->cm != NULL )
			{
				CollisionModel cmCur = SkinMesh[i]->cm[0];

				for( j=0; j < NumOfCTerrain; j++)
				{
					//if there's collision
					float Y = terrain[j].GetYByXZ(SkinMesh[i]->coordinate.x, SkinMesh[i]->coordinate.z);
					//float Y = terrain[j].GetYByXZ( cmCur.vMin.x, cmCur.vMin.z);
					
					if( Y != OUTOFTERRAIN && Y < cmCur.vMax.y && Y > cmCur.vMin.y )
					{
						D3DXVECTOR3 terrNormal;
						D3DXVec3Normalize( &terrNormal, &terrain[j].GetNormalByXZ(SkinMesh[i]->coordinate.x, SkinMesh[i]->coordinate.z));

						if( terrain[j].type == UP )
						{
							//set vertical collision bool
							SkinMesh[i]->bVerticalCollisionUP = TRUE;
						
							if(SkinMesh[i]->speed.y < 0) SkinMesh[i]->speed.y = 0;
							SkinMesh[i]->coordinate.y += -0.01f + Y - cmCur.vMin.y;

							//test
							//if(i==0) sprintf(logstr, "x:%f, y:%f, z:%f", terrNormal.x, terrNormal.y, terrNormal.z);
							if( terrNormal.y < 0.8f )
								SkinMesh[i]->coordinate = SkinMesh[i]->oldCoord;
						}
						else if( terrain[j].type == DOWN )
						{
							//set vertical collision bool
							SkinMesh[i]->bVerticalCollisionDOWN = TRUE;
						
							if(SkinMesh[i]->speed.y > 0) SkinMesh[i]->speed.y = -SkinMesh[i]->speed.y;
							SkinMesh[i]->coordinate -= ((cmCur.vMax.y - Y)) * terrNormal;
						}
					}



					// step decal & sound
					if( SkinMesh[i]->bSwitchLegs && (Y != OUTOFTERRAIN) && (cmCur.vMin.y - Y < 8.f) )
					{

						m_pMusicManager->Create3DSegmentFromFile( &SkinMesh[i]->m_pSound, L"data/sound/step.wav", TRUE, FALSE, NULL);

						if(i == 0)
						{
							SkinMesh[i]->m_pSound->SetObjectProperties( &D3DXVECTOR3(0,0,0) );
						}
						else
						{
							//D3DXMATRIX RotMatrix;
							//D3DXMatrixRotationYawPitchRoll( &RotMatrix, -SkinMesh[0]->angle.x, -SkinMesh[0]->angle.y, -SkinMesh[0]->angle.z);
							//D3DXVECTOR3 coord2new;
							//D3DXVec3TransformCoord( &coord2new,
							//						&(SkinMesh[i]->coordinate - SkinMesh[0]->coordinate),
							//						&RotMatrix );
							
							
							SkinMesh[i]->m_pSound->SetObjectProperties( &SkinMesh[i]->coordinate );
						}

						if( SkinMesh[i]->m_pSound) SkinMesh[i]->m_pSound->Play();
						

						//if(SkinMesh[i]->bRightLegStep) AddDecal( &SkinMesh[i]->cm[4], &terrain[j] );
						//else if(SkinMesh[i]->bLeftLegStep) AddDecal( &SkinMesh[i]->cm[5], &terrain[j] );

					}

				}
			}
		}

		//---------
		
		
		//weapons VS skin meshes

		for( i=0; i < NumOfSkinMesh; i++)
		{
			if( SkinMesh[i]->cm != NULL )
			{
				for( j=0; j < NumOfWeapons; j++)
				{
					if( Weapons[j].cm != NULL )
					{
						if(	Weapons[j].attacherId != SkinMesh[i]->id &&
							SkinMesh[ Weapons[j].attacherId ]->IsAttack() != NOATTACK &&
							(SkinMesh[i]->bHurt != TRUE && SkinMesh[i]->bDead != TRUE) &&
							D3DXVECTOR3(0,0,0) != BoxVsBoxCollision( &SkinMesh[i]->cm[0], &Weapons[j].cm[0]) )
						{

							if( SkinMesh[ Weapons[j].attacherId ]->IsAttack() == SkinMesh[i]->IsBlock())
							{
								SkinMesh[ Weapons[j].attacherId ]->bFail = TRUE;

								//soundeffect
								m_pMusicManager->Create3DSegmentFromFile( &SkinMesh[i]->m_pSound, L"data/sound/block.wav", TRUE, FALSE, NULL);
								if( SkinMesh[i]->m_pSound ) SkinMesh[i]->m_pSound->Play();

							}

							//hitted!
							else
							{
								//blood effect
								ParticleSystem[1].SetEmitterPosition( Weapons[j].cm[0].vMax );
								ParticleSystem[1].Emit( 400 * m_fElapsedTime );

								SkinMesh[i]->bHurt = TRUE;
								SkinMesh[i]->life -= m_fElapsedTime*100;

								if(SkinMesh[i]->life <= 0) SkinMesh[i]->bDead = TRUE;

								//sound effect
								m_pMusicManager->Create3DSegmentFromFile( &SkinMesh[i]->m_pSound, L"data/sound/hurt.wav", TRUE, FALSE, NULL);
								if( SkinMesh[i]->m_pSound ) SkinMesh[i]->m_pSound->Play();
							}
						}
					}
				}
			}
		}

		//---------

	}
	//------------------
*/
	

	//weapons VS skin meshes

	for( i=0; i < NumOfSkinMesh; i++)
	{
		//get hitted weapon index
		if( SkinMesh[i]->usrdata.bHit == TRUE )
		{
			j = GetCModelIndexByID( Weapons, NumOfWeapons, SkinMesh[i]->usrdata.targetid );
			//HTMLLog("%d - %d\n", j, SkinMesh[i]->usrdata.targetid);
			SkinMesh[i]->usrdata.bHit = FALSE;
	
			if(	Weapons[j]->attacherId != SkinMesh[i]->id &&
				SkinMesh[ Weapons[j]->attacherId ]->IsAttack() != NOATTACK &&
				(SkinMesh[i]->bHurt != TRUE && SkinMesh[i]->bDead != TRUE && SkinMesh[i]->OnceAnimationHurt()==FALSE ) 
			  )
			{

				if( SkinMesh[ Weapons[j]->attacherId ]->IsAttack() == SkinMesh[i]->IsBlock())
				{
					SkinMesh[ Weapons[j]->attacherId ]->bFail = TRUE;

					//soundeffect
					m_pMusicManager->Create3DSegmentFromFile( &SkinMesh[i]->m_pSound, L"data/sound/block.wav", TRUE, FALSE, NULL);
					if( SkinMesh[i]->m_pSound ) SkinMesh[i]->m_pSound->Play();
				}

				//hitted!
				else
				{
					//blood effect
					ParticleSystem[1].SetEmitterPosition( NxVec3ToDxVec3(Weapons[j]->object->getGlobalPosition() ) );//Weapons[j].cm[0].vMax );
					ParticleSystem[1].Emit( 400 * m_fElapsedTime );

					SkinMesh[i]->bHurt = TRUE;
					SkinMesh[i]->life -= m_fElapsedTime*1000;

					//death poll
					if(SkinMesh[i]->life <= 0)
					{ 
						SkinMesh[i]->bDead = TRUE; 
						SkinMesh[i]->PassAnimation("FALLDOWN", 0.1f);
						//SkinMesh[i]->SetFree(TRUE);
					}

					//sound effect
					m_pMusicManager->Create3DSegmentFromFile( &SkinMesh[i]->m_pSound, L"data/sound/hurt.wav", TRUE, FALSE, NULL);
					if( SkinMesh[i]->m_pSound ) SkinMesh[i]->m_pSound->Play();
				}
			}
		}
	}



	//---- FRAMEMOVING itself ----
	// most of updating functions



#ifdef TESTMODE
	LogPlease("Updating functions: \n");
#endif

	//skinned meshes
	for(i=0; i<NumOfSkinMesh; i++)
	{
		
		ProcessMovement( SkinMesh[i] );
		SkinMesh[i]->UpdateCoordinates(m_fElapsedTime);
		
	
		while(SkinMesh[i]->angle.y > D3DX_PI * 2.0f) SkinMesh[i]->angle.y -= D3DX_PI * 2.0f;
		while(SkinMesh[i]->angle.y < 0 ) SkinMesh[i]->angle.y += D3DX_PI * 2.0f;

		if(i==0) //player controlled
		{
			// controls
			DInputCallback();

			//camera
			camera->Update(D3DXVECTOR3( (float)input->mouse.lX, (float)input->mouse.lY, (float)input->mouse.lZ ),
						   //D3DXVECTOR3( SkinMesh[0]->object->getGlobalPosition().x, SkinMesh[0]->object->getGlobalPosition().y + 100, SkinMesh[0]->object->getGlobalPosition().z ),
						   //D3DXVECTOR3( SkinMesh[0]->coordinate.x, SkinMesh[0]->coordinate.y + 100, SkinMesh[0]->coordinate.z ),
						   //NxVec3ToDxVec3( SkinMesh[0]->controller->GetCharacterActor()->getGlobalPosition() ),
						   NxVec3ToDxVec3( SkinMesh[0]->GetBipedByName("CBP_Head")->getGlobalPosition() ),
						   SkinMesh[0]->angle.y,
						   m_fElapsedTime,
						   D3DX_PI/3.1f,
						   (FLOAT)m_d3dsdBackBuffer.Width / (FLOAT)m_d3dsdBackBuffer.Height,
						   1.0f,
						   100000.0f);	

			camera->Apply(m_pd3dDevice);
		
		}
		else //npc or non-c)
		{
			AICallback( SkinMesh[i] );
		}



		//discount collisions
		SkinMesh[i]->bVerticalCollisionUP = FALSE;
		SkinMesh[i]->bVerticalCollisionDOWN = FALSE;

	}



//---- BASE UPDATE - (test block)
//----

	//SKINNED
#ifdef TESTMODE
	LogPlease("SkinnedObjects base update: \n");
#endif

	for(i=0; i < NumOfSkinMesh; i++)
	{

		//---- CLONE ----
		if(SkinMesh[i]->MType == CLONE)
		{		

			//complicated procedure of copying
			SkinMesh[i]->Update(0);
			SkinMesh[i]->UpdateMatrices(camera->util.GetViewMatrix(), camera->util.GetProjMatrix());

			AnimSetInfo animSetInfo;

			//SkinMesh[i]->ParentModel->SetModelState( &animSetInfo ); //remember old modelstate
			SkinMesh[i]->SetModelState( SkinMesh[i]->ParentModel ); //set new modelstate

			SkinMesh[i]->ParentModel->Update( m_fElapsedTime );


			SkinMesh[i]->UpdateMatricesPhysX( &SkinMesh[i]->controller->GetCharacterActor()->getGlobalPose(),
											  &SkinMesh[i]->controller->GetCharacterActor()->getGlobalPosition() );
			SkinMesh[i]->ParentModel->SetMatWorld( SkinMesh[i]->GetMatWorld() );
			
	

			SkinMesh[i]->ParentModel->UpdateMatrices( camera->util.GetViewMatrix(), camera->util.GetProjMatrix() );

			SkinMesh[i]->GetModelState( SkinMesh[i]->ParentModel );
			SkinMesh[i]->StoreBoneMatrices( SkinMesh[i]->ParentModel );
			SkinMesh[i]->UpdateBipedsPhysX();


			//attached
			Heads[i]->CopyBonesState( SkinMesh[i], camera->util.GetViewMatrix(), camera->util.GetProjMatrix());
			

			if( SkinMesh[i]->bAttacher == TRUE )
			{
				Weapons[ SkinMesh[i]->attachedId ]->UpdateAttachmentMatrix();
			}		
			
		}

	}


	for(i=0; i < NumOfObjects; i++)
	{
		Objects[i]->Update(m_fElapsedTime);
		Objects[i]->UpdateMatricesPhysX();
		Objects[i]->UpdateMatrices(camera->util.GetViewMatrix(), camera->util.GetProjMatrix());
		Objects[i]->StoreFrameMatrices(NULL);
	}

	for(i=0; i < NumOfWeapons; i++)
	{
		Weapons[i]->Update(m_fElapsedTime);
		Weapons[i]->UpdateMatrices(camera->util.GetViewMatrix(), camera->util.GetProjMatrix());
		Weapons[i]->UpdateMatricesPhysX();
		Weapons[i]->StoreFrameMatrices(NULL);
	}

//----
//------------------






	//---- Particles ----
#ifdef TESTMODE
	LogPlease("Particles: \n");
#endif


	//snow
	ParticleSystem[0].SetEmitterPosition(SkinMesh[0]->coordinate);

	//cheap fire  &  spell effect
	for( i=0; i < NumOfSkinMesh; i++)
	{
		char* name;
		if(SkinMesh[i]->MType == BASE) name = SkinMesh[i]->GetTrackAnimationSetName(0);
		else if(SkinMesh[i]->MType == CLONE) name = SkinMesh[i]->ASI.AnimSetName;

		
		if( !strcmp( name, "CAST1") )//&& SkinMesh[i]->cm != NULL )
		{
			ParticleSystem[2].SetEmitterPosition( NxVec3ToDxVec3(SkinMesh[i]->GetBipedByName("CBP_L_Hand")->getGlobalPosition()) );
			ParticleSystem[2].Emit(1);
			ParticleSystem[2].FrameMove(0, rand()%50);
		}
		else if( !strcmp( name, "CAST2") )//&& SkinMesh[i]->cm != NULL )
		{
			ParticleSystem[3].SetEmitterPosition( NxVec3ToDxVec3(SkinMesh[i]->GetBipedByName("CBP_Pelvis")->getGlobalPosition()) );
			ParticleSystem[3].Emit(1);
			ParticleSystem[3].FrameMove(0, rand()%50);
		}
		else if( !strcmp( name, "CAST3") )//&& SkinMesh[i]->cm != NULL )
		{
			//one hand
			ParticleSystem[2].SetEmitterPosition( NxVec3ToDxVec3(SkinMesh[i]->GetBipedByName("CBP_L_Hand")->getGlobalPosition()) );
			ParticleSystem[2].Emit(1);
			ParticleSystem[2].FrameMove(0, rand()%50);

			//second hand
			ParticleSystem[2].SetEmitterPosition( NxVec3ToDxVec3(SkinMesh[i]->GetBipedByName("CBP_R_Hand")->getGlobalPosition())  );
			ParticleSystem[2].Emit(1);
			ParticleSystem[2].FrameMove(0, rand()%50);
		}
		
	}

	for( i=0; i < NumOfParticle; i++)
	{
		ParticleSystem[i].FrameMove(m_fElapsedTime, 1000);
	}

	//---------
	

	
	//---- Decals ----
#ifdef TESTMODE
	LogPlease("Decals: \n");
#endif

	for( i=0; i < NumOfDecals; i++)
	{
		Decals[i].Update(m_fElapsedTime);
	}

	//---------

	//HTMLLog("GetTickCount: %dl\n", timeGetTime());

#ifdef TESTMODE
	LogPlease("<b>FrameMove successfully ended</b>\n");
#endif


    return S_OK;
}



//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::UpdateEffectVariables()
{
	D3DXVECTOR4 vLightVec, vEyeVec, vEyePos;
	int i;

	//float x = 90; //timeGetTime()/30%360*3.14f/180.f;
	float x = WorldTime * ( 3.14f / 24.f );


	D3DXVECTOR3 vLightPos = D3DXVECTOR3(0 ,1000*sin(x), 1000*cos(x));
	//if( sin(x) < 0.0f ) LightActivity = MAX(0, (1.f - fabs(sin(x)) )*2.f - 1.f);


	D3DXVECTOR3 vLightAim = D3DXVECTOR3( 0, 0, 0);

	vLightVec = D3DXVECTOR4(vLightPos.x, vLightPos.y, vLightPos.z, 1.0f);
	m_pEffect->SetVector("vLightVec", &vLightVec);

	vEyeVec = D3DXVECTOR4(camera->util.GetEyePt().x - camera->util.GetLookatPt().x,
						  camera->util.GetEyePt().y - camera->util.GetLookatPt().y,
						  camera->util.GetEyePt().z - camera->util.GetLookatPt().z,
						  1.0f);
	m_pEffect->SetVector("vEyeVec", &vEyeVec );

	vEyePos = D3DXVECTOR4(camera->util.GetEyePt().x,
						  camera->util.GetEyePt().y,
						  camera->util.GetEyePt().z,
						  1.0f);
	m_pEffect->SetVector("vEyePos", &vEyePos );


	float k = MAX(0, 2.0f * (1.f - fabs(WorldTime / 12.f - 1.f)) - 0.4f);
	k = MIN( k, 0.8f );
	EnvColor = D3DCOLOR_COLORVALUE(k, k, k, k);

	
	D3DXCOLOR color = D3DXCOLOR(EnvColor);
	D3DXVECTOR4 colorVec = D3DXVECTOR4(color.r, color.g, color.b, color.a);
	m_pEffect->SetVector("vLightColor", &colorVec);


	//set lights
	//m_pEffect->SetVector( "vLight2", &D3DXVECTOR4(dirlights[0].Pos.x, dirlights[0].Pos.y, dirlights[0].Pos.z, 1.0f) );
	//m_pEffect->SetVector( "vLight2Color", &D3DXVECTOR4(1, 0.2f, 0.2f, 1.0f));



	//lighting relative
	vLightPos += SkinMesh[0]->coordinate;
	vLightAim += SkinMesh[0]->coordinate;



	D3DXMATRIX matProj, matView;
	D3DXMATRIX matLightViewProj;

	D3DXMatrixOrthoLH( &matProj, 5000.0f, 5000.0f, 1.0f, 10000.0f );
	D3DXMatrixLookAtLH( &matView, &vLightPos, &vLightAim, &D3DXVECTOR3(0.f, 1.f, 0.f) );
	matLightViewProj = matView * matProj;
	

	// Compute the texture matrix for shadow maps
	float fTexOffs = 0.5 + (0.5 / (float)SHADOWMAPSIZE);
	D3DXMATRIX matTexAdj( 0.5f,		0.0f,	0.0f,	0.0f,
						  0.0f,    -0.5f,	0.0f,	0.0f,
						  0.0f,		0.0f,	1.0f,	0.0f,
						  fTexOffs, fTexOffs,  0.0f, 1.0f );

;
	//D3DXMATRIX matTexture = matLightViewProj * matTexAdj;

	//setting variables
	m_pEffect->SetMatrix( "mLightViewProj", &matLightViewProj );
	m_pEffect->SetMatrix( "mTexAdj", &matTexAdj);

	m_pEffect->SetFloat( "time", (float)timeGetTime() );
	m_pEffect->SetFloat( "ElapsedTime", (float)m_fElapsedTime );

	return S_OK;
}





//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Called once per frame, the call is the entry point for 3d
//       rendering. This function sets up render states, clears the
//       viewport, and renders the scene.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::Render()
{
	int i;
	UINT uPasses, uPass;


	// Clear the backbuffer
    m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, 
                         EnvColor, 1.0f, 0L );




    // Setup the light
    D3DLIGHT9 light;
    D3DUtil_InitLight( light, D3DLIGHT_DIRECTIONAL, -1.0f, 0.0f, 1.0f );
    m_pd3dDevice->SetLight(0, &light );
    m_pd3dDevice->LightEnable(0, TRUE );





	//set states
	m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	
	m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE);
	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );


    // Begin the scene
    if( SUCCEEDED( m_pd3dDevice->BeginScene() ) )
    {

		
		//ONE-TIME RENDER BLOCK
		if( m_bOneTimeRender == TRUE )
		{
			m_bOneTimeRender = FALSE;
		}



		//updating effect variables!
		//-----

		UpdateEffectVariables();

		//-----



		//skybox render first

		//---------
		// fog	
		float Start = 5100.f;
		float End = 8100.f;
		float Density = 0.1f;
		DWORD Mode = D3DFOG_LINEAR;

		m_pd3dDevice->SetRenderState(D3DRS_FOGENABLE, TRUE);
		m_pd3dDevice->SetRenderState(D3DRS_FOGCOLOR, EnvColor);

		m_pd3dDevice->SetRenderState(D3DRS_FOGTABLEMODE, Mode);
		m_pd3dDevice->SetRenderState(D3DRS_FOGDENSITY, *((DWORD*) (&Density)));


		m_pd3dDevice->SetRenderState(D3DRS_FOGSTART, *(DWORD *)(&Start)); 
		m_pd3dDevice->SetRenderState(D3DRS_FOGEND, *(DWORD *)(&End));



		//draw skybox
#ifdef TESTMODE
			LogPlease("drawing SkyBox...");
#endif

		if(FAILED( m_pEffect->SetTechnique( "SkyBox" )) ) LogPlease("Cannot set technique SkyBox!\n");
		uPasses = 0;
		m_pEffect->Begin( &uPasses, 0 );
		for(uPass = 0; uPass < uPasses; uPass++)
		{
				m_pEffect->BeginPass( uPass );
				skybox->Render( D3DXVECTOR3( camera->util.GetEyePt().x,
											 camera->util.GetEyePt().y,
											 camera->util.GetEyePt().z ),
								m_fElapsedTime,
								NULL,
								camera->util.GetViewMatrix(), camera->util.GetProjMatrix());

				m_pEffect->EndPass();
		}
		m_pEffect->End();

#ifdef TESTMODE
			LogPlease(" ...success\n");
#endif


		Start = VIEWDISTANCE_TERRAIN/2.f;
		End = VIEWDISTANCE_TERRAIN;


		//world fog state
		m_pd3dDevice->SetRenderState(D3DRS_FOGENABLE, TRUE);
		m_pd3dDevice->SetRenderState(D3DRS_FOGSTART, *(DWORD *)(&Start)); 
		m_pd3dDevice->SetRenderState(D3DRS_FOGEND, *(DWORD *)(&End));
		//--------


	
		
	
		//--------------------------------
		//---- DRAW SCENE USING SHADERS --
		//--------------------------------

		

		//preparing to draw into shadow map
		m_pd3dDevice->GetRenderTarget( 0, &g_pOldColorRT );
		m_pd3dDevice->GetDepthStencilSurface( &g_pOldDepthRT );
    
		m_pd3dDevice->SetRenderTarget( 0, g_pShadowSurf );
		m_pd3dDevice->SetDepthStencilSurface( g_pShadowDepth );
    
		m_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xFFFFFFFF, 1.0f, 0 );

		// don't draw textures at the first time! - time economy
		for(i = 0; i < NumOfSkinMesh; i++)
		{
			SkinMesh[i]->msg = DONTDRAWTEXTURES;
			(*Heads[i]).msg = DONTDRAWTEXTURES;
		}
		for(i = 0; i < NumOfObjects; i++) Objects[i]->msg = DONTDRAWTEXTURES;
		for(i = 0; i < NumOfWeapons; i++) Weapons[i]->msg = DONTDRAWTEXTURES;
		for(i = 0; i < NumOfArmor; i++) Armor[i]->msg = DONTDRAWTEXTURES;
		for(i = 0; i < NumOfCTerrain; i++) terrain[i].msg = 1; 


	
		if(bShadowsEnable)
		{

			//-----
			// Part 1: shadow pass

			//drawing models
			// 1. skinned meshes
			// 2. non-skinned meshes


			//SKINNED
#ifdef TESTMODE
			LogPlease("drawing Skinned meshes: shadow pass...");
#endif

			m_pEffect->SetTechnique( "SMSkinning" );
			uPasses = 0;
			m_pEffect->Begin( &uPasses, 0 );
			for(uPass = 0; uPass < uPasses; uPass++)
			{
				m_pEffect->BeginPass( uPass );
				
				for(i=0; i < NumOfSkinMesh; i++)
				{

					//if(DISTANCE(*SkinMesh) < VIEWDISTANCE_OBJECT) 
						SkinMesh[i]->Render();
					//if(DISTANCE(*SkinMesh) < VIEWDISTANCE_OBJECT) 
						Heads[i]->Render();

					//armor & stuff
					if( SkinMesh[i]->CurrentArmor != NOARMOR /*&& DISTANCE(*SkinMesh) < VIEWDISTANCE_OBJECT*/ )
					{
						//HTMLLog("\nArmor[ %d ]->CopyBonesState( SkinMesh[ %d ], camera )", SkinMesh[i]->CurrentArmor, i);				
						Armor[ SkinMesh[i]->CurrentArmor ]->CopyBonesState( SkinMesh[i], camera->util.GetViewMatrix(), camera->util.GetProjMatrix());
						//Armor[ SkinMesh[i]->CurrentArmor ].StoreBoneMatrices(NULL);
						Armor[ SkinMesh[i]->CurrentArmor ]->Render();
					}

				}

				m_pEffect->EndPass();

			}
			m_pEffect->End();

#ifdef TESTMODE
			LogPlease(" ...success\n");
#endif


			//ORDINARY
#ifdef TESTMODE
			LogPlease("drawing Ordinary meshes: shadow pass...");
#endif

			m_pEffect->SetTechnique( "SMOrdinary" );
			uPasses = 0;
			m_pEffect->Begin( &uPasses, 0 );
			for(uPass = 0; uPass < uPasses; uPass++)
			{
				m_pEffect->BeginPass( uPass );
				
				for(i=0; i < NumOfObjects; i++)
				{
					if(DISTANCE(Objects) < VIEWDISTANCE_OBJECT) Objects[i]->Render();
				}
				for(i=0; i < NumOfWeapons; i++)
				{
					if(D3DXVec3Length(&( SkinMesh[ Weapons[i]->attacherId ]->coordinate - camera->util.GetEyePt()))
					   < VIEWDISTANCE_OBJECT) Weapons[i]->Render();
				}

				m_pEffect->EndPass();
				
			}
			m_pEffect->End();

#ifdef TESTMODE
			LogPlease(" ...success\n");
#endif

		}//end of (bShadowsEnable)



		//-----
		// Part 2 - simple drawing

		// preparing 2
		// draw textures this time
		for(i = 0; i < NumOfSkinMesh; i++)
		{
			SkinMesh[i]->msg = 0;
			(*Heads[i]).msg = 0;
		}
		for(i = 0; i < NumOfObjects; i++) Objects[i]->msg = 0;
		for(i = 0; i < NumOfWeapons; i++) Weapons[i]->msg = 0;
		for(i = 0; i < NumOfArmor; i++) Armor[i]->msg = 0;
		for(i = 0; i < NumOfCTerrain; i++) terrain[i].msg = 0; 



		m_pd3dDevice->SetRenderTarget( 0, g_pOldColorRT );
		m_pd3dDevice->SetDepthStencilSurface( g_pOldDepthRT );
		SAFE_RELEASE( g_pOldColorRT );
		SAFE_RELEASE( g_pOldDepthRT );
		
		// Set the textures
		m_pEffect->SetTexture( "tShadowMap", g_pShadowMap );


		// things, that dont cast shadows
		// 1. terrain
		// 2. decals
		// 3. water


#ifdef TESTMODE
			LogPlease("Drawing terrain... ");
#endif

		// TERRAIN
		m_pEffect->SetTechnique( "Terrain" );
		uPasses = 0;
		m_pEffect->Begin( &uPasses, 0 );
		for(uPass = 0; uPass < uPasses; uPass++)
		{

			m_pEffect->BeginPass( uPass );

			//draw terrain
			for(i=0; i < NumOfCTerrain; i++)
			{
				terrain[i].Render( camera->util.GetViewMatrix(),
								   camera->util.GetProjMatrix(),
								   terrain[i].GetNByXZ(SkinMesh[0]->coordinate.x, SkinMesh[0]->coordinate.z),
								   terrain[i].GetNByXZ(camera->vEye.x, camera->vEye.z) );
			}

			m_pEffect->EndPass();

		}
		m_pEffect->End();


#ifdef TESTMODE
			LogPlease(" ...success\n");
#endif



#ifdef TESTMODE
			LogPlease("Drawing decals...");
#endif
		//DECALS
		m_pEffect->SetTechnique( "Ordinary" );
		uPasses = 0;
		m_pEffect->Begin( &uPasses, 0 );
		for(uPass = 0; uPass < uPasses; uPass++)
		{
			m_pEffect->BeginPass( uPass );
			
			//draw decals
			for(i=0; i < NumOfDecals; i++)
			{
				D3DXMATRIXA16  matTemp;
				D3DXMatrixTranslation( &matTemp,0,0,0);
				m_pd3dDevice->SetTransform(D3DTS_WORLD, &matTemp); 

				Decals[i].Render();
			}

			m_pEffect->EndPass();
		}
		m_pEffect->End();

#ifdef TESTMODE
			LogPlease(" ...success\n");
#endif	



		// -----
		// WATER

#ifdef TESTMODE
			LogPlease("Drawing water... ");
#endif

		//preparing to draw into shadow map
		m_pd3dDevice->GetRenderTarget( 0, &g_pOldColorRT );
		m_pd3dDevice->GetDepthStencilSurface( &g_pOldDepthRT );

		//refraction (simple - just for this case)
		m_pd3dDevice->StretchRect(g_pOldColorRT, NULL, water->m_pRefrSurf, NULL, D3DTEXF_NONE);

		m_pd3dDevice->SetRenderTarget( 0, water->m_pEnvSurf );
		m_pd3dDevice->SetDepthStencilSurface( g_pNewDepthRT );
		m_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET, EnvColor, 0.0f, 0 );

		//reflect and draw skybox
		D3DXMATRIX reflectMat = water->GetReflectionMatrix();
		D3DXVECTOR3 newCamEye, newCamAt;
		D3DXVec3TransformCoord( &newCamEye, &camera->util.GetEyePt(), &reflectMat );
		D3DXVec3TransformCoord( &newCamAt, &camera->vAt, &reflectMat );

		D3DXMATRIX newViewMatrix;
		D3DXMatrixLookAtLH( &newViewMatrix, &newCamEye, &newCamAt, &camera->vUp );
		//m_pd3dDevice->SetTransform( D3DTS_VIEW,  &newViewMatrix );


		//DRAW REFLECTED OBJECTS!
			//render skybox
		
			m_pEffect->SetTechnique( "SkyBox" );
			uPasses = 0;
			m_pEffect->Begin( &uPasses, 0 );
			for(uPass = 0; uPass < uPasses; uPass++)
			{
					m_pEffect->BeginPass( uPass );

					skybox->Render( newCamEye,
									0,
									NULL,
									newViewMatrix,
									camera->util.GetProjMatrix());

					m_pEffect->EndPass();
			}
			m_pEffect->End();
		
			
		m_pd3dDevice->SetRenderTarget( 0, g_pOldColorRT );
		m_pd3dDevice->SetDepthStencilSurface( g_pOldDepthRT );
		SAFE_RELEASE( g_pOldColorRT );
		SAFE_RELEASE( g_pOldDepthRT );
		SAFE_RELEASE( g_pNewDepthRT );

		//camera->Apply(m_pd3dDevice);
		

		//drawing itself
		m_pEffect->SetTechnique( "Water" );
		uPasses = 0;
		m_pEffect->Begin( &uPasses, 0 );
		for(uPass = 0; uPass < uPasses; uPass++)
		{
			m_pEffect->BeginPass( uPass );
			
			water->Render( camera->util.GetViewMatrix(),
						   camera->util.GetProjMatrix() );
			m_pEffect->EndPass();
		}
		m_pEffect->End();

#ifdef TESTMODE
			LogPlease(" ...success\n");
#endif
		// end of water
		//-----



		// Drawing Models
		// 1. skinned meshes
		// 2. non-skinned meshes

		//SKINNED
#ifdef TESTMODE
			LogPlease("drawing Skinned objects...");
#endif

		m_pEffect->SetTechnique( "Skinning" );
		uPasses = 0;
		m_pEffect->Begin( &uPasses, 0 );
		for(uPass = 0; uPass < uPasses; uPass++)
		{
			m_pEffect->BeginPass( uPass );
			
			for(i=0; i < NumOfSkinMesh; i++)
			{
				//if(DISTANCE(*SkinMesh) < VIEWDISTANCE_OBJECT)
					SkinMesh[i]->Render();
				//if(DISTANCE(*SkinMesh) < VIEWDISTANCE_OBJECT)
				if( camera->mode != FIRSTPERSONVIEW || i != 0 )
					Heads[i]->Render();

				//armor & stuff
				if( SkinMesh[i]->CurrentArmor != NOARMOR /*&& DISTANCE(*SkinMesh) < VIEWDISTANCE_OBJECT*/ )
				{
					Armor[ SkinMesh[i]->CurrentArmor ]->CopyBonesState( &(*SkinMesh[i]), camera->util.GetViewMatrix(), camera->util.GetProjMatrix());
					//Armor[ SkinMesh[i]->CurrentArmor ].StoreBoneMatrices(NULL);
					Armor[ SkinMesh[i]->CurrentArmor ]->Render();
				}

			}
			m_pEffect->EndPass();

		}
		m_pEffect->End();

#ifdef TESTMODE
			LogPlease("...success\n");
#endif


		//ORDINARY
#ifdef TESTMODE
			LogPlease("drawing Ordinary objects...");
#endif
		m_pEffect->SetTechnique( "Ordinary" );
		uPasses = 0;
		m_pEffect->Begin( &uPasses, 0 );
		for(uPass = 0; uPass < uPasses; uPass++)
		{
			m_pEffect->BeginPass( uPass );
			
			for(i=0; i < NumOfObjects; i++)
			{
				if(DISTANCE(Objects) < VIEWDISTANCE_OBJECT) Objects[i]->Render();
			}
			for(i=0; i < NumOfWeapons; i++)
			{
				if(DISTANCE(Weapons) < VIEWDISTANCE_OBJECT) Weapons[i]->Render();
			}

			m_pEffect->EndPass();

		}
		m_pEffect->End();
#ifdef TESTMODE
			LogPlease("...success\n");
#endif	

		//------------------------------
		//------------------------------
		//------------------------------



		
		//bboxes - slow slow slow version

		//for( i=0; i < NumOfObjects; i++ )
		//	RenderPhysXBox( Objects[i]->object );
		//for( i=0; i < NumOfSkinMesh; i++)
		//	RenderPhysXCapsule( SkinMesh[i]->controller->GetCharacterActor() );
		//for( i=0; i < NumOfWeapons; i++)
		//	RenderPhysXBox( Weapons[i]->object );

		
		for( i=0; i < SkinMesh[2]->NumOfBiped; i++)
		{
			RenderPhysXBox( SkinMesh[2]->biped[i] );
		}
		

		/*
		if( bConsole && bBoundingBoxes )
		{	
			for(i=0; i < NumOfSkinMesh; i++) RenderCM( &SkinMesh[i]->cm[0] );
			for(i=0; i < NumOfWeapons; i++) RenderCM( &Weapons[i].cm[0] );
			for(i=0; i < NumOfObjects; i++) RenderCM( &Objects[i].cm[0] );
		}
		*/


		//particles
#ifdef TESTMODE
			LogPlease("drawing particles...");
#endif

		m_pEffect->SetTechnique( "Particle" );
		uPasses = 0;
		m_pEffect->Begin( &uPasses, 0 );
		for(uPass = 0; uPass < uPasses; uPass++)
		{
			m_pEffect->BeginPass( uPass );

			for( i=0; i < NumOfParticle; i++)
			{  
				
				//D3DXMATRIXA16  matTemp;
				//D3DXMatrixTranslation( &matTemp,0,0,0);
				//m_pd3dDevice->SetTransform(D3DTS_WORLD, &matTemp);
				m_pEffect->SetMatrix( "mViewProj", &(camera->util.GetViewMatrix() * camera->util.GetProjMatrix()) );
				m_pEffect->SetMatrix( "mWorld", &camera->util.GetViewMatrix() );

				m_pd3dDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
				ParticleSystem[i].Render();
				m_pd3dDevice->SetRenderState(D3DRS_FOGENABLE, TRUE);
			}

			m_pEffect->EndPass();
		}
		m_pEffect->End();

#ifdef TESTMODE
			LogPlease(" ...success\n");
#endif


		//--------
		//GUI
#ifdef TESTMODE
			LogPlease("drawing GUI...");
#endif

		//menu
		if(bMenu)
		{
			pic[0].Render(NULL, &D3DXVECTOR2(1,1), NULL, 0, &D3DXVECTOR3(0,0,0), FALSE );

		}
		else
		{
			//crosshair
			pic[5].Render(NULL, &D3DXVECTOR2( 1.0f, 1.0f), NULL, 0, &D3DXVECTOR3(SCREENWIDTH*0.495f, SCREENHEIGHT*0.495f, 0), TRUE );

			//healthbar
			pic[1].Render(NULL, &D3DXVECTOR2( MAX((float)SkinMesh[0]->life/100.f, 0), 1.f), NULL, 0, &D3DXVECTOR3(SCREENWIDTH*0.02f, SCREENHEIGHT*0.94f, 0), TRUE );
			
			//manabar
			pic[2].Render(NULL, &D3DXVECTOR2( MAX((float)SkinMesh[0]->mana/100.f, 0), 1.f), NULL, 0, &D3DXVECTOR3(SCREENWIDTH*0.02f, SCREENHEIGHT*0.96f, 0), TRUE );
		
			//chatback
			if(bChat)
			{
				//active chatback
				pic[3].Render(NULL, &D3DXVECTOR2( 1.f, 1.f), NULL, 0, &D3DXVECTOR3(SCREENWIDTH*0.65, SCREENHEIGHT - 253, 0), TRUE );
			}
			else
			{
				//passive chatback
				pic[4].Render(NULL, &D3DXVECTOR2( 1.f, 1.f), NULL, 0, &D3DXVECTOR3(SCREENWIDTH*0.65, SCREENHEIGHT - 253, 0), TRUE );
			}
		}

#ifdef TESTMODE
			LogPlease("...success\n");
#endif
		//----
		//--------



        // Render stats and help text  
		RenderText();


		


#ifdef TESTMODE
			LogPlease("<b>Render successfully completed!</b>");
#endif
        // End the scene.
        m_pd3dDevice->EndScene();
    }



    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: RenderPhysXCapsule
// Desc: useful function
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RenderPhysXCapsule(NxActor* object)
{
	if(!object) return E_FAIL;

	D3DXMATRIX pmatrix;

	NxShape*const* shape = object->getShapes();
	NxU32 nShapes = object->getNbShapes();
	float height;
	float radius;

	while (nShapes--)
	{
		NxCapsuleShape* CapsuleShape = shape[nShapes]->isCapsule();
		radius = CapsuleShape->getRadius();
		height = CapsuleShape->getHeight();
		//HTMLLog("%d %d %d %d\n", (int)nShapes, (int)vector.x, (int)vector.y, (int)vector.z);

		if (en_box) en_box->Release();
		en_box = NULL;
		D3DXCreateBox( m_pd3dDevice,
					   radius,
					   height + radius,
					   radius,
					   &en_box,
					   NULL );
		if(!en_box) return E_FAIL;
		

		CapsuleShape->getGlobalPose().getColumnMajor44( pmatrix );

		m_pd3dDevice->SetTransform(D3DTS_WORLD, &pmatrix);

		en_box->DrawSubset(0);
	}

	return S_OK;
}


//-----------------------------------------------------------------------------
// Name: RenderPhysXBox
// Desc: useful function
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RenderPhysXBox(NxActor* object)
{
	if(!object) return E_FAIL;

	D3DXMATRIX pmatrix;

	NxShape*const* shape = object->getShapes();
	NxU32 nShapes = object->getNbShapes();
	NxVec3 vector;

	while (nShapes--)
	{
		NxBoxShape* BoxShape = shape[nShapes]->isBox();
		vector = BoxShape->getDimensions() * 2;
		//HTMLLog("%d %d %d %d\n", (int)nShapes, (int)vector.x, (int)vector.y, (int)vector.z);

		if (en_box) en_box->Release();
		en_box = NULL;
		D3DXCreateBox( m_pd3dDevice,
					   vector.x,
					   vector.y,
					   vector.z,
					   &en_box,
					   NULL );
		if(!en_box) return E_FAIL;
		

		BoxShape->getGlobalPose().getColumnMajor44( pmatrix );

		m_pd3dDevice->SetTransform(D3DTS_WORLD, &pmatrix);

		en_box->DrawSubset(0);
	}

	return S_OK;
}



/*

//-----------------------------------------------------------------------------
// Name: RenderÑM()
// Desc: kinda old function
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RenderCM(CollisionModel* cmIn)
{
	D3DXMATRIX matWorld;
	D3DXMatrixTranslation(&matWorld, 0,0,0);
	m_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);


	D3DXVECTOR3 pVert[24];
	VOID* pBV;



	// 1-2
	pVert[0] = cmIn->vMax;
	pVert[1] = cmIn->vMax; pVert[1].z = cmIn->vMin.z;
	// 2-3
	pVert[2] = cmIn->vMax; pVert[2].z = cmIn->vMin.z;
	pVert[3] = pVert[2]; pVert[3].y = cmIn->vMin.y;
	// 3-4
	pVert[4] = pVert[2]; pVert[4].y = cmIn->vMin.y;
	pVert[5] = cmIn->vMax; pVert[5].y = cmIn->vMin.y;
	// 4-1
	pVert[6] = cmIn->vMax; pVert[6].y = cmIn->vMin.y;
	pVert[7] = cmIn->vMax;

	// 7-8
	pVert[8] = cmIn->vMin;
	pVert[9] = cmIn->vMin; pVert[9].z = cmIn->vMax.z;
	// 8-5
	pVert[10] = cmIn->vMin; pVert[10].z = cmIn->vMax.z;
	pVert[11] = pVert[9]; pVert[11].y = cmIn->vMax.y;
	// 5-6
	pVert[12] = pVert[10]; pVert[12].y = cmIn->vMax.y;
	pVert[13] = cmIn->vMin; pVert[13].y = cmIn->vMax.y;
	// 6-7
	pVert[14] = cmIn->vMin; pVert[14].y = cmIn->vMax.y;
	pVert[15] = cmIn->vMin;


	//1-5
	pVert[16] = cmIn->vMax;
	pVert[17] = pVert[10]; pVert[17].y = cmIn->vMax.y;
	//2-6
	pVert[18] = cmIn->vMax; pVert[18].z = cmIn->vMin.z;
	pVert[19] = cmIn->vMin; pVert[19].y = cmIn->vMax.y;
	//3-7
	pVert[20] = pVert[2]; pVert[20].y = cmIn->vMin.y;
	pVert[21] = cmIn->vMin;
	//4-8
	pVert[22] = cmIn->vMax; pVert[22].y = cmIn->vMin.y;
	pVert[23] = cmIn->vMin; pVert[23].z = cmIn->vMax.z;



		//initializing
		if( FAILED( pBoundingBoxVertBuffer->Lock( 0, sizeof(pVert), (void**)&pBV, 0 ) ) )  return E_FAIL;
		memcpy( pBV, pVert, sizeof(pVert) );
		pBoundingBoxVertBuffer->Unlock();

		//drawing
		m_pd3dDevice->SetStreamSource( 0, pBoundingBoxVertBuffer, 0, sizeof(D3DXVECTOR3) );
		m_pd3dDevice->SetFVF( D3DFVF_XYZ );
		m_pd3dDevice->DrawPrimitive( D3DPT_LINELIST, 0, 12 );




	return S_OK;
}
*/


//-----------------------------------------------------------------------------
// Name: MenuCallback
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::MenuCallback()
{

	//main menu
	if(curmenu == MAINMENU)
	{
		if(selection == 0)
		{
			//newgame
			DXUtil_Timer( TIMER_START );
			bMenu = FALSE;
		}
		else if(selection == 1)
		{
			//savegame

		}
		else if(selection == 2)
		{
			//loadgame

		}
		else if(selection == 3)
		{
			//options

		}
		else if(selection == 4)
		{
			//quit
			DeleteDeviceObjects();
			FinalCleanup();
			exit(0);
			//SendMessage( m_hWnd, WM_CLOSE, 0, 0 );
		}
	}


	return S_OK;
}



#define SELECTIONCHECK(num) if(selection == num) menuFont = D3DCOLOR_ARGB(255,255,0,0); else menuFont=D3DCOLOR_ARGB(255,255,255,255)
//-----------------------------------------------------------------------------
// Name: MenuRender
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::MenuRender()
{
	D3DCOLOR menuFont;//white
	TCHAR szMsg[MAX_PATH] = TEXT("");

	FLOAT fNextLine = SCREENHEIGHT/3*2; 


	if(curmenu == MAINMENU)
	{
		SELECTIONCHECK(0);
		//lstrcpy( szMsg, L"NEW GAME" );
		m_pFont->DrawText(SCREENWIDTH* 3/5, fNextLine, menuFont, (unsigned char*)"NEW GAME");

		SELECTIONCHECK(1);
		lstrcpy( szMsg, L"LOAD GAME" );
		fNextLine += 40.0f;
		m_pFont->DrawText(SCREENWIDTH* 3/5, fNextLine, menuFont, (unsigned char*)"LOAD GAME" );

		SELECTIONCHECK(2);
		lstrcpy( szMsg, L"SAVE GAME" );
		fNextLine += 40.0f;
		m_pFont->DrawText(SCREENWIDTH* 3/5, fNextLine, menuFont, (unsigned char*)"SAVE GAME" );

		SELECTIONCHECK(3);
		lstrcpy( szMsg, L"OPTIONS" );
		fNextLine += 40.0f;
		m_pFont->DrawText(SCREENWIDTH* 3/5, fNextLine, menuFont, (unsigned char*)"OPTIONS" );

		SELECTIONCHECK(4);
		lstrcpy( szMsg, L"QUIT" );
		fNextLine += 40.0f;
		m_pFont->DrawText(SCREENWIDTH* 3/5, fNextLine, menuFont, (unsigned char*)"QUIT" );
	}


	return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RenderText()
// Desc: Renders stats and help text to the scene.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RenderText()
{
    D3DCOLOR fontColor        = D3DCOLOR_ARGB(255,255,255,255); //yellow
	TCHAR szMsg[MAX_PATH] = TEXT("");
	int i,j;
	FLOAT fNextLine;


		// FPS
		fNextLine = 20.0f;
		sprintf(logstr, "FPS: %f", m_fFPS);
		m_pFont->DrawText( m_d3dsdBackBuffer.Width*0.92f, fNextLine, fontColor, (unsigned char*)logstr );


	if(bMenu)	MenuRender();

	else if(bConsole)
	{
		D3DRECT rtx3d;
		rtx3d.x1 = 0;
		rtx3d.y1 = 0;
		rtx3d.x2 = m_d3dsdBackBuffer.Width;
		rtx3d.y2 = m_d3dsdBackBuffer.Height * 0.2f;
		m_pd3dDevice->Clear( 1, &rtx3d, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0f, 0L );//black
	

		// Console
		fNextLine = m_d3dsdBackBuffer.Height * 0.2f - 23.0f; 

		m_pFont->DrawText( 5, fNextLine, fontColor, (unsigned char*)consoleStr );
		fNextLine -= 15;

		for( i = 0; i < CONSOLEHISTORYNUM && fNextLine > 1; i++)
		{
			j = consoleHistoryCur - i;
			if( j < 0 ) j += CONSOLEHISTORYNUM;
			m_pFont->DrawText( 5, fNextLine, fontColor, (unsigned char*)consoleHistory[j] );
			fNextLine -= 15;
		}
	
		// Developer information
		fNextLine = (FLOAT) m_d3dsdBackBuffer.Height; 

		sprintf(logstr, "angle - %f", SkinMesh[0]->angle.y );
		fNextLine -= 20.0f; m_pFont->DrawText( 2, fNextLine, fontColor, (unsigned char*)logstr );

		sprintf(logstr, "WORLD TIME:  %d:%d", (int)(WorldTime), (int)( 60.f*(WorldTime - (float)((int)WorldTime))) );
		fNextLine -= 20.0f; m_pFont->DrawText( 2, fNextLine, fontColor, (unsigned char*)logstr );

		sprintf(logstr, "%d %d %d", (int)SkinMesh[0]->speed.x, (int)SkinMesh[0]->speed.y, (int)SkinMesh[0]->speed.z);
		fNextLine -= 20.0f; m_pFont->DrawText( 2, fNextLine, fontColor, (unsigned char*)logstr );

		sprintf(logstr, "%d, %d, %d", (int)SkinMesh[0]->coordinate.x, (int)SkinMesh[0]->coordinate.y, (int)SkinMesh[0]->coordinate.z);
		fNextLine -= 20.0f; m_pFont->DrawText( 2, fNextLine, fontColor, (unsigned char*)logstr );

		sprintf(logstr, "once - %d; per - %d, animtime - %f, asn - %s, sasn - %s",
			(int)SkinMesh[0]->OnceAnimation(), (int)SkinMesh[0]->ASI.bAnimPeriodPassed, (float)SkinMesh[0]->ASI.AnimTime, SkinMesh[0]->ASI.AnimSetName, SkinMesh[0]->ASI.StockAnimationSet);
		fNextLine -= 20.0f; m_pFont->DrawText( 2, fNextLine, fontColor, (unsigned char*)logstr );

		sprintf(logstr, "%f", (float)(SkinMesh[0]->GetAnimControllerTime()));
		fNextLine -= 20.0f; m_pFont->DrawText( 2, fNextLine, fontColor, (unsigned char*)logstr );

		sprintf(logstr, "%d %d", (int)terrain[0].GetNByXZ( SkinMesh[0]->coordinate.x, SkinMesh[0]->coordinate.z).x,
								 (int)terrain[0].GetNByXZ( SkinMesh[0]->coordinate.x, SkinMesh[0]->coordinate.z).y);
		fNextLine -= 20.0f; m_pFont->DrawText( 2, fNextLine, fontColor, (unsigned char*)logstr );	
	}



	// Chat
	else
	{
		fNextLine = m_d3dsdBackBuffer.Height - 23.0f; 

		m_pFont->DrawText( m_d3dsdBackBuffer.Width*0.7f, fNextLine, fontColor, (unsigned char*)chatStr );
		fNextLine -= 15;

		for( i = 0; i < 15 && fNextLine > 1; i++)
		{
			j = chatHistoryCur - i;
			if( j < 0 ) j += CHATHISTORYNUM;
			m_pFont->DrawText( m_d3dsdBackBuffer.Width*0.7f, fNextLine, fontColor, (unsigned char*)chatHistory[j] );
			fNextLine -= 15;
		}
	}


    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: Overrrides the main WndProc, so the sample can do custom message
//       handling (e.g. processing mouse, keyboard, or menu commands).
//-----------------------------------------------------------------------------
LRESULT CMyD3DApplication::MsgProc( HWND hWnd,
								    UINT msg,
									WPARAM wParam,
                                    LPARAM lParam )
{
    switch( msg )
    {
        case WM_PAINT:
        {
            if( m_bLoadingApp )
            {
                // Draw on the window tell the user that the app is loading
                // TODO: change as needed
                HDC hDC = GetDC( hWnd );
                TCHAR strMsg[MAX_PATH];
                wsprintf( strMsg, TEXT("...Loading...") );
                RECT rct;
                GetClientRect( hWnd, &rct );
                DrawText( hDC, strMsg, -1, &rct, DT_CENTER|DT_VCENTER|DT_SINGLELINE );
                ReleaseDC( hWnd, hDC );
            }
            break;
        }


		//console & chat processing
		case WM_CHAR:
		{

			//for console
			if(bConsole)
			{
				if( consoleSymNum < 255 )
				{
					char cKey = (char)wParam;
					//HTMLLog("%c - %d\n", cKey, (int)cKey);

					if( cKey != '`' && 
						(cKey >= 32 || ( cKey < 0 && cKey > -65)) )
					{
						consoleStr[consoleSymNum] = cKey;
						consoleStr[consoleSymNum+1] = '\0';
						consoleSymNum++;
					}
					else if(cKey == 8 && consoleSymNum >= 1)
					{
						consoleSymNum--;
						consoleStr[consoleSymNum] = '\0';
					}
				}
			}


			//for chat
			else if(bChat)
			{
				if( chatSymNum < 255 )
				{
					char cKey = (char)wParam;
					//HTMLLog("%c - %d\n", cKey, (int)cKey);

					if( cKey != '`' && 
						(cKey >= 32 || ( cKey < 0 && cKey > -65)) )
					{
						chatStr[chatSymNum] = cKey;
						chatStr[chatSymNum+1] = '\0';
						chatSymNum++;
					}
					else if(cKey == 8 && chatSymNum >= 1)
					{
						chatSymNum--;
						chatStr[chatSymNum] = '\0';
					}
				}
			}

		}

		case WM_KEYDOWN:
		{

		}

		case WM_INPUTLANGCHANGE:
		{
			//TCHAR layoutName[KL_NAMELENGTH];
			//GetKeyboardLayoutName(layoutName);
			//HTMLLog("%s\n", (char*)layoutName);

			//LoadKeyboardLayout( K_LAYOUT_RUS, KLF_ACTIVATE ); 
		}


    break;


    }

    return CD3DApplication::MsgProc( hWnd, msg, wParam, lParam );
}




//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: Invalidates device objects.  Paired with RestoreDeviceObjects()
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InvalidateDeviceObjects()
{
    // TODO: Cleanup any objects created in RestoreDeviceObjects()
    m_pFont->InvalidateDeviceObjects();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DeleteDeviceObjects()
// Desc: Paired with InitDeviceObjects()
//       Called when the app is exiting, or the device is being changed,
//       this function deletes any device dependent objects.  
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::DeleteDeviceObjects()
{
	int i;

	LogPlease("\n\n-= Ending in-game Log =-\n\n");
	LogPlease("\n\n-= Deleting device objects =-\n");

	HTMLLog("...releasing physx... ");
	if(SUCCEEDED(ReleaseNx())) HTMLLog("... success!\n");


	LogPlease("...deleting GUI...\n");
	for( i=0; i < NumOfPics; i++) pic[i].Release();



	LogPlease("...delete textable for terrain...\n");
	texTable->Release();


	LogPlease("...delete terrain...\n");
	for(i=0; i < NumOfCTerrain; i++)
	{
		terrain[i].Release();
	}


	LogPlease("...delete SkinMeshes and Heads...\n");
    for(i=0; i < NumOfSkinMesh; i++)
	{
		SkinMesh[i]->Release();
		Heads[i]->Release();
	}

	LogPlease("...delete Animations...\n");
	for(i=0; i < NumOfAnimations; i++)
	{
		Animations[i]->Release();
	}

	LogPlease("...delete objects...\n");
    for(i=0; i < NumOfObjects; i++)
	{
		Objects[i]->Release();
	}


	LogPlease("...delete weapons...\n");
	for(i=0; i < NumOfWeapons; i++)
	{
		Weapons[i]->Release();
	}


	LogPlease("...delete armor...\n");
	for(i=0; i < NumOfArmor; i++)
	{
		Armor[i]->Release();
	}


	LogPlease("...delete particles...\n");
	for(i=0; i < NumOfParticle; i++)
	{
		ParticleSystem[i].Release();
	}


	LogPlease("...delete decals...\n");
	for(i=0; i < NumOfDecals; i++)
	{
		Decals[i].Release();
	}



	free(dirlights);


	LogPlease("...uninitialize direct input...\n");
	input->DeleteKeyboard();
	input->DeleteMouse();
	input->DeleteInput();
	free(input);
	input = NULL;

	
	skybox->Release();
	water->Release();


    m_pFont->DeleteDeviceObjects();


	m_pMusicManager->StopAll();


	m_pEffect->Release();


	//shadow map release
	LogPlease("...release shadowmap surfaces...\n");
	SAFE_RELEASE( g_pShadowMap );
	SAFE_RELEASE( g_pShadowSurf );
	SAFE_RELEASE( g_pShadowDepth );
	SAFE_RELEASE( g_pOldColorRT );
	SAFE_RELEASE( g_pOldDepthRT );



	//engine stuff release
	//pBoundingBoxVertBuffer->Release();
	if( en_box ) en_box->Release();


	//release physx
	LogPlease("...release PhysX...\n");
	if (gScene) gPhysicsSDK->releaseScene(*gScene);
	if (gPhysicsSDK)  NxReleasePhysicsSDK(gPhysicsSDK);

	LogPlease("-= DeleteDeviceObjects completely succeeded!\n");

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FinalCleanup()
// Desc: Paired with OneTimeSceneInit()
//       Called before the app exits, this function gives the app the chance
//       to cleanup after itself.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::FinalCleanup()
{
    // TODO: Perform any final cleanup needed
    // Cleanup D3D font
    SAFE_DELETE( m_pFont );

    // Cleanup DirectX audio objects
    //SAFE_DELETE( m_pSound );
    SAFE_DELETE( m_pMusicManager );

	m_pBackgroundMusic->Release();


    return S_OK;
}




