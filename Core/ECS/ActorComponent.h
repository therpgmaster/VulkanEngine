#pragma once

#include "Core/ECS/GenericDataObject.h"
#include "Core/Types/CommonTypes.h"
// glm
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

class Actor;

class ActorComponent : public GenericDataObject
{
public:
	ActorComponent() {};
	~ActorComponent();

	ActorComponent& operator=(const ActorComponent&) = default;
	ActorComponent& operator=(ActorComponent&&) = default;
	bool operator==(ActorComponent* comparePtr) const { return comparePtr == this; }

	/* must be called immediately when the component is created */
	void setParentActor(Actor& newParentActor) { parentActor = &newParentActor; }

	/* attempts to make this component standalone, returns false if unsuccessful 
	components already assigned to an actor cannot be orphaned */
	bool setNoParent();

	Actor* getParentActor() { return parentActor; }

	/* used by parent actor to decide if this component has user defined logic to run on tick */
	bool getComponentCanTick() const { return canTick; }

	bool getComponentHasPhysicalPresence() const { return hasPhysicalPresence; }

	bool getComponentHasOwner() const { return hasOwner; }

	Transform transform{};

	virtual void tick(const float& deltaTime) {};

protected:
	/* if false, parent actor will not attempt to call user defined tick functions on this component */
	bool canTick = true;
	/* should be true if the component type has any primitives which can be visually rendered */
	bool hasPhysicalPresence = false;

private:
	/* if hasOwner is false, the parentActor pointer is allowed to remain null */
	Actor* parentActor = nullptr;
	bool hasOwner = true;

};
