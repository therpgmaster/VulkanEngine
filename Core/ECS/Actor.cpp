#include "Actor.h"
#include "ActorComponent.h"
#include "StaticMesh.h"
#include <cassert>

Actor::~Actor() 
{
	for (auto* c : components) { removeComponent(c); }
}

void Actor::registerComponent(ActorComponent& component, const bool& enableComponentTick, const bool& hasMesh)
{
	assert(!component.getParentActor() && "cannot register component which already has a parent");
	component.setParentActor(*this);
	components.push_back(&component); // add to array of registered components
}

void Actor::removeComponent(ActorComponent* component)
{
	assert(component && "tried to remove null component");
	assert(component->getParentActor() == this && "tried to remove component of another actor, bad practice");
	delete component; // component must notify its parent during deletion 
}

void Actor::componentPendingDeletion(ActorComponent* component) 
{
	for (uint32_t i = 0; i < components.size(); i++) 
	{
		if (components[i] == component) { components.erase(std::next(components.begin(), i)); }
	}
	components.shrink_to_fit();
}