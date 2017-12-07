#pragma once
#include "Game.h"
#include "ContentManager.h"
#include "RenderingComponent.h"
#include "GlobalFunctions.h"
#include <unordered_map>

using namespace DirectX;
//Contains static constructors for preformed entities
class Constructors {
	static unordered_map<std::string, RenderingComponent> m_renderingComponents;
public:
	static void Init(Game * g) {
#if BENCHMARK >= 0
		m_renderingComponents["testObj"] = {
			g->m_contentManager.GetMaterial("brickLightingNormalMap"),
			g->m_contentManager.GetMeshStore("cone.obj").m_m
		};
		m_renderingComponents["testObj2"] = {
			g->m_contentManager.GetMaterial("brickLightingNormalMap"),
			g->m_contentManager.GetMeshStore("cube.obj").m_m
		};
#endif
		m_renderingComponents["groundPlane"] = {
			g->m_contentManager.GetMaterial("Ground"),
			g->m_contentManager.GetMeshStore("Quad.obj").m_m
		};
	}
#if BENCHMARK >= 0
	static void CreateTestObject(Game * game) {
		//get entityID and add system list to game
		EntityId eid = game->m_entities.add(vector<ISystem*>{
			&game->m_transformSystem, 
			&game->m_collisionSystem,
			&game->m_renderingSystem
		});

		//get mesh and bounding box
		MeshStore ms = game->m_contentManager.GetMeshStore("cone.obj");
		PhysicsComponent pc;
		pc.m_velocity = XMFLOAT3(0, 0, 0);
		pc.m_acceleration = XMFLOAT3(0, 0, 0);
		
		TransformComponent tc;
		tc.m_position = XMFLOAT3(fRand(-100, 100), fRand(0, 100), fRand(-100, 100));
		ms.m_bb.m_collisionType = CollisionType::test1;

		//create components
		game->m_transformSystem.Create(eid, tc, pc);
		game->m_collisionSystem.Create(eid, ms.m_bb);
		game->m_renderingSystem.Create(eid, &m_renderingComponents["testObj"]);
	}

	static void CreateTestObject2(Game * game) {
		//get entityID and add system list to game
		EntityId eid = game->m_entities.add(vector<ISystem*>{
			&game->m_transformSystem,
			&game->m_collisionSystem,
			&game->m_renderingSystem
		});

		//get mesh and bounding box
		MeshStore ms = game->m_contentManager.GetMeshStore("cube.obj");
		PhysicsComponent pc;
		pc.m_velocity = XMFLOAT3(0, 0, 0);
		pc.m_acceleration = XMFLOAT3(0, 0, 0);

		pc.m_rotationalVelocity = XMFLOAT3(fRand(-30, 30), fRand(-30, 30), fRand(-30, 30));
		TransformComponent tc;
		tc.m_position = XMFLOAT3(fRand(-100, 100), fRand(0, 100), fRand(-100, 100));

		ms.m_bb.m_collisionType = CollisionType::test2;

		//create components
		game->m_transformSystem.Create(eid, tc, pc);
		game->m_collisionSystem.Create(eid, ms.m_bb);
		game->m_renderingSystem.Create(eid, &m_renderingComponents["testObj2"]);
	}
#endif

	static EntityId CreatePlayer(Game * g)
	{
		EntityId eid = g->m_entities.add(vector<ISystem*>{
			&g->m_transformSystem
		});

		//get mesh and bounding box
		MeshStore ms = g->m_contentManager.GetMeshStore("cone.obj");

		PhysicsComponent pc;
		pc.m_velocity = XMFLOAT3(0, 0, 0);
		pc.m_acceleration = XMFLOAT3(0, 0, 0);

		TransformComponent tc;
		tc.m_scale = 1.0f;

		ms.m_bb.m_collisionType = CollisionType::player;

		//create components
		g->m_transformSystem.Create(eid, tc, pc); //transform system

		return eid;
	}

	static EntityId CreateGround(Game * g, DirectX::XMFLOAT3 position, float size)
	{
		EntityId eid = g->m_entities.add(vector<ISystem*>{
			&g->m_transformSystem,
			&g->m_renderingSystem
		});

		PhysicsComponent pc;
		pc.m_velocity = XMFLOAT3(0, 0, 0);
		pc.m_acceleration = XMFLOAT3(0, 0, 0);
		pc.m_gravity = false;

		TransformComponent tc;
		tc.m_position = position;
		tc.m_scale = size;

		//create components
		g->m_transformSystem.Create(eid, tc, pc); //transform system
		g->m_renderingSystem.Create(eid, &m_renderingComponents["groundPlane"]); //rendering system

		return eid;
	}
};