#include "Core/Types/Octree.h"

void Octree::Node::createChildren() 
{
	for (uint32_t i = 0; i < 8; i++) { children[i] = new Node(); }
}

