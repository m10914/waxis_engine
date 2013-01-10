//-----------------------------------------------------------------------------
// File: SkinnedMesh.h
//
// Desc: Header file SkinnedMesh sample app
//-----------------------------------------------------------------------------
#pragma once


//base
#define STRICT
#define NOMINMAX  //this somehow connected to PhysX

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <basetsd.h>
#include <math.h>
#include <stdio.h>
#include <d3dx9.h>
#include <dxerr9.h>
#include <dshow.h>
#include <tchar.h>
#include "DXUtil.h"
#include "D3DEnumeration.h"
#include "D3DSettings.h"
#include "D3DApp.h"
#include "D3DFont.h"
#include "D3DFile.h"
#include "D3DUtil.h"
#include "DMUtil.h"
#include "resource.h"
#include <dxfile.h>


//mine
#include "input.h"
#include "MyModel.h"
#include "ccamera.h"
#include "terrain.h"
#include "SkyBox.h"
#include "particle.h"
#include "decal.h"
#include "cpic.h"
#include "Mp3MusicManager.h"
#include "Water.h"


//physx)
#include "UserAllocator.h"
//#include "UserData.h"
#include "Stream.h"
//#include "Actors.h"
#include "cooking.h"
#include "Joints.h"
#include "NxPhysics.h"
#include "NxControllerManager.h"
#include "MyCharacterController.h"
#include "ContactReport.h"
#include "ErrorStream.h"
#include "PerfRenderer.h"
#include "Utilities.h"
#include "SamplesVRDSettings.h"

//STL
#include <vector>
#include <list>
#include <iostream>
using namespace std; //for STL to get it work


//adds logging to render function
//#define TESTMODE

//-----------------------------------------------------------------------------
// Defines, and constants
//-----------------------------------------------------------------------------
// TODO: change "DirectX AppWizard Apps" to your name or the company name
#define DXAPP_KEY        TEXT("Mephisto studios' WAXIS engine")




struct DIRLIGHT
{
	D3DXVECTOR3 Pos;
	D3DXVECTOR3 Aim;
};



//for shadow maps
#define SHADOWMAPSIZE 3000


//for console & chat
#define CONSOLEHISTORYNUM 12
#define CHATHISTORYNUM 50


struct POLYGON
{
	D3DXVECTOR3 v[3];
};

struct QUAD
{
	D3DXVECTOR3 v[4];
};


#define DISTANCE(type) D3DXVec3Length(&(type[i]->coordinate - camera->util.GetEyePt()))

#define ONCEANIMATIONPERIOD (SkinMesh[i]->OnceAnimation()==TRUE && SkinMesh[i]->ASI.bAnimPeriodPassed==FALSE)
#define ONCEANIMATIONNOSTRIKEPERIOD (SkinMesh[i]->OnceAnimationNoStrikes()==TRUE && SkinMesh[i]->ASI.bAnimPeriodPassed==FALSE)
#define ONCEANIMATIONHURTPERIOD (SkinMesh[i]->OnceAnimationHurt()==TRUE && SkinMesh[i]->ASI.bAnimPeriodPassed==FALSE)


//console macros
#define AddMessageToConsole(str) consoleHistoryCur++; if( consoleHistoryCur >= CONSOLEHISTORYNUM ) consoleHistoryCur -= CONSOLEHISTORYNUM; strcpy( consoleHistory[consoleHistoryCur], str);
#define AddMessageToChat(str) chatHistoryCur++; if( chatHistoryCur >= CHATHISTORYNUM ) chatHistoryCur -= CHATHISTORYNUM; strcpy( chatHistory[chatHistoryCur], str);


//menu define
#define MAINMENU 1
#define SAVEGAMEMENU 2
#define LOADGAMEMENU 3
#define OPTIONSMENU 4


//class gopoclass{ public: int a,b,c; gopoclass(int, int, int); };


//-----------------------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: Application class. The base class (CD3DApplication) provides the 
//       generic functionality needed in all Direct3D samples. CMyD3DApplication 
//       adds functionality specific to this sample program.
//-----------------------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{

	BOOL                    m_bLoadingApp;          // TRUE, if the app is loading
	BOOL					m_bOneTimeRender;
	

	BOOL					bShadowsEnable;
	BOOL					bBoundingBoxes;

	//engine constants
	int						VIEWDISTANCE_OBJECT;
	int						VIEWDISTANCE_TERRAIN;

	int						SCREENWIDTH;
	int						SCREENHEIGHT;

	//timing
	float					WorldTime;
	float					TimeQ;

	//lock stuff
	float					lockTime;
	float					itemTime;


	// console
	BOOL					bConsole;
    double					consoleTime;
	char					consoleStr[256];
	char					consoleSymNum;
	char					consoleHistory[ CONSOLEHISTORYNUM ][256];
	char					consoleHistoryCur;

	// chat
	BOOL					bChat;
    double					chatTime;
	char					chatStr[256];
	char					chatSymNum;
	char					chatHistory[ CHATHISTORYNUM ][256];
	char					chatHistoryCur;

	// very important stuff
	double					EnterTime;

	

	// Font for drawing text
	CD3DFont*               m_pFont;                
	char					logstr[256];


	//menu
	BOOL					bMenu;
	double					menuTime;
	int						curmenu;
	int						selection;
	BOOL					bMenuLMB;


	//sound - directmusic
	FLOAT                   m_fSoundPlayRepeatCountdown; // Sound repeat timer
    CMusicManager*          m_pMusicManager;			 // DirectMusic manager class
    //C3DMusicSegment*          m_pSound;

	//sound - directshow
	CMp3MusicManager*		m_pBackgroundMusic;
	

	//skybox
	CSkyBox*				skybox;


	//water
	CWater*					water;


	//----- MODELS
	// types:
	// 1. Animations
	// 2. Skinned Meshes
	// 3. Objects
	// 4. Weapons
	// 5. Armor

	int							NumOfAnimations;
	vector<CMyModel*>			Animations;

	int							NumOfSkinMesh;
	vector<CMyModel*>			SkinMesh;
	vector<CModel*>				Heads;

	int							NumOfObjects;	
	vector<CModel*>				Objects;

	int							NumOfWeapons;
	vector<CModel*>				Weapons;

	int							NumOfArmor;
	vector<CModel*>				Armor;

	//-----


	//terrain
	int							NumOfCTerrain;
	CTerrain*					terrain;
	CTextureTable*				texTable;


	//camera
	CCamera*					camera;
	
	
	//dinput
	Input*						input;	


	//particles
	int							NumOfParticle;
	CParticleSystemParent*		ParticleSystem;

	
	//decals
	int							NumOfDecals;
	CDecal*						Decals;


	//effects
	LPD3DXEFFECT				m_pEffect;


	//lights
	DIRLIGHT*					dirlights;
	int							NumOfLights;

	DWORD						EnvColor;


	//shadow map stuff
	
	LPDIRECT3DTEXTURE9			g_pShadowMap;
	LPDIRECT3DSURFACE9			g_pShadowSurf;
	LPDIRECT3DSURFACE9			g_pShadowDepth;

	LPDIRECT3DSURFACE9			g_pNewDepthRT;
	LPDIRECT3DSURFACE9			g_pOldColorRT;
	LPDIRECT3DSURFACE9			g_pOldDepthRT;



	//menu stuff
	int							NumOfPics;
	CPic*						pic;


	//engine stuff
	//LPDIRECT3DVERTEXBUFFER9		pBoundingBoxVertBuffer;
	LPD3DXMESH					en_box;




	//----- PHYSX STUFF -----
	//

	NxPhysicsSDK*			gPhysicsSDK;
	NxScene*				gScene;
	UserAllocator*			gAllocator;

	NxControllerManager*	gManager;

	HRESULT			InitNx();
	HRESULT			StartNx();
	HRESULT			FetchNx();
	HRESULT			ReleaseNx();


	//
	//-----------------------


protected:
    virtual HRESULT OneTimeSceneInit();
    virtual HRESULT InitDeviceObjects();
    virtual HRESULT RestoreDeviceObjects();
    virtual HRESULT InvalidateDeviceObjects();
    virtual HRESULT DeleteDeviceObjects();
    virtual HRESULT Render();
    virtual HRESULT FrameMove();
    virtual HRESULT FinalCleanup();
    virtual HRESULT ConfirmDevice( D3DCAPS9*, DWORD, D3DFORMAT );



    HRESULT RenderText();
	HRESULT RenderPhysXBox(NxActor* object);
	HRESULT RenderPhysXCapsule(NxActor* object);
	//HRESULT RenderCM(CollisionModel* cmIn);


	//---- COLLISION DETECTION ----

	D3DXVECTOR3 BoxVsBoxCollision( CollisionModel* cm1, CollisionModel* cm2 ); //returns offsets of dx, dy, dz
	POLYGON*	BoxVsPolygon( CollisionModel* cm, POLYGON* pl );

	//---------

	HRESULT AddActor(int inAnimType, char* inMeshPath, char* inHeadMeshPath, D3DXVECTOR3 inCoord);
	HRESULT AddDecal( CollisionModel* cm, CTerrain* terr );


	//menu stuff
	HRESULT MenuCallback();
	HRESULT MenuRender();


	//process stuff
	int GetCModelIndexByID( vector<CModel*> cm, int NumOfcm, int id );
	HRESULT ProcessConsoleMsg();
	HRESULT ProcessChatMsg();


	//audio
    HRESULT InitAudio( HWND hWnd );


	//effect
	HRESULT	InitEffect();
	HRESULT UpdateEffectVariables();



	//Callbacks
	HRESULT	DInputCallback();
	HRESULT AICallback(CModel*);


	//ProcessMovement - based on animation
	HRESULT ProcessMovement(CModel*);



public:
    LRESULT MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
    CMyD3DApplication();
    virtual ~CMyD3DApplication();
};
