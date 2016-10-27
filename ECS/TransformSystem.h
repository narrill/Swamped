#pragma once

#include "System.h"
#include "TransformComponent.h"

class TransformSystem : public System<TransformComponent> {
public:
	void Update(Game * g);
	size_t GetSize();
	size_t GetCount();
};