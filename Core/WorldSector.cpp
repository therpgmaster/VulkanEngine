#include "Core/WorldSector.h"

namespace World 
{
	/*
	PhysicalElementInterface::~PhysicalElementInterface()
	{
		// object destroyed, remove element from octree
		if (linkedElement) 
		{
			
		}
	}

	void PhysicalElementInterface::reportPositionUpdated(const float& x,
												const float& y, const float& z) 
	{

	}

	void Octree::Branch::destroyEmptyElements()
	{
		Element* e = firstElement;
		Element* prev = nullptr;
		while (e) 
		{
			if (e->linkedObj) 
			{
				// not empty
				if (e->next) 
				{ 
					prev = e;
					e = e->next; // ++ (loop will end if null)
				}
			}
			else
			{
				// empty
				Element* next = e->next;
				if (!prev) { firstElement = next; }
				else { prev->next = next; }
				delete e;
				e = next; // (loop will end if null)
			}
		}
	}
	*/
} // namespace