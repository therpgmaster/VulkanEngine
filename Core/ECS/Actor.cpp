#include "Core/ECS/Actor.h"
#include "Core/ECS/ActorComponent.h"
#include "Core/ECS/Primitive.h"
#include <cassert>

namespace ECS 
{
	Actor::~Actor()
	{}

	void Actor::registerComponent(ActorComponent* component)
	{
		if (component->getParentActor() == this) { return; }
		assert(!component->getParentActor() && "cannot register component, it already has a parent");
		component->parentActor = this;
		components.push_back(component); // add to record
	}

	void Actor::unregisterComponent(ActorComponent* component)
	{
		assert(component->getParentActor() == this && "tried to unregister component from another actor, bad practice");
		const auto i = findComponent(component); // search
		if (i < 0) { return; }
		unregisterComponent_internal(i, true); // remove from record
	}

	void Actor::unregisterComponent_internal(const uint32_t& i, const bool& setNullParent)
	{
		// internal version of unregisterComponent, this one assumes index is valid
		if (setNullParent) { components[i]->parentActor = nullptr; }
		components.erase(std::next(components.begin(), i));
		components.shrink_to_fit();
	}

	void Actor::destroyComponent(ActorComponent* component)
	{
		assert((!component->getParentActor() || component->getParentActor() == this) 
					&& "tried to destroy component of another actor, bad practice");
		const auto i = findComponent(component);
		if (i < 0) { return; }
		component->parentActor = nullptr; // prevents self-reporting
		delete component; // destroy
		unregisterComponent_internal(i, false);
	}

	void Actor::componentDeletionSelfReport(ActorComponent* caller)
	{
		// component reported sudden pending deletion
		const auto i = findComponent(caller);
		if (i >= 0) { unregisterComponent_internal(i, true); }
	}

	uint32_t Actor::findComponent(ActorComponent* childComponent)
	{
		if (components.empty()) { return -1; }
		for (uint32_t i = 0; i < components.size(); i++)
		{ if (components[i] == childComponent) { return i; } }
		return -1;
	}

} // namespace