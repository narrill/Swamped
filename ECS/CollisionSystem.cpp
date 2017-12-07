#include "CollisionSystem.h"
#include "TransformSystem.h"
#include "Game.h"
#include "CollisionFunctions.h"

#define CELL_DIVISIONS 9
#define DESIRED_OBJECT_DENSITY 1500

using namespace DirectX;

CollisionSystem::CollisionSystem() {
#if BENCHMARK >=0
	m_collisionFunctions.push_back(std::make_tuple(CollisionType::test1, CollisionType::test2, &CollisionFunctions::NoOpCollision));
	m_collisionFunctions.push_back(std::make_tuple(CollisionType::test1, CollisionType::test1, &CollisionFunctions::NoOpCollision));
#endif
}

CollisionSystem::~CollisionSystem() {

}

XMFLOAT3 CollisionSystem::GetCellCounts() {
	return m_cellCounts;
}

//Generates AABBs then checks them for collisions
void CollisionSystem::Update(Game * game, float dt) {
	StartTimer();
	Collapse();
	if (m_collapsedCount == 0)
		return;
	//pre-allocate stuff
	EntityId entityId;
	TransformSystem * ts = &game->m_transformSystem;
	BoundingBox * cc;
	vector<ComponentData> & tcds = ts->GetComponentData();
	FreeVector<TransformComponent> & tcs = ts->GetComponentList1();
	TransformComponent * tc;
	XMVECTOR original;
	XMMATRIX modelToWorld;
	XMVECTOR max;
	XMVECTOR min;
	XMVECTOR globalMax = XMLoadFloat3(&XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX));
	XMVECTOR globalMin = XMLoadFloat3(&XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX));
	XMVECTOR globalPad = XMLoadFloat3(&XMFLOAT3(1, 1, 1));
	//XMVECTOR position;

	//size aabb list appropriately
	m_aabbs.resize(m_components.count()); //use count to avoid dead elements in aabb list
	unsigned int aabbIndex = 0; //reset index	

	//loop through bounding boxes
	unsigned int transformIndex = 0;
	for (unsigned int c = 0; c < m_collapsedCount; c++) {

		cc = &m_collapsedComponents[c].m_component; //get component
		entityId = m_collapsedComponents[c].m_entityId; //get entityID

		//save the transform and its quat
		tc = &ts->GetComponent1(entityId);
		modelToWorld = TransformSystem::GetMatrix(*tc);

		//reset max and min values
		max = XMLoadFloat3(&XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX));
		min = XMLoadFloat3(&XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX));

		//loop through eight bounding box points
		for (unsigned int n = 0; n < 8; n++) {
			//load and translate to world space
			original = DirectX::XMLoadFloat3(&cc->m_corners[n]);
			original = XMVector3Transform(original, modelToWorld);
			//check against max and min
			max = XMVectorMax(original, max);
			min = XMVectorMin(original, min);
		}
		float distanceFromGround = XMVectorGetY(min) - 0;
		if (distanceFromGround < 0)
		{
			XMVECTOR offset = XMLoadFloat3(&XMFLOAT3(0, -distanceFromGround, 0));
			max = XMVectorAdd(max, offset);
			min = XMVectorAdd(min, offset);
			XMStoreFloat3(&(tc->m_position), XMVectorAdd(XMLoadFloat3(&tc->m_position), offset));
			ts->GetComponentList2()[transformIndex].m_velocity.y = 0;
		}
		//store final translated max and min in aabb list
		XMStoreFloat3(&m_aabbs[c].m_component.m_max, max);
		XMStoreFloat3(&m_aabbs[c].m_component.m_min, min);
		globalMax = XMVectorMax(globalMax, max);
		globalMin = XMVectorMin(globalMin, min);
		m_aabbs[c].m_component.m_collisionType = cc->m_collisionType;
		m_aabbs[c].m_entityId = entityId;
		m_aabbs[c].m_handle = m_collapsedComponents[c].m_handle;
	}

	globalMax = XMVectorAdd(globalMax, globalPad);
	globalMin = XMVectorSubtract(globalMin, globalPad);

	//prep spatial hash grid
	XMVECTOR dimensions = XMVectorSubtract(globalMax, globalMin);
	float cellDivisions = ceilf(static_cast<float>(m_collapsedCount) / DESIRED_OBJECT_DENSITY);
	dimensions = XMVectorScale(dimensions, 1.0f/cellDivisions);
	m_cellCounts = XMFLOAT3(cellDivisions, cellDivisions, cellDivisions);
	for (unsigned int c = 0; c < m_spatialHashGrid.size(); c++)
		for (unsigned int n = 0; n < CollisionType::NUMTYPES;n++)
			m_spatialHashGrid[c][n].clear();
	m_spatialHashGrid.resize(static_cast<size_t>(m_cellCounts.x * m_cellCounts.y * m_cellCounts.z), vector<ClearVector<CollapsedComponent<MaxMin>>>(CollisionType::NUMTYPES));

	//populate spatial hash grid
#ifdef _DEBUG
	for (unsigned int c = 0; c < m_collapsedCount; c++) {
#else
	parallel_for(size_t(0), m_collapsedCount, [&](unsigned int c) {
#endif
		CollapsedComponent<TypedMaxMin> caabb = m_aabbs[c];
		TypedMaxMin aabb = caabb.m_component;
		XMFLOAT3 bb[8] = {
			{aabb.m_max.x, aabb.m_max.y, aabb.m_max.z},
			{ aabb.m_max.x, aabb.m_max.y, aabb.m_min.z },
			{ aabb.m_max.x, aabb.m_min.y, aabb.m_max.z },
			{ aabb.m_max.x, aabb.m_min.y, aabb.m_min.z },
			{ aabb.m_min.x, aabb.m_max.y, aabb.m_max.z },
			{ aabb.m_min.x, aabb.m_max.y, aabb.m_min.z },
			{ aabb.m_min.x, aabb.m_min.y, aabb.m_max.z },
			{ aabb.m_min.x, aabb.m_min.y, aabb.m_min.z }
		};
		XMVECTOR pointxm;
		ClearArray<8, unsigned int> gridIndices;
		for (auto& point : bb) {
			pointxm = XMLoadFloat3(&point);
			XMStoreFloat3(&point, XMVectorFloor(XMVectorDivide(XMVectorSubtract(pointxm, globalMin), dimensions)));
			gridIndices.push(static_cast<unsigned int>(point.z * m_cellCounts.x * m_cellCounts.y + point.y * m_cellCounts.x + point.x), true);
		}
		for (unsigned int n = 0; n < gridIndices.size(); n++) {
			m_spatialHashGrid[gridIndices[n]][caabb.m_component.m_collisionType].add({ aabb, caabb.m_entityId, caabb.m_handle });
		}
#ifdef _DEBUG
	}
#else
	});
#endif

	//clear collision map
	for (auto& kv : m_collisionMap) {
		kv.second.clear();
	}
	m_registeredCollisions.resize(m_componentData.size());
	for (auto& cl : m_registeredCollisions)
		cl.clear();

	//check collisions
#ifdef _DEBUG
	for (unsigned int b = 0; b < m_spatialHashGrid.size(); b++) {
#else
	parallel_for(size_t(0), m_spatialHashGrid.size(), [&](unsigned int b) {
#endif
		auto& bucketCv = m_spatialHashGrid[b];
		for (auto cfDef : m_collisionFunctions) {
			CollisionType ct1 = std::get<0>(cfDef);
			CollisionType ct2 = std::get<1>(cfDef);
			if (bucketCv[ct1].size() == 0 || bucketCv[ct2].size() == 0)
				continue;
			bool selfCheck = ct1 == ct2;
			unsigned int outerLength = (!selfCheck) ? bucketCv[ct1].size() : bucketCv[ct1].size() - 1;
			CollisionFunction cf = std::get<2>(cfDef);
			for (unsigned int c = 0; c < outerLength; c++) {
				CollapsedComponent<MaxMin> caabb1 = bucketCv[ct1][c];
				MaxMin aabb1 = caabb1.m_component;
				CollapsedComponent<MaxMin> caabb2;
				MaxMin aabb2;
				for (unsigned int n = (selfCheck) ? c + 1 : 0; n < bucketCv[ct2].size(); n++) {
					caabb2 = bucketCv[ct2][n];
					aabb2 = caabb2.m_component;

					//add pair of indices on collision
					if (aabb1.m_max.x > aabb2.m_min.x && aabb1.m_min.x < aabb2.m_max.x
						&& aabb1.m_max.y > aabb2.m_min.y && aabb1.m_min.y < aabb2.m_max.y
						&& aabb1.m_max.z > aabb2.m_min.z && aabb1.m_min.z < aabb2.m_max.z
						&& !m_registeredCollisions[caabb1.m_handle].contains(caabb2.m_entityId)
						&& !m_registeredCollisions[caabb2.m_handle].contains(caabb1.m_entityId))
					{
						m_registeredCollisions[caabb1.m_handle].add(caabb2.m_entityId);
						m_registeredCollisions[caabb2.m_handle].add(caabb1.m_entityId);
						m_collisionMap[cf].push_back(std::make_pair(caabb1.m_entityId, caabb2.m_entityId));
					}
				}
			}
		}
#ifdef _DEBUG
	}
#else
	});
#endif
	
	//loop through all collision functions
	for (auto& kv : m_collisionMap) {
		auto collisions = kv.second; //get list of actual collisions
		for (unsigned int c = 0; c < collisions.size(); c++) {
			//pass both entityIDs and dT to collision function
			kv.first(game, collisions[c].first, collisions[c].second, dt);
		}
	}
	StopTimer();
}