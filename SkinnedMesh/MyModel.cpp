// MyModel.cpp: implementation of the CMyModel class.
//
//////////////////////////////////////////////////////////////////////

#include "MyModel.h"




//-----------------------------------------------------------------------------
// Name: AI BLOCK
// Desc: fuctions for AI
//-----------------------------------------------------------------------------

void CMyModel::AIFace(D3DXVECTOR3 InCoord, double angxelapsedtime)
{
	D3DXVECTOR3 CoordVec = InCoord - coordinate;

	float ang = atan(CoordVec.x/CoordVec.z);

	if(CoordVec.z < 0) ang += D3DX_PI;
	if(ang < 0) ang += D3DX_PI * 2.0f;
		ang += D3DX_PI;
	if( ang > D3DX_PI * 2.0f ) ang -= D3DX_PI * 2.0f;

	float pang = angle.y - ang;

	//holy crap! that was hard
	if( (fabs(pang) < D3DX_PI && fabs(pang) < 7.7f * angxelapsedtime) ||
		(fabs(pang) > D3DX_PI && D3DX_PI * 2.0f - fabs(pang) < 7.7f * angxelapsedtime)
	  ) 
	{ 
		angle.y = ang; 
		return; 
	}
	//if( id == 0 && (pang > 2 || pang < -2)) HTMLLog("%f %f - %f\n", angle.y, ang, pang);

	if( (pang > 0 && pang < D3DX_PI) || ( pang < 0 && pang < -D3DX_PI ) )
		 angle.y -= 7.7f * angxelapsedtime;
	else angle.y += 7.7f * angxelapsedtime;

	return;
}


BOOL CMyModel::AIRandomAttack()
{

	//getting name
	BOOL bAnykeyDown = FALSE;
	//char* AnimSetName = AnimSetName;
	int x = rand()%130;

	
	if( x == 0 )
	{
		if(  !strcmp(ASI.AnimSetName, "WPNSTANDA") ||
			 //!strcmp(AnimSetName, "STRIKELSTA") ||
			 !strcmp(ASI.AnimSetName, "STRIKERSTA") ||
			 //!strcmp(AnimSetName, "STRIKEDSTA") ||
			 !strcmp(ASI.AnimSetName, "STRIKEUSTA")
		  )
		{
			PassAnimation("STRIKEDA", 0.3f);
			SetStockAnimationSetByName("STRIKEDSTA", 0.1f);

			bAnykeyDown = TRUE;
		}

	}
	else if( x == 1 )
	{
		if(  !strcmp(ASI.AnimSetName, "WPNSTANDA") ||
			 //!strcmp(AnimSetName, "STRIKELSTA") ||
			 !strcmp(ASI.AnimSetName, "STRIKERSTA") ||
			 !strcmp(ASI.AnimSetName, "STRIKEDSTA")
			 //!strcmp(AnimSetName, "STRIKEUSTA")
		  )
		{
			PassAnimation("STRIKEUA", 0.3f);
			SetStockAnimationSetByName("STRIKEUSTA", 0.1f);

			bAnykeyDown = TRUE;
		}
	}

	else if( x == 2 )
	{
		if(  !strcmp(ASI.AnimSetName, "WPNSTANDA") ||
			 !strcmp(ASI.AnimSetName, "STRIKELSTA") ||
			 //!strcmp(AnimSetName, "STRIKERSTA") ||
			 !strcmp(ASI.AnimSetName, "STRIKEDSTA") 
			 //!strcmp(AnimSetName, "STRIKEUSTA")
		  )
		{
			PassAnimation("STRIKERA", 0.15f);
			SetStockAnimationSetByName("STRIKERSTA", 0.1f);

			bAnykeyDown = TRUE;
		}
	}

	else if( x == 3 )
	{
		if(  !strcmp(ASI.AnimSetName, "WPNSTANDA") ||
			 //!strcmp(AnimSetName, "STRIKELSTA") ||
			 !strcmp(ASI.AnimSetName, "STRIKERSTA") 
			 //!strcmp(AnimSetName, "STRIKEDSTA") ||
			 //!strcmp(AnimSetName, "STRIKEUSTA")
		  )
		{
			PassAnimation("STRIKELA", 0.2f);
			SetStockAnimationSetByName("STRIKELSTA", 0.1f);

			bAnykeyDown = TRUE;
		}
	}


	return bAnykeyDown;
}



void CMyModel::AIBlock(char AttackID)
{
	
	if( AttackID == ATTACKUP )
	{
		PassAnimation("BLOCKUA", 0.05f);
		SetStockAnimationSetByName("WPNSTANDA", 0.1f);			
	}
	else if( AttackID == ATTACKDOWN )
	{
		PassAnimation("BLOCKDA", 0.05f);
		SetStockAnimationSetByName("WPNSTANDA", 0.1f);

	}
	else if( AttackID == ATTACKLEFT )
	{
		PassAnimation("BLOCKRA", 0.05f);
		SetStockAnimationSetByName("WPNSTANDA", 0.1f);
	}
	else if( AttackID == ATTACKRIGHT )
	{
		PassAnimation("BLOCKLA", 0.05f);
		SetStockAnimationSetByName("WPNSTANDA", 0.1f);
	}


	return;
}














//-----------------------------------------------------------------------------
// Name: constructor
// Desc:
//-----------------------------------------------------------------------------
CMyModel::CMyModel()
{
	//a lot of things
	bIsWpnDrawned = FALSE;
	bVerticalCollisionUP = FALSE;

	//LogPlease("constructor!!\n");
	//RPG-stuff
	CurrentArmor = NOARMOR;
	life = 100;
	mana = 100;

	bDead = FALSE;
	bFail = FALSE;
	bLocked = FALSE;
	bItem = FALSE;
	fFlyTime = 0;

	ASI.AnimSetName = "STAND";
	ASI.StockAnimationSet = "STAND";

	m_pSound = NULL;
}



//-----------------------------------------------------------------------------
// Name: PassAnimationStock()
// Desc: override for CMyModel
//-----------------------------------------------------------------------------
HRESULT CMyModel::PassAnimationStock()
{

	PassAnimation( ASI.StockAnimationSet, ASI.StockTime );
	
	//doubtful part of the code
	if( bIsWpnDrawned == TRUE ) ASI.StockAnimationSet = "WPNSTANDA";
	else ASI.StockAnimationSet = "STAND";

	return S_OK;
}


/*
//-----------------------------------------------------------------------------
// Name:
// Desc: every frame!!!
//-----------------------------------------------------------------------------
CollisionModel CMyModel::ComputeBigBoundingBox(void)
{

	CollisionModel result;
	CollisionModel dummy;


	dummy = GetCMByName("Bip01_R_Foot");
	result.vMin.y = dummy.vMin.y;
	dummy = GetCMByName("Bip01_L_Foot");
	
	if( result.vMin.y < dummy.vMin.y )
	{
		if(bLeftLegStep == TRUE) bSwitchLegs = TRUE;
		else bSwitchLegs = FALSE;

		bRightLegStep = TRUE;
		bLeftLegStep = FALSE;
	}
	else
	{
		result.vMin.y = dummy.vMin.y;

		if(bRightLegStep == TRUE) bSwitchLegs = TRUE;
		else bSwitchLegs = FALSE;

		bRightLegStep = FALSE;
		bLeftLegStep = TRUE;
	}
				



	dummy = GetCMByName("Bip01_L_Clavicle");
	result.vMax.y = dummy.vMax.y + 20.f;

	result.vMax.x = dummy.vMax.x;
	result.vMax.z = dummy.vMax.z;
	result.vMin.x = dummy.vMin.x;
	result.vMin.z = dummy.vMin.z;
	
	result.vMin.y = min(dummy.vMin.y, result.vMin.y);

	dummy = GetCMByName("Bip01_R_Clavicle");
	result.vMax.x = max( result.vMax.x, dummy.vMax.x);
	result.vMax.z = max( result.vMax.z, dummy.vMax.z);
	result.vMin.x = min( result.vMin.x, dummy.vMin.x);
	result.vMin.z = min( result.vMin.z, dummy.vMin.z);

	result.vMin.y = min(dummy.vMin.y, result.vMin.y);

	return result;
}
*/


//-----------------------------------------------------------------------------
// Name:
// Desc: designed to simplify animation controlling process
//-----------------------------------------------------------------------------
HRESULT CMyModel::SwitchToAnimation(SAT_TYPE flag, LPCSTR animName, double fTime, char* stockanimName, double fStockTime )
{

	//deal with animation and stock
	switch(flag)
	{
	//empty case
	case SAT_TEST:
		break;


	case SAT_STOCK:
		if( fTime > 0 ) SetStockAnimationSetByName( (char*)animName, fTime );
		break;

	//apply stock animation
	case SAT_APPLYSTOCK:
		PassAnimationStock();
		break;


	//force to apply animation
	case SAT_FORCE:
		if( fTime > 0 ) PassAnimation(animName, fTime);
		if( fStockTime > 0 ) SetStockAnimationSetByName( stockanimName, fStockTime );
		break;

	//do animation only if current animation is over
	case SAT_ANIMATIONPERIOD:

		if( OnceAnimation() == FALSE || 
			ASI.bAnimPeriodPassed == TRUE )
		{
			if( fTime > 0 ) PassAnimation(animName, fTime);
			if( fStockTime > 0 ) SetStockAnimationSetByName( stockanimName, fStockTime );
		}
		break;

	//do animation only if current animation is over
	case SAT_ANIMATIONPERIODNOSTRIKES:

		if( OnceAnimationNoStrikes() == FALSE || 
			ASI.bAnimPeriodPassed == TRUE )
		{

			if( fTime > 0 ) PassAnimation(animName, fTime);
			if( fStockTime > 0 ) SetStockAnimationSetByName( stockanimName, fStockTime );
		}
		break;

	//do animation only if current animation is over
	case SAT_ANIMATIONPERIODHURT:

		if( OnceAnimationHurt() == FALSE || 
			ASI.bAnimPeriodPassed == TRUE )
		{

			if( fTime > 0 ) PassAnimation(animName, fTime);
			if( fStockTime > 0 ) SetStockAnimationSetByName( stockanimName, fStockTime );
		}
		break;
	}


	return S_OK;
}


//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
char CMyModel::IsAttack()
{
	char* name;

	//first check
	if(MType == BASE) name = GetTrackAnimationSetName(0);
	else if(MType == CLONE) name = ASI.AnimSetName;

	if( !strcmp(name, "STRIKEDA") )
		return ATTACKDOWN;

	else if( !strcmp(name, "STRIKEUA") )
		return ATTACKUP;

	else if( !strcmp(name, "STRIKELA") )
		return ATTACKLEFT;
	
	else if( !strcmp(name, "STRIKERA") )
		return ATTACKRIGHT;

	else return NOATTACK;
}


//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
char CMyModel::IsBlock()
{
	char* name;

	//first check
	if(MType == BASE) name = GetTrackAnimationSetName(0);
	else if(MType == CLONE) name = ASI.AnimSetName;


	if( !strcmp(name, "BLOCKLA") ) return BLOCKRIGHT;

	else if( !strcmp(name, "BLOCKRA") ) return BLOCKLEFT;

	else if( !strcmp(name, "BLOCKDA") ) return BLOCKDOWN;

	else if( !strcmp(name, "BLOCKUA") ) return BLOCKUP;

	else return NOBLOCK;
}


//-----------------------------------------------------------------------------
// Name:
// Desc: animations that cannot be interrupted
//-----------------------------------------------------------------------------
BOOL CMyModel::OnceAnimation()
{
	char* name;

	if( IsAnimPass() == TRUE )
	{
		return TRUE;
	}


	//first check
	if(MType == BASE) name = GetTrackAnimationSetName(0);
	else if(MType == CLONE) name = ASI.AnimSetName;


	//(!strcmp(name, "") && bAnimPeriodPassed == TRUE ) ||
	if( ( !strcmp(name, "WPNDRAW1H") ) ||
		( !strcmp(name, "WPNHIDE1H") ) ||


		( !strcmp(name, "STRIKELA") ) ||
		( !strcmp(name, "STRIKELSTA") ) ||
		( !strcmp(name, "STRIKERA") ) ||
		( !strcmp(name, "STRIKERSTA") ) ||
		( !strcmp(name, "STRIKEDA") ) ||
		( !strcmp(name, "STRIKEDSTA") ) ||
		( !strcmp(name, "STRIKEUA") ) ||
		( !strcmp(name, "STRIKEUSTA") ) ||


		( !strcmp(name, "CAST3") ) ||
		( !strcmp(name, "CAST2") ) ||
		( !strcmp(name, "CAST1") ) ||


		( !strcmp(name, "WPNGRACEA") ) ||

		( !strcmp(name, "HURTLITTLE") ) ||
		( !strcmp(name, "HURTMUCH") ) ||
		( !strcmp(name, "DODGE") ) ||


		( !strcmp(name, "STRIKELFAIL") ) ||
		( !strcmp(name, "STRIKERFAIL") ) ||
		( !strcmp(name, "STRIKEUFAIL") ) ||
		( !strcmp(name, "STRIKEDFAIL") ) ||

		( !strcmp(name, "BLOCKLA") ) ||
		( !strcmp(name, "BLOCKRA") ) ||
		( !strcmp(name, "BLOCKUA") ) ||
		( !strcmp(name, "BLOCKDA") ) ||

		( !strcmp(name, "TALK1") ) ||
		( !strcmp(name, "TALK2") ) ||

		( !strcmp(name, "FALLDOWN") ) ||
		( !strcmp(name, "FALLDOWNGETUP") ) ||
		( !strcmp(name, "FLY") ) ||
		( !strcmp(name, "JUMP"    ) )
		) 
		return TRUE;

	else return FALSE;
}



//-----------------------------------------------------------------------------
// Name:
// Desc: specially for once animations
//-----------------------------------------------------------------------------
BOOL CMyModel::OnceAnimationHurt()
{
	char* name;


	//if( IsAnimPass() == TRUE )
	//{
	//	return TRUE;
	//}


	//first check
	if(MType == BASE) name = GetTrackAnimationSetName(0);
	else if(MType == CLONE) name = ASI.AnimSetName;


	//(!strcmp(name, "") && bAnimPeriodPassed == TRUE ) ||
	if( !strcmp(name, "HURTLITTLE") ||
		!strcmp(name, "HURTMUCH") ||
		!strcmp(name, "FALLDOWNGETUP") ||
		!strcmp(name, "FALLDOWN")

		) return TRUE;

	else return FALSE;
}




//-----------------------------------------------------------------------------
// Name:
// Desc: specially for once animations
//-----------------------------------------------------------------------------
BOOL CMyModel::OnceAnimationNoStrikes()
{
	char* name;



	if( IsAnimPass() == TRUE )
	{
		return TRUE;
	}


	//first check
	if(MType == BASE) name = GetTrackAnimationSetName(0);
	else if(MType == CLONE) name = ASI.AnimSetName;



	//(!strcmp(name, "") && bAnimPeriodPassed == TRUE ) ||
	if( ( !strcmp(name, "WPNDRAW1H") ) ||
		( !strcmp(name, "WPNHIDE1H") ) ||
		

		( !strcmp(name, "CAST3") ) ||
		( !strcmp(name, "CAST2") ) ||
		( !strcmp(name, "CAST1") ) ||

		( !strcmp(name, "WPNGRACEA") ) ||


		( !strcmp(name, "HURTLITTLE") ) ||
		( !strcmp(name, "HURTMUCH") ) ||
		( !strcmp(name, "DODGE") ) ||

		( !strcmp(name, "BLOCKLA") ) ||
		( !strcmp(name, "BLOCKRA") ) ||
		( !strcmp(name, "BLOCKUA") ) ||
		( !strcmp(name, "BLOCKDA") ) ||
		
		
		( !strcmp(name, "STRIKELFAIL") ) ||
		( !strcmp(name, "STRIKERFAIL") ) ||
		( !strcmp(name, "STRIKEUFAIL") ) ||
		( !strcmp(name, "STRIKEDFAIL") ) ||


		( !strcmp(name, "TALK1") ) ||
		( !strcmp(name, "TALK2") ) ||

		( !strcmp(name, "FALLDOWN") ) ||
		( !strcmp(name, "FALLDOWNGETUP") ) ||
		( !strcmp(name, "FLY") ) ||
		( !strcmp(name, "JUMP"    ) )

		) return TRUE;

	else return FALSE;
}


//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
D3DXVECTOR3 CMyModel::UpdateCoordinates(double inElapsedTime)
{
	//store previous
	oldCoord = coordinate;

	//displacement
	//coordinate.x += speed.x * (float)inElapsedTime;
	//coordinate.y += speed.y * (float)inElapsedTime;
	//coordinate.z += speed.z * (float)inElapsedTime;

	D3DXVECTOR3 displ = D3DXVECTOR3(0,0,0);
	displ.x += speed.x * (float)inElapsedTime;
	displ.y += speed.y * (float)inElapsedTime;
	displ.z += speed.z * (float)inElapsedTime;

	NxU32 collisionFlags;
	controller->Move( DxVec3ToNxVec3(displ), collisionFlags);
	mManager->updateControllers();

	if (collisionFlags & NXCC_COLLISION_DOWN)
	{
		fFlyTime = 0;
		speed.y = 0;
		bVerticalCollisionUP = TRUE;
	}
	else if (collisionFlags & NXCC_COLLISION_UP)
	{
		fFlyTime = 0;
		speed.y = 0;
		bVerticalCollisionDOWN = TRUE;
	}
	else
	{
		fFlyTime += inElapsedTime;
		speed.y -= 9.8f*inElapsedTime*100;
	}


	NxMat33 matrot = controller->GetCharacterActor()->getGlobalOrientation();
	matrot.rotY( angle.y );
	controller->GetCharacterActor()->setGlobalOrientation(matrot);

	return displ;
}


