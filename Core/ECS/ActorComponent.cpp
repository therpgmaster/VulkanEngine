#include "ActorComponent.h"
#include "Core/ECS/Actor.h"
#include <cassert>

namespace ECS 
{
	ActorComponent::~ActorComponent() 
	{
		if (parentActor) { parentActor->componentDeletionSelfReport(this); }
	}

} // namespace