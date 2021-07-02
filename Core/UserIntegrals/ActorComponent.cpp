#include "ActorComponent.h"
#include "GenericActor.h"

// std
#include <stdexcept>

ActorComponent::~ActorComponent() 
{
	if (hasOwner) 
	{
		// when this component is destroyed, we must report it to the parent actor
		if (parentActor == nullptr) 
		{ throw std::runtime_error("destructor called on component with unknown parent actor"); }
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