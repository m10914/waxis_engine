//------------------------------------------------------
// CLASS CMODEL (-HEADER-)
// Desc: using for loading and processing
//		 skinned meshes into program
//------------------------------------------------------

#define NOMINMAX  //this somehow connected to PhysX
#define CMODEL_H
#include <windows.h>
#include <commdlg.h>
#include <tchar.h>
#include <stdio.h>
#include <d3dx9.h>
#include "DXUtil.h"
#include "D3DEnumeration.h"
#include "D3DSettings.h"
#include "D3DApp.h"
#include "D3DFont.h"
#include "D3DUtil.h"

//physics
#include "NxPhysics.h"
#include "UserAllocator.h"
//#include "Actors.h"
#include "MyCharacterController.h"
//#include "ContactReport.h"
#include "Stream.h"
#include "Joints.h"
#include "cooking.h"
#include "ErrorStream.h"
//#include "PerfRenderer.h"
//#include "Utilities.h"
//#include "SamplesVRDSettings.h"


#include "log.h"
#include "mmath.h"



#define PF_CONVEX 0
#define PF_BOX 1


#define FT_DOL 0
#define FT_SOL 1
#define FT_COL 2
#define FT_NONE 3
#define FT_CHARACTER 4



#define REVOLUTE 0
#define SPHERICAL 1

//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
struct CollisionModel
{
	D3DXVECTOR3 vMax, vMin;
};


//-----------------------------------------------------------------------------
// Name: struct D3DXFRAME_DERIVED
// Desc: Structure derived from D3DXFRAME so we can add some app-specific
//       info that will be stored with each frame
//-----------------------------------------------------------------------------
struct D3DXFRAME_DERIVED: public D3DXFRAME
{
    D3DXMATRIXA16        CombinedTransformationMatrix;
};





//-----------------------------------------------------------------------------
// Name: struct D3DXMESHCONTAINER_DERIVED
// Desc: Structure derived from D3DXMESHCONTAINER so we can add some app-specific
//       info that will be stored with each mesh
//-----------------------------------------------------------------------------
struct D3DXMESHCONTAINER_DERIVED: public D3DXMESHCONTAINER
{
    LPDIRECT3DTEXTURE9*  ppTextures;       // array of textures, entries are NULL if no texture specified  
	LPDIRECT3DTEXTURE9*  ppBumpTextures;
	                            
    // SkinMesh info             
    LPD3DXMESH           pOrigMesh;
    LPD3DXATTRIBUTERANGE pAttributeTable;
    DWORD                NumAttributeGroups; 
    DWORD                NumInfl;
    LPD3DXBUFFER         pBoneCombinationBuf;
    D3DXMATRIX**         ppBoneMatrixPtrs;
    D3DXMATRIX*          pBoneOffsetMatrices;
    DWORD                NumPaletteEntries;
    bool                 UseSoftwareVP;
    DWORD                iAttributeSW;     // used to denote the split between SW and HW if necessary for non-indexed skinning

	//+!!!
	LPD3DXBUFFER		pVertexRemap;

	//collision
	CollisionModel*		cm;
	CollisionModel*		cmUpdated;

};




HRESULT AllocateName( LPCSTR Name, LPTSTR *pNewName );


class CModel;




struct AnimSetInfo
{
	//----------------------------
	// pass animation stuff 
	BOOL						ap_bIsAnimPass;
	BOOL						bAnimPeriodPassed;

	double						ap_BreakTime; //time of set1, when it was interrupted
	double						ap_StartTime; //absolute start time
	double						ap_CurTime;	  //absolute curtime
	
	DWORD						ap_NumOfSet1;		// number of first animset
	DWORD						ap_NumOfSet2;		// number of second animset
	
	double						ap_fTime;			//full time (length of passing)
	
	//+simple animation stuff
	char*						AnimSetName;
	double						AnimTime;

	//+additional animation stuff
	char*						StockAnimationSet;
	float						StockTime;
	//----------------------------
};
HRESULT LogAnimSetInfo(AnimSetInfo* asInfo);



//-----------------------------------------------------------------------------
// Name: class CAllocateHierarchy
// Desc: Custom version of ID3DXAllocateHierarchy with custom methods to create
//       frames and meshcontainers.
//-----------------------------------------------------------------------------
/*
class CAllocateHierarchy: public ID3DXAllocateHierarchy
{
public:
    STDMETHOD(CreateFrame)(THIS_ LPCSTR Name, LPD3DXFRAME *ppNewFrame);
    STDMETHOD(CreateMeshContainer)(THIS_ LPCSTR Name, LPD3DXMESHDATA pMeshData,
                            LPD3DXMATERIAL pMaterials, LPD3DXEFFECTINSTANCE pEffectInstances, DWORD NumMaterials, 
                            DWORD *pAdjacency, LPD3DXSKININFO pSkinInfo, 
                            LPD3DXMESHCONTAINER *ppNewMeshContainer);
    STDMETHOD(DestroyFrame)(THIS_ LPD3DXFRAME pFrameToFree);
    STDMETHOD(DestroyMeshContainer)(THIS_ LPD3DXMESHCONTAINER pMeshContainerBase);
    CAllocateHierarchy(CModel *cm) :cMod(cm) {}

public:
    CModel* cMod;
};
*/
class CAllocateHierarchy: public ID3DXAllocateHierarchy
{
public:
    STDMETHOD(CreateFrame)(THIS_ LPCSTR Name, LPD3DXFRAME *ppNewFrame);
    STDMETHOD(CreateMeshContainer)(THIS_ 
        LPCSTR Name, 
        CONST D3DXMESHDATA *pMeshData,
        CONST D3DXMATERIAL *pMaterials, 
        CONST D3DXEFFECTINSTANCE *pEffectInstances, 
        DWORD NumMaterials, 
        CONST DWORD *pAdjacency, 
        LPD3DXSKININFO pSkinInfo, 
        LPD3DXMESHCONTAINER *ppNewMeshContainer);
    STDMETHOD(DestroyFrame)(THIS_ LPD3DXFRAME pFrameToFree);
    STDMETHOD(DestroyMeshContainer)(THIS_ LPD3DXMESHCONTAINER pMeshContainerBase);
    CAllocateHierarchy(CModel *cm) { cMod = cm; }

public:
    CModel* cMod;
};

//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
#define ORDINARY 0
#define SKINNED 1


#define BASE 0
#define CLONE 1


//messages to CModel
#define DONTDRAWTEXTURES 1
#define DONTDRAWBUMP 2


#define OT_CHARACTER 0
#define OT_OBJECT 1
#define OT_WEAPON 2

class CModel
{

protected:
	LPDIRECT3DDEVICE9			m_pd3dDevice;
	D3DCAPS9					m_d3dCaps;


	D3DXMATRIXA16               matWorld;
	D3DXMATRIXA16               matRotation;


	LPD3DXFRAME                 m_pFrameRoot;
    LPD3DXANIMATIONCONTROLLER   m_pAnimController;

	D3DXVECTOR3                 m_vObjectCenter;    // Center of bounding sphere of object
    FLOAT                       m_fObjectRadius;    // Radius of bounding sphere of object
	

	//for skinned
	int							numBones;
	int							numFrames;
	

	//for ordinary
	UINT						numOfCm;
	UINT						curNum;
	UINT						seekNum;
	D3DXMATRIX*					pCurMatrix;



	//----------------------------
	// bone stuff
    D3DXMATRIXA16*              m_pBoneMatrices;

	D3DXMATRIXA16*				m_pBoneMatricesLocal;
	CHAR**						m_pBoneMatricesLocalNames;
    UINT                        m_NumBoneMatricesMax;
	//----------------------------
	


	//----------------------------
	//shader stuff
	LPD3DXEFFECT				m_pEffect;
	D3DXMATRIX					m_matView;
	D3DXMATRIX					m_matProj;
	//----------------------------


public:
	
	//united
	AnimSetInfo					ASI;
	AnimSetInfo					asinfo;	//backuper
	//----------------------------


	//path to mesh
	char*						meshPath;

	//messages from source to cmodel
	char						msg;
	int							frameCounter;
	


	//classification
	int							id;
	BOOL						dublicate;
	char						type;		//skinned or ordinary

	char						MType;		//base or clone
	char						MAnimType;	//predefined animtypes in .lvl
	CModel*						ParentModel;//if(MType == CLONE), then it's useful



	D3DXVECTOR3					coordinate;
	D3DXVECTOR3					angle;
	float						height;



	//----------------------------
	
	// for ordinary meshes - attach
	BOOL						bAttached;
	int							attacherId;
	D3DXMATRIX*					a_TransformMatrix;
	D3DXMATRIX					CurrentAttachmentTransformMatrix;
	
	// for skinned meshes - attach mirror
	BOOL						bAttacher;
	int							attachedId;

	//----------------------------



	//----- physX
	// just test for now - physx will be much more complicated

	NxPhysicsSDK*			mPhysicsSDK;
	NxScene*				mScene;
	NxControllerManager*	mManager;

	//character controller variables
	MyCharacterController*  controller;

	//single
	NxActor*				object;
	D3DXMATRIX*				objectMatrix;

	//biped
	int						NumOfBiped;
	int						CurBiped;
	NxActor**				biped;
	D3DXMATRIX**			bipedMatrix;
	char**					bipedNames;

	int						NumOfJoint;
	int						CurJoint;
	NxRevoluteJoint**		joint;
	char**					jointNames;


	char					physxtype;
	CModelUserData			usrdata;
	BOOL					bFree;

	//functions
	HRESULT						InitPhysX( NxPhysicsSDK* gPhysicsSDK, NxScene* gScene, NxControllerManager*	gManager, char flag);

	void						CreatePhysXBipedObject(void);
	void						CreatePhysXObject(void);
	void						CreatePhysXObjectFrame( LPD3DXFRAME pFrame, NxActorDesc* ActorDesc);
	void						CreatePhysXBipedObjectFrame( LPD3DXFRAME pFrame );
	void						CreatePhysXJointFrame( LPD3DXFRAME pFrame );
	char						ScanFramesForKeywords( LPD3DXFRAME pFrame );
	int							GetNumBipeds( LPD3DXFRAME pFrame );
	int							GetNumJoints( LPD3DXFRAME pFrame );
	NxActor*					GetBipedByName( char* name );
	HRESULT						SetFree( bool bFreeFlag = TRUE );
	void						CreatePhysXObjectBoxMeshContainer( LPD3DXMESHCONTAINER pMeshContainerBase, LPD3DXFRAME pFrameBase, NxActorDesc* ActorDesc );
	void						CreatePhysXBipedObjectMeshContainer( LPD3DXMESHCONTAINER pMeshContainerBase, LPD3DXFRAME pFrameBase );
	void						CreatePhysXJointMeshContainer( LPD3DXMESHCONTAINER pMeshContainerBase, LPD3DXFRAME pFrameBase );

	HRESULT						UpdateMatricesPhysX( NxMat34* outmatrix = NULL, NxVec3* invector = NULL );
	HRESULT						UpdateBipedsPhysX();
	int							UpdateBipedsPhysXByFrame( LPD3DXFRAME pFrame, int parentindex, BOOL firsttime = TRUE );
	int							GetBipedIndexByName(char* name);

	float						GetHeight();

//functions
public:
	CModel();

	//--------------------------------------------------
	//initialization
	HRESULT						Init(LPDIRECT3DDEVICE9, D3DCAPS9, LPCWSTR, LPD3DXEFFECT, CModel*);
	HRESULT						GenerateSkinnedMesh( D3DXMESHCONTAINER_DERIVED *pMeshContainer);
	//--------------------------------------------------

	
	//--------------------------------------------------
	//FrameMove
	int							GetFrameIndexByName(char* name);
	int							GetNumOfFrames(LPD3DXFRAME pFrame);
	LPD3DXFRAME					GetFrameByNum(LPD3DXFRAME pFrame, int Num, BOOL firsttime = TRUE);

	LPD3DXANIMATIONCONTROLLER	GetAnimController(void);
	char*						GetTrackAnimationSetName(int);
	HRESULT						SetTrackAnimationSetByName(LPCSTR, int TrackNum, BOOL TrackEnable, float TrackSpeed,
														   float TrackWeight, float BlendPriority);
	HRESULT						PassAnimation(LPCSTR, double fTime);
	HRESULT						SetStockAnimationSetByName(char* name, float time);
	HRESULT						PassAnimationStock();

	//updating functions
	HRESULT						CopyAnimationState(CModel* modelIn); // ONLY for meshes with the same bones and animations
	HRESULT						CopyBonesState(CModel* modelIn, D3DXMATRIX, D3DXMATRIX);	//just copying bones matrices by name
	HRESULT						StoreBoneMatrices(CModel* modelIn);
	HRESULT						StoreFrameMatrices(CModel* modelIn);

	//for skeletal clones
	HRESULT						GetModelState(CModel* modelIn);  //GetModelState - means that CModel gets ASI from modelIn
	HRESULT						SetModelState(CModel* modelOut); //SetModelState - means CModel sets modelIn's ASI to its
	HRESULT						GetModelState(AnimSetInfo* animInfoIn);
	HRESULT						SetModelState(AnimSetInfo* animInfoOut);

	//simple updating
	HRESULT						Update(double);
	HRESULT						UpdateMatrices(D3DXMATRIX matView, D3DXMATRIX matProj);
	void						UpdateAttachmentMatrix();


	//bools
	BOOL						IsAnimPass();
	virtual BOOL				OnceAnimation() { return FALSE; }
	
	//attachment
	HRESULT						Attach(CModel*,LPCSTR);
	HRESULT						Detach();
	
	D3DXMATRIXA16*				GetMatWorld();
	HRESULT						SetMatWorld( D3DXMATRIXA16* MatWorldIn );

	HRESULT						SetCoordinate(D3DXVECTOR3);
	HRESULT						SetAngle(D3DXVECTOR3);

	HRESULT						SetAnimControllerTime(double);
	double						GetAnimControllerTime();
		
	//--------------------------------------------------


	//--------------------------------------------------
	//Render
	HRESULT						Render();
	void						LogFrameHierarchy( LPD3DXFRAME pFrame, int generation );
	//--------------------------------------------------


	//release
	HRESULT						Release();


protected:
	
    HRESULT						SetupBoneMatrixPointersOnMesh( LPD3DXMESHCONTAINER pMeshContainer );
    HRESULT						SetupBoneMatrixPointers( LPD3DXFRAME pFrame );

	void						DrawMeshContainer( LPD3DXMESHCONTAINER pMeshContainerBase, LPD3DXFRAME pFrameBase );
    void						DrawFrame( LPD3DXFRAME pFrame );


	void						UpdateFrameMatrices( LPD3DXFRAME pFrameBase, LPD3DXMATRIX pParentMatrix );
	void						UpdateFrameMatricesAnimPass( LPD3DXFRAME pFrameBase, LPD3DXMATRIX pParentMatrix );
	
	


	int							GetBoneNumberByName(LPCSTR);
	LPD3DXSKININFO				FindFirstSkinInfo(LPD3DXFRAME);
	LPD3DXMESHCONTAINER			FindFirstMeshContainerWithSkinInfo(LPD3DXFRAME);

};
