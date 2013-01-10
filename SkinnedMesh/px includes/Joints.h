

#ifndef JOINTS_H
#define JOINTS_H

#include "NxPhysics.h"

NxFixedJoint* CreateFixedJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis, NxScene* gScene);
NxRevoluteJoint* CreateRevoluteJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis, NxScene* gScene);
NxSphericalJoint* CreateSphericalJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis, NxScene* gScene);
NxPrismaticJoint* CreatePrismaticJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis, NxScene* gScene);
NxCylindricalJoint* CreateCylindricalJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis, NxScene* gScene);
NxPointOnLineJoint* CreatePointOnLineJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis, NxScene* gScene);
NxPointInPlaneJoint* CreatePointInPlaneJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis, NxScene* gScene);

NxSphericalJoint* CreateRopeSphericalJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis, NxScene* gScene);
NxSphericalJoint* CreateClothSphericalJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis, NxScene* gScene);
NxSphericalJoint* CreateBodySphericalJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis, NxScene* gScene);
NxRevoluteJoint* CreateWheelJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis, NxScene* gScene);
NxRevoluteJoint* CreateStepJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis, NxScene* gScene);

NxRevoluteJoint* CreateChassisJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis, NxScene* gScene);
NxFixedJoint* CreateCannonJoint(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis, NxScene* gScene);

NxSphericalJoint* CreateBladeLink(NxActor* a0, NxActor* a1, const NxVec3& globalAnchor, const NxVec3& globalAxis, NxScene* gScene);

#endif  // JOINTS_H

