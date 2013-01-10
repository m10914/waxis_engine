#ifndef MYCHARACTERCONTROLLER_H
#define MYCHARACTERCONTROLLER_H

#include "NxControllerManager.h"
#include "NxBoxController.h"
#include "NxCapsuleController.h"
#include "NxPhysics.h"
#include "../log.h"

enum CollisionGroup
{
	GROUP_NON_COLLIDABLE,
	GROUP_COLLIDABLE_NON_PUSHABLE,
	GROUP_COLLIDABLE_PUSHABLE,
	GROUP_CONTROLLER,
	GROUP_WEAPONS,
	GROUP_BIPED,
};

void SetActorCollisionGroup(NxActor* actor, NxCollisionGroup group);

struct HitReport
{
	void* contruserdata; //userData of controller
	void* shapeuserdata; //userData of shape
};

#define UI_SKINMESH 0
#define UI_OBJECT 1
#define UI_WEAPON 2
#define UI_UNKNOWN 3


//this struct is for weapons
//we will know
//
struct CModelUserData
{
	//hit data - for target
	bool bHit;
	char targettype;
	int targetid;

	//cmodel data
	char type;
	int id;
	int* attacherid; //for weapons 
};


class MyControllerHitReport : public NxUserControllerHitReport
{
	public:

	virtual NxControllerAction onShapeHit(const NxControllerShapeHit& hit)
	{
		if(hit.shape)
		{
			NxCollisionGroup group = hit.shape->getGroup();
			if(group == GROUP_COLLIDABLE_NON_PUSHABLE)
			{
				NxActor& actor = hit.shape->getActor();
				if(actor.isDynamic())
				{
					// We only allow horizontal pushes. Vertical pushes when we stand on dynamic objects creates
					// useless stress on the solver. It would be possible to enable/disable vertical pushes on
					// particular objects, if the gameplay requires it.	
					if(hit.dir.y==0.0f)
					{
						NxF32 coeff = actor.getMass() * hit.length * 1.0f;
						actor.addForceAtLocalPos(hit.dir*coeff, NxVec3(0,0,0), NX_IMPULSE);
					}
				}
			}
			else if(group == GROUP_WEAPONS )
			{
				HTMLLog("hit!\n");
				/*NxActor* actor;
				actor = hit.controller->getActor();
				if(actor->userData)
				{
					char* hit = (char*)actor->userData;
					*hit = 1;
				}*/
				
			}
		}
		
		return NX_ACTION_NONE;
	}

	virtual NxControllerAction onControllerHit(const NxControllersHit& hit)
	{
		return NX_ACTION_NONE;
	}

};

class MyCharacterController
{
public:
	MyCharacterController(NxControllerManager* manager, NxScene *scene, NxVec3 pos, NxVec3 dim);
	MyCharacterController(NxControllerManager* manager, NxScene *scene, NxVec3 pos, NxReal radius, NxReal height);

	~MyCharacterController();

	void Move(const NxVec3 &disp, NxU32 &flag);
	NxActor* GetCharacterActor();
	bool SetCharacterPos(NxVec3 pos);
	void UpdateCharacterExtents(NxScene *scene, NxVec3 offset);

private:
	NxControllerManager* mManager; // Singleton
	NxController* mController;
	MyControllerHitReport mReport;
};
#endif
