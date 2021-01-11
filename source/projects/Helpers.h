#pragma once

struct NodeForRender
{
	int nodeIdx;	
	// Additional Information
	float costSoFar{};
	float fCost{};
	
	bool operator<(const NodeForRender& other) const
	{
		return costSoFar < other.costSoFar || fCost < other.fCost;
	}

	bool operator==(const NodeForRender& other) const
	{
		return costSoFar == other.costSoFar || fCost == other.fCost;
	}
};
