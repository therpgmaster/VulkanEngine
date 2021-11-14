#pragma once

#include "GenericDataObject.h"

// std
#include <vector>

class ActorComponent;
class StaticMesh;

// common base class for actor objects which can (but do not necessarily) exist in a world
// instances could have a spatial presence and occupy 3D space
class GenericActor : public GenericDataObject
{
public:
	/* keeps track of all the components registered to this actor */
	std::vector<ActorComponent*> components;

	/* holds pointers to only the 3D primitive components registered to this actor */
	std::vector<StaticMesh*> meshComponents;

	const bool& getHasPhysicalPresence() { return hasPhysicalPresence; }

	std::vector<StaticMesh*> getAllMeshComponents();

	void componentPendingDeletion(ActorComponent* component) 
	{ if (component != nullptr) { unregisterComponent(*component); } }

protected:
	/* must be called manually for each defined member component after initialization */
	void registerComponent(ActorComponent& component, const bool& enableComponentTick, const bool& hasMesh);
	/* must be called for components when they are deleted, so that actors do not attempt to access them */
	void unregisterComponent(ActorComponent& component);
	/* deletes and unregisters the component pointed to */
	void removeComponent(ActorComponent* component);

private:
	/* if false, the object has no transform or representation in physical space by default 
	this should not be changed after initialization */
	bool hasPhysicalPresence;
	/* if false, user defined tick functions will be skipped for this actor and its components */
	bool hasTickEnabled;

};
