#include "ActorComponent.h"
#include "Actor.h"


ActorComponent::~ActorComponent() 
{
	if (hasOwner) 
	{
		// when this component is destroyed, we must report it to the parent actor
		parentActor->componentPendingDeletion(this);
	}
}

bool ActorComponent::setNoParent() 
{
	if (parentActor == nullptr)
	{
		hasOwner = false;
		return true;
	}
	return false;
}