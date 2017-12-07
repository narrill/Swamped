#pragma once
#include "CollisionComponent.h"
#include "CollisionFunctionTypeDef.h"
#include "EntityIdTypeDef.h"
#include "GlobalFunctions.h"
#include <vector>

class CollisionFunctions {
public:
	static void NoOpCollision(Game * g, EntityId entityId1, EntityId entityId2, float dt);
	static void EndState(Game * g, EntityId entityId1, EntityId entityId2, float dt);
	static float frand2(float min, float max);
};
