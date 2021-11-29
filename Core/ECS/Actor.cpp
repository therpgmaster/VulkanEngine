#include "Actor.h"
#include "ActorComponent.h"
#include "StaticMesh.h"

void Actor::registerComponent(ActorComponent& component, const bool& enableComponentTick, const bool& hasMesh)
{
	component.setParentActor(*this);
	components.push_back(&component); // add to vector of registered components
	if (hasMesh) { meshComponents.push_back( (StaticMesh*) &component ); }
	if (component.getComponentHasPhysicalPresence() || hasMesh) 
	{ hasPhysicalPresence = true; }
}

void Actor::unregisterComponent(ActorComponent& component) 
{
	// move the destroyed component to end of vector and remove it
	std::vector<ActorComponent*>::iterator newEnd = std::remove(components.begin(), components.end(), &component);
	components.erase(newEnd, components.end());
	components.shrink_to_fit(); // free unused memory
}

void Actor::removeComponent(ActorComponent* component)
{
	if (component == nullptr) { return; }
	unregisterComponent(*component);
	delete component;
}

std::vector<StaticMesh*> Actor::getAllMeshComponents()
{
	if (meshComponents.empty()) { return std::vector<StaticMesh*>{ nullptr }; }
	return meshComponents;
}