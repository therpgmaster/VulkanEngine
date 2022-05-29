#pragma once
#include <cstddef>



template<typename T>
class Octree
{
	struct Element
	{
		Element* next = nullptr;
		T* obj = nullptr;
		operator T* () const { return obj; }
	};
	class Node
	{
		Node* children[8] = {}; // sub-nodes
		Element* firstElem = nullptr; // elements list start
		struct ElemsIterator
		{
			using iterator_category = std::forward_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = Element;
			using pointer = Element*;
			using reference = Element&;

			ElemsIterator(pointer p) : p{ p } {}; // ctor
			reference operator*() const { return *p; }
			pointer operator->() { return p; }
			ElemsIterator& operator++() { p = p->next; return *this; }
			ElemsIterator operator++(int) { ElemsIterator tmp = *this; ++(*this); return tmp; }

			friend bool operator== (const ElemsIterator& a, const ElemsIterator& b) { return a.p == b.p; };
			friend bool operator!= (const ElemsIterator& a, const ElemsIterator& b) { return a.p != b.p; };

		private:
			pointer p;
		};
		ElemsIterator begin() { return ElemsIterator(firstElem); }
		ElemsIterator end() { return ElemsIterator(nullptr); }
	public:
		bool isEmpty() const { return !firstElem; }
		bool isLeaf() const { return !children[0]; }
		void createChildren();
	};


	Node root;

public:
	Octree(uint32_t rootExtent);


};