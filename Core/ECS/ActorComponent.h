#pragma once

#include "Core/ECS/GenericDataObject.h"
#include "Core/Types/CommonTypes.h"

#include <glm/glm.hpp>

class Actor;

class ActorComponent : public GenericDataObject
{
	Actor* parentActor = nullptr;

public:
	ActorComponent() {};
	~ActorComponent();

	ActorComponent& operator=(const ActorComponent&) = default;
	ActorComponent& operator=(ActorComponent&&) = default;
	bool operator==(ActorComponent* comparePtr) const { return comparePtr == this; }

	/* should be called once for every component, using Actor::registerComponent() */
	void setParentActor(Actor& parent);

	Actor* getParentActor() { return parentActor; }

	/* used by parent actor to check if this component has user defined logic to run on tick */
	bool getComponentCanTick() const { return canTick; }

	Transform transform{};

	virtual void tick(const float& deltaTime) {};

protected:
	/* if false, parent actor will not attempt to call user defined tick functions on this component */
	bool canTick = true;
};
