#pragma once
struct IndexedItem
{
	int HeapIndex;
	double NodeWeight;
	int NodeIndex;

	IndexedItem(int nodeIndex, double nodeWeight)
	{
		NodeIndex = nodeIndex;
		HeapIndex = -1;
		NodeWeight = nodeWeight;
	}
	IndexedItem() 
	{

	}
};