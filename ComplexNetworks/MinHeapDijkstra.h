#pragma once
//#include "Dijkstra.h"
#include "IndexedItem.h"
class MinHeapDijkstra
{
public:
	int Count;
	vector<IndexedItem> items; //items in the heap

	MinHeapDijkstra(int maxLength, IndexedItem &first)
	{
		//vector<IndexedItem> items(maxLength);
		items.resize(maxLength);
		Count = 1;
			items[0] = first;
			first.HeapIndex = 0;
			//heapify();
	}

	void addItem(IndexedItem &i)
	{
		items[Count] = i;
		i.HeapIndex = Count;
		items[Count].HeapIndex = Count; // added this line
		Count++;
		heapifyUp(Count - 1);
	}
	bool isEmpty() { return Count == 0; }
	
	void decreaseKey(int pos)
	{
		if (Count <= 1)
			return;
		int parent = (pos - 1) / 2;
		//if greater than parent
		if (items[parent].NodeWeight > items[pos].NodeWeight)
			heapifyUp(pos);
		else
			heapifyDown(pos);
	}

	IndexedItem peek() { return items[0]; }

	void heapify()
	{
		int start = (Count - 2) / 2; //Parent of last element

		while (start >= 0)
		{
			heapifyDown(start);
			start--;
		}
	}

	IndexedItem extractMin()
	{
		IndexedItem min = items[0]; //Get min item

									//set new min to last item
		items[0] = items[--Count];
		items[0].HeapIndex = 0;

		//Heapify
		heapifyDown(0);

		//return
		return min;
	}

	int heapifyUp(int pos)
	{
		if (pos >= Count) return -1;

		while (pos > 0)
		{
			int parent = (pos - 1) / 2;
			if (items[parent].NodeWeight > items[pos].NodeWeight)
			{
				swap(parent, pos);
				pos = parent;
			}
			else break;
		}

		return pos;
	}

	void heapifyDown(int pos)
	{
		if (pos >= Count) return;

		while (true)
		{
			int smallest = pos;
			int leftChild = 2 * pos + 1;
			int rightChild = leftChild + 1;

			if (leftChild < Count && items[smallest].NodeWeight > items[leftChild].NodeWeight)
			{
				smallest = leftChild;
			}
			if (rightChild < Count && items[smallest].NodeWeight > items[rightChild].NodeWeight)
			{
				smallest = rightChild;
			}
			if (smallest != pos)
			{
				swap(smallest, pos);
				pos = smallest;
			}
			else break; // if pos was not updated, we are done
		}
	}

	void swap(int a, int b)
	{
		IndexedItem temp = items[a];
		temp.HeapIndex = b; //change the indicies in the map
		items[a] = items[b];
		items[b] = temp;
		items[a].HeapIndex = a;//change the indicies in the map
	}

};