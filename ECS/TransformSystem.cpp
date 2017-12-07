#include "TransformSystem.h"
#include "Constructors.h"
void TransformSystem::Update(Game * game, float dt) {
	StartTimer();
	Collapse();
	
	//loop through all components
	parallel_for(size_t(0), m_collapsedCount, [&](unsigned int c) {
		XMVECTOR position;
		XMVECTOR rotation;
		XMVECTOR velocity;
		XMVECTOR acceleration;
		XMVECTOR rotationalVelocity;

		//load stuff
		position = XMLoadFloat3(&m_collapsedComponents1[c].m_component.m_position);
		rotation = XMLoadFloat4(&m_collapsedComponents1[c].m_component.m_rotation);
		velocity = XMLoadFloat3(&m_collapsedComponents2[c].m_component.m_velocity);
		acceleration = XMLoadFloat3(&m_collapsedComponents2[c].m_component.m_acceleration);
		if(m_collapsedComponents2[c].m_component.m_gravity)
			acceleration = XMVectorAdd(acceleration, XMLoadFloat3(&XMFLOAT3(0, m_gravity, 0)));
		rotationalVelocity = XMLoadFloat3(&m_collapsedComponents2[c].m_component.m_rotationalVelocity);
		if (XMVectorGetX(XMVector3Length(rotationalVelocity)) != 0)
		{
			rotation = XMQuaternionSlerp(rotation, XMQuaternionMultiply(rotation, XMQuaternionRotationAxis(rotationalVelocity, XMVectorGetX(XMVector3Length(rotationalVelocity)))), dt);
		}

		//do maths
		velocity += dt*acceleration;
		position += dt*velocity;

		//store stuff
		XMStoreFloat3(&m_components2[m_collapsedComponents2[c].m_handle].m_velocity, velocity);
		XMStoreFloat3(&m_components1[m_collapsedComponents1[c].m_handle].m_position, position);
		XMStoreFloat3(&m_components2[m_collapsedComponents2[c].m_handle].m_rotationalVelocity, rotationalVelocity);
		XMStoreFloat4(&m_components1[m_collapsedComponents1[c].m_handle].m_rotation, rotation);
	});
	StopTimer();
}

//Returns a matrix generated from the given component's properties
XMMATRIX TransformSystem::GetMatrix(TransformComponent& tc) {
	if (tc.m_scale == 0.0f)
		tc.m_scale = 1.0f;
	XMMATRIX scale = XMMatrixScaling(tc.m_scale, tc.m_scale, tc.m_scale);
	return XMMatrixMultiply(XMMatrixMultiply(scale,XMMatrixRotationQuaternion(XMLoadFloat4(&tc.m_rotation))), XMMatrixTranslationFromVector(XMLoadFloat3(&tc.m_position)));
}