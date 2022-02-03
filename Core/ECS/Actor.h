#pragma once

#include "Core/Types/CommonTypes.h"
#include "Core/WorldSector.h"

// std
#include <vector>

namespace ECS 
{
	class ActorComponent; // forward-declaration

	/*	common base class for actor objects which can exist in a world
		instances could have a spatial presence and occupy 3D space */
	class Actor : public World::PhysicalElementInterface
	{
	public:
		Actor() = default;
		~Actor();

		ActorTransform transform;

		/* keeps track of all the components registered to this actor, may not preserve indices */
		std::vector<ActorComponent*> components;

		void componentDeletionSelfReport(ActorComponent* caller);

	protected:
		/* must be called for each user-declared component after initialization (e.g, in custom actor class constructor) */
		void registerComponent(ActorComponent* component);
		/* unregisters a component belonging to this actor, but does not delete it */
		void unregisterComponent(ActorComponent* component);
		/* deletes and unregisters a component (also disables deletion-self-reporting) */
		void destroyComponent(ActorComponent* component);

	private:
		void unregisterComponent_internal(const uint32_t& i, const bool& setNullParent);

		/* if false, user defined tick functions will be skipped for this actor and its components */
		bool hasTickEnabled = false;

		// returns index (temporary) if found, -1 otherwise
		uint32_t findComponent(ActorComponent* childComponent);
		

	};
} // namespace
