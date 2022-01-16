#include "ActorComponent.h"
#include "Actor.h"
#include <cassert>


ActorComponent::~ActorComponent() 
{
	if (parentActor)
	{
		// when this component is destroyed, we must report it to the parent actor
		parentActor->componentPendingDeletion(this);
	}
}

void ActorComponent::setParentActor(Actor& parent) 
{
	assert(!parentActor && "cannot set parent actor, component already has a parent");
	if (!parentActor) { parentActor = &parent; }
}