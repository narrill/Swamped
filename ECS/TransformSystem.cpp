#include "TransformSystem.h"
#include "Constructors.h"
void TransformSystem::Update(Game * g, float dt) {
	for (unsigned int c = 0; c < m_components1.size(); c++) {
		if (m_componentData[c].m_active && rand() % 2 == 1)
			g->RemoveEntity(m_componentData[c].GetEntityId());
	}
	unsigned int newComponents = rand() % 10000;
	for (unsigned int c = 0; c < newComponents; c++)
	{
		PhysicsComponent pc;
		pc.m_velocity = XMFLOAT3(0, 0, 0);
		pc.m_acceleration = XMFLOAT3(0, 0, 0);
		XMStoreFloat4(&pc.m_rotationalVelocity, XMQuaternionRotationRollPitchYaw(1, 0, 0));
		XMStoreFloat4(&pc.m_rotationalAcceleration, XMQuaternionRotationRollPitchYaw(0,0,0));
		Constructors::CreateTransform(g, {}, pc, {
			{
				{ -1,-1,-1 },
				{ -1,-1,1 },
				{ -1,1,-1 },
				{ -1,1,1 },
				{ 1,-1,-1 },
				{ 1,-1,1 },
				{ 1,1,-1 },
				{ 1,1,1 }
			}
		});
	}

	//Movement
	//Pre-allocate stuff
	XMVECTOR position;
	XMVECTOR rotation;
	XMVECTOR velocity;
	XMVECTOR acceleration;
	XMVECTOR rotationalVelocity;
	XMVECTOR rotationalAcceleration;
	//loop through all components
	for (unsigned int c = 0; c < m_components1.size(); c++) {
		if (m_componentData[c].m_active) //only do for active components
		{
			//load stuff
			position = XMLoadFloat3(&m_components1[c].m_position);
			rotation = XMLoadFloat4(&m_components1[c].m_rotation);
			velocity = XMLoadFloat3(&m_components2[c].m_velocity);
			acceleration = XMLoadFloat3(&m_components2[c].m_acceleration);
			rotationalVelocity = XMLoadFloat4(&m_components2[c].m_rotationalVelocity);
			rotationalAcceleration = XMLoadFloat4(&m_components2[c].m_rotationalAcceleration);
			//do maths
			velocity += dt*acceleration;
			position += dt*velocity;
			rotationalVelocity = XMQuaternionMultiply(rotationalVelocity, dt*rotationalAcceleration);
			rotation = XMQuaternionMultiply(rotation, dt*rotationalVelocity);
			//store stuff
			XMStoreFloat3(&m_components2[c].m_velocity, velocity);
			XMStoreFloat3(&m_components1[c].m_position, position);
			XMStoreFloat4(&m_components2[c].m_rotationalVelocity, rotationalVelocity);
			XMStoreFloat4(&m_components1[c].m_rotation, rotation);
		}
	}
}

XMMATRIX TransformSystem::GetMatrix(TransformComponent tc) {
	return XMMatrixMultiply(XMMatrixTranslationFromVector(XMLoadFloat3(&tc.m_position)), XMMatrixRotationQuaternion(XMLoadFloat4(&tc.m_rotation)));
}

size_t TransformSystem::GetSize() {
	return m_components1.size();
}

size_t TransformSystem::GetCount() {
	return m_components1.count();
}