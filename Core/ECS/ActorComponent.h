#pragma once
#include "Core/Types/CommonTypes.h"

#include <glm/glm.hpp>

namespace ECS 
{
	class Actor; // forward-declaration

	/* base class for components */
	class ActorComponent
	{
		friend class Actor; // allow actor (parent) full access
	public:
		ActorComponent() {};
		~ActorComponent();
	
		ActorComponent& operator=(const ActorComponent&) = default;
		ActorComponent& operator=(ActorComponent&&) = default;
		bool operator==(const ActorComponent& comparePtr) const { return &comparePtr == this; }
	
		
		Actor* getParentActor() { return parentActor; }
	
		Transform transform{}; // location/rotation/scale relative to parent
	
		virtual void tick(const float& deltaTime) {};

	private:
		Actor* parentActor = nullptr; // set by parent on registration
		bool canTick = true; // if true, parent actor will attempt to call user-defined tick logic
		

	};
} // namespace


