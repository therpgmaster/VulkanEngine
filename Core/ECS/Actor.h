#pragma once

#include "Core/ECS/GenericDataObject.h"
#include "Core/Types/CommonTypes.h"
#include "Core/WorldSector.h"

// std
#include <vector>

class ActorComponent;
class StaticMesh;

// common base class for actor objects which can (but do not necessarily) exist in a world
// instances could have a spatial presence and occupy 3D space
class Actor : public GenericDataObject, public World::PhysicalElementInterface
{
public:
	Actor() = default;
	~Actor();

	ActorTransform transform;

	/* keeps track of all the components registered to this actor, may not preserve indices */
	std::vector<ActorComponent*> components;

	void componentPendingDeletion(ActorComponent* component);

protected:
	/* must be called manually for each defined member component after initialization */
	void registerComponent(ActorComponent& component, const bool& enableComponentTick, const bool& hasMesh);

	/* deletes and unregisters a component belonging to this actor */
	void removeComponent(ActorComponent* component);

private:
	/* if false, user defined tick functions will be skipped for this actor and its components */
	bool hasTickEnabled;

	struct ActorTransform 
	{
		float x;
		float y;
		float z;
	};

};
