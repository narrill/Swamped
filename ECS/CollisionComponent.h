#pragma once
#include <DirectXMath.h>

//Component data struct
struct CollisionComponent {
	DirectX::XMFLOAT3 m_Obb[8];
};