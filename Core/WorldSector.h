#pragma once

#include "Core/ECS/Actor.h"

// std
#include <cstdint>
#include <cmath>
#include <stdexcept>

namespace World
{
	// at least 7 decimal digits of float precision assumed (32-bit)
	#define SECTOR_MAX 1048576 // ~10km if unit = cm, millimeter precision guaranteed
	#define SECTOR_TREE_MIN_BRANCH_WIDTH 10

	class Octree;
	class PhysicalElementInterface
	{
		/*	the element interface should be inherited by all
			objects with a physical presence (physics/collision)
			it is used for communicating data with a spatial data structure */
	protected:
		friend class Octree;
		// pointer to corresponding data element in the octree
		//Octree::Element* linkedElement;
		// reports a change in physical location, used to organize data by location
		//void reportPositionUpdated(const float& x, const float& y, const float& z);
		
	public:
		//~PhysicalElementInterface();
	};

	class Octree
	{
	public:
		struct OctreeProperties
		{
			// physical width, must be power of two and below SECTOR_MAX
			uint32_t rootWidth = 409600; // ~ 4km
			// branches with an element count above this value will split
			uint32_t branchThreshold = 250;
		};

		Octree(const OctreeProperties& treeProps) : props{ treeProps }
		{
			if (treeProps.rootWidth % 2 != 0) { throw std::runtime_error("octree width must be a power of two "); }
			if (treeProps.rootWidth > SECTOR_MAX) { throw std::runtime_error("octree width cannot exceed SECTOR_MAX"); }
			root = new Branch();
		}
		~Octree() { delete root; }

		/*Actor* addActor(const Actor& actor)
		{
			Element* e = new Element();
			e->linkedObj = new Actor(actor);
			root->addElement(e);
		}*/

	protected:
		friend class PhysicalElementInterface;
		/*	an octree element does not contain the object it represents,
			but is linked to it, if the element is deleted we also delete the
			associated object to ensure that it doesn't become leaked memory */
		struct Element
		{
			Element* next;
			PhysicalElementInterface* linkedObj;
			~Element() { if (linkedObj) { delete linkedObj; } }
		};

		struct Branch
		{
			Branch* parentBranch = nullptr;
			Element* firstElement = nullptr;
			Branch* subBranches[8] = {};

			class Iterator
			{
			public:
				Iterator(Element* e) : elem{ e } {};
				Iterator& operator++() { elem = (elem && elem->next) ? elem->next : nullptr; return *this; }
				bool operator!=(const Iterator& other) const { return &elem != &other.elem; }
				const Element& operator*() const { return *elem; }
			private:
				Element* elem;
			};
			Iterator begin() const { return Iterator(firstElement); }
			Iterator end() const { return Iterator(nullptr); }

			~Branch() { destroyAllElements(); destroySubBranches(); }
			void destroyEmptyElements();
			void destroyAllElements()
			{
				auto* next = firstElement;
				while (next)
				{
					auto* p = next;
					next = next->next; // next
					delete p;
				}
				firstElement = nullptr;
			}
			void destroySubBranches() { for (Branch* b : subBranches) { if (b) { delete b; } } }
			void addElement(Element* element)
			{
				if (!firstElement) { firstElement = element; return; }
				auto* p = firstElement;
				while (p->next) { p = p->next; }
				p->next = element;
			}
			uint32_t level()
			{
				Branch* p = this;
				uint32_t count = 0;
				while (p->parentBranch) { p = p->parentBranch; count++; }
				return count;
			}
			// checks and re-assigns elements which have moved to another branch
			bool updateElementPositions()
			{
				auto l = level();
				for (auto& e : *this)
				{
				}
			}
		};

		OctreeProperties props;
		Branch* root;

		uint32_t getBranchWidth(const uint32_t& level) { return props.rootWidth / (2 ^ level); }


	};

} // namespace

