// MyModel.h: interface for the CMyModel class.
//
//////////////////////////////////////////////////////////////////////

#include "CModel.h"
#include "DXUtil.h"
#include "D3DEnumeration.h"
#include "D3DSettings.h"
#include "D3DApp.h"
#include "D3DFont.h"
#include "D3DFile.h"
#include "D3DUtil.h"
#include "DMUtil.h"


#define NOARMOR -1


//animation type defines
#define HUMAN 0
#define DARK 1
#define WOLF 2





//define block types
#define NOBLOCK 0 
#define BLOCKLEFT 1 
#define BLOCKRIGHT 2 
#define BLOCKDOWN 3
#define BLOCKUP 4 


//define attack types
#define NOATTACK 0 
#define ATTACKLEFT 1 
#define ATTACKRIGHT 2 
#define ATTACKDOWN 3 
#define ATTACKUP 4


enum SAT_TYPE
{
	SAT_APPLYSTOCK, //apply stock animation
	SAT_STOCK,
	SAT_FORCE, //do it anyway
	SAT_ANIMATIONPERIOD, //do it if current once animation just ended
	SAT_ANIMATIONPERIODNOSTRIKES,
	SAT_ANIMATIONPERIODHURT,

	SAT_TEST //just in case
};



//-----------------------------------------------
// Name: 
// Desc: wrap for CModel, maybe a stupid idea
//		 designed specially for characters
//-----------------------------------------------
class CMyModel : public CModel
{

//variables
public:
	BOOL bIsWpnDrawned;
	int CurrentArmor;
	int CurrentWeapon;

	BOOL bLocked;
	int lockTarget;

	BOOL bItem;
	int itemTarget;

	D3DXVECTOR3 speed;
	D3DXVECTOR3 oldCoord;

	BOOL bHurt;
	BOOL bDead;
	BOOL bFail;
	float fFlyTime;

	//SOUND
	C3DMusicSegment*          m_pSound;


	//collisiondetention stuff
	BOOL bLeftLegStep;
	BOOL bRightLegStep;
	BOOL bSwitchLegs;


	BOOL bVerticalCollisionUP;
	BOOL bVerticalCollisionDOWN;


	//rpg stuff
	int life;
	int mana;
	

//functions
public:
	CMyModel();

	HRESULT PassAnimationStock();
	HRESULT SwitchToAnimation( SAT_TYPE flag, LPCSTR animName = NULL, double fTime = -100.f, char* stockanimName = NULL, double fStockTime = -100.f );


	//statistic
	BOOL OnceAnimation(); //including strikes
	BOOL OnceAnimationNoStrikes(); //without strikes
	BOOL OnceAnimationHurt(); //just hurt
	
	char IsAttack();
	char IsBlock();
	

	//FrameMove
	D3DXVECTOR3 UpdateCoordinates(double);


	//collision additional stuff
	CollisionModel ComputeBigBoundingBox();



	// AI STUFF
	//------

	void AIFace(D3DXVECTOR3 InCoord, double ElapsedTime);

	BOOL AIRandomAttack();
	void AIBlock(char AttackID);

	//------
};
