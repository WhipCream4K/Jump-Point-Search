#pragma once

//#include "framework/EliteAI/EliteGraphs/EIGraph.h"
//namespace Elite
//{
//	template<typename T_NodeType, typename T_ConnectionType>
//	class Elite::IGraph<T_NodeType, T_ConnectionType>;
//}

template<typename T_NodeType, typename T_ConnectionType>
class JumpPointSearch
{
public:

	JumpPointSearch(
		Elite::IGraph<T_NodeType, T_ConnectionType>* pGraph,
		Elite::Heuristic pHeuristicFunc,
		int graphColCount,
		int graphRowCount);

	enum class Direction
	{
		Left,
		Right,
		TopRight,
		TopLeft,
		BottomRight,
		BottomLeft,
		Up,
		Down,
		None
	};

	struct NodeRecord
	{
		T_NodeType* pNode = nullptr;
		T_ConnectionType* pConnection = nullptr;
		Direction parentDirection = Direction::None;
		float gCost{};
		float hCost{};

		bool operator==(const NodeRecord& other) const
		{
			return pNode == other.pNode
				&& pConnection == other.pConnection
				&& gCost == other.gCost
				&& hCost == other.hCost;
		}

		bool operator<(const NodeRecord& other) const
		{
			return gCost + hCost < other.gCost + other.hCost;
		}
	};


	// Utilities
	std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pEndNode);

private:

	// weak reference to graph object
	Elite::IGraph<T_NodeType, T_ConnectionType>* m_pGraph;

	std::vector<NodeRecord> m_OpenLists;
	std::vector<NodeRecord> m_VisitedJumpPoints;
	Elite::Heuristic m_pHeuristicFunction;
	int m_GraphColumnsCount;
	int m_GraphRowsCount;

	float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

	// Can only retrieve direction to adjacent node
	Direction GetDirection(T_ConnectionType* pConnection) const;
	bool IsNodeStillInGrid(int nodeIdx);

	/// <summary>
	///	check horizontal line for any interesting node ahead
	/// </summary>
	/// <param name="nodeIdx"> current node index </param>
	/// <param name="dir">horizontal direction being +1 or -1 (right or left)</param>
	/// <returns>returns true if it found forced neighbour</returns>
	bool HorizontalSearch(
		int nodeIdx,
		int dir,
		const std::vector<T_NodeType*>& grid,
		T_NodeType* pEndNode,
		T_NodeType* pStartNode,
		float parentGCost);

	/// <summary>
	/// check vertical line for any interesting node ahead
	/// </summary>
	/// <param name="nodeIdx">current node index</param>
	/// <param name="dir">vertical direction being +1 or -1 ( top or bottom )</param>
	/// <param name="grid"></param>
	/// <param name="pEndNode"></param>
	/// <param name="pStartNode"></param>
	/// <returns>returns true if found forced neighbour(s)</returns>
	bool VerticalSearch(
		int nodeIdx,
		int dir,
		const std::vector<T_NodeType*>& grid,
		T_NodeType* pEndNode,
		T_NodeType* pStartNode,
		float parentGCost);

	/// <summary>
	/// check diagonal line for any interesting node ahead
	/// </summary>
	/// <param name="nodeIdx">current node index</param>
	/// <param name="horDir">horizontal value for the diagonal search +1 or -1 (right or left)</param>
	/// <param name="verDir">vertical value for the diagonal search +1 or -1 (up or down)</param>
	/// <param name="grid"></param>
	/// <param name="pEndNode"></param>
	/// <param name="pStartNode"></param>
	/// <returns>returns true if found forced neighbour(s)</returns>
	bool DiagonalSearch(
		int nodeIdx,
		int horDir,
		int verDir,
		const std::vector<T_NodeType*>& grid,
		T_NodeType* pEndNode,
		T_NodeType* pStartNode,
		float parentGCost);
};

template <typename T_NodeType, typename T_ConnectionType>
JumpPointSearch<T_NodeType, T_ConnectionType>::JumpPointSearch(
	Elite::IGraph<T_NodeType, T_ConnectionType>* pGraph,
	Elite::Heuristic pHeuristicFunc,
	int graphColCount,
	int graphRowCount)
	: m_pGraph(pGraph)
	, m_OpenLists()
	, m_VisitedJumpPoints()
	, m_pHeuristicFunction(pHeuristicFunc)
	, m_GraphColumnsCount(graphColCount)
	, m_GraphRowsCount(graphRowCount)
{
}

template <typename T_NodeType, typename T_ConnectionType>
std::vector<T_NodeType*> JumpPointSearch<T_NodeType, T_ConnectionType>::FindPath(
	T_NodeType* pStartNode,
	T_NodeType* pEndNode)
{
	// always early exits
	if (pStartNode == pEndNode || !pStartNode || !pEndNode)
		return std::vector<T_NodeType*>{pStartNode};


	auto gridArray = m_pGraph->GetAllNodes();

	// First Condition ( No parent, adds all available path from the start )
	for (const auto& connection : m_pGraph->GetNodeConnections(pStartNode->GetIndex()))
	{
		if (connection->IsValid())
		{
			T_NodeType* pointedNode{ m_pGraph->GetNode(connection->GetTo()) };
			const float connectionCost{ connection->GetCost() };

			NodeRecord startNode{};

			startNode.hCost = GetHeuristicCost(pointedNode, pEndNode);
			startNode.gCost = connectionCost;
			startNode.pConnection = connection;
			startNode.pNode = pointedNode;
			startNode.parentDirection = GetDirection(connection);

			m_OpenLists.emplace_back(startNode);
		}
	}



	// HOT ZONE!!
	while (!m_OpenLists.empty())
	{
		// 0. determine the best node and direction to start with ( probably needed a heuristic test )
		const auto cIt{ std::min_element(m_OpenLists.begin(),m_OpenLists.end()) };
		NodeRecord jumpPoint{ *cIt };

		// 1. search HORIZONTALLY for an 'interesting' node
		// beware for the up and down blocked node
		// if found one depend on the direction we are going for
		// add the diagonal node to the open list

		T_NodeType* estimateBestNode{};
		float estimatedBestTravelCost{ FLT_MAX };

		for (const auto& connection : m_pGraph->GetNodeConnections(jumpPoint.pNode))
		{
			if (connection->IsValid())
			{
				T_NodeType* pointedNode = m_pGraph->GetNode(connection->GetTo());
				const float costSofar{ jumpPoint.gCost + connection->GetCost() };
				if (costSofar + GetHeuristicCost(pointedNode, pEndNode) < estimatedBestTravelCost)
					estimateBestNode = pointedNode;
			}
		}

		if (jumpPoint.pNode != pEndNode)
		{
			// remove the visited jump point out of the open list
			m_OpenLists.erase(cIt);

			int pointedIdx{ jumpPoint.pNode->GetIndex() };
			switch (jumpPoint.parentDirection)
			{
			case Direction::Left:
				HorizontalSearch(pointedIdx, -1, gridArray, pEndNode, pStartNode, jumpPoint.gCost);		break;
			case Direction::Right:
				HorizontalSearch(pointedIdx, -1, gridArray, pEndNode, pStartNode, jumpPoint.gCost);		break;
			case Direction::Up:
				VerticalSearch(pointedIdx, 1, gridArray, pEndNode, pStartNode, jumpPoint.gCost);			break;
			case Direction::Down:
				VerticalSearch(pointedIdx, -1, gridArray, pEndNode, pStartNode, jumpPoint.gCost);			break;
			case Direction::TopLeft:
				DiagonalSearch(pointedIdx, -1, 1, gridArray, pEndNode, pStartNode, jumpPoint.gCost);		break;
			case Direction::TopRight:
				DiagonalSearch(pointedIdx, 1, -1, gridArray, pEndNode, pStartNode, jumpPoint.gCost);		break;
			case Direction::BottomLeft:
				DiagonalSearch(pointedIdx, -1, -1, gridArray, pEndNode, pStartNode, jumpPoint.gCost);		break;
			case Direction::BottomRight:
				DiagonalSearch(pointedIdx, 1, -1, gridArray, pEndNode, pStartNode, jumpPoint.gCost);		break;
			default:
				break;
			}
		}
		else
		{
			// TODO: Do the path tracing
			switch(jumpPoint.pConnection)
		}
	}



	return std::vector<T_NodeType*>{pStartNode};
}

template <typename T_NodeType, typename T_ConnectionType>
float JumpPointSearch<T_NodeType, T_ConnectionType>::GetHeuristicCost(
	T_NodeType* pStartNode,
	T_NodeType* pEndNode) const
{
	const Elite::Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
	return m_pHeuristicFunction(abs(toDestination.x), abs(toDestination.y));
}

template <typename T_NodeType, typename T_ConnectionType>
typename JumpPointSearch<T_NodeType, T_ConnectionType>::Direction JumpPointSearch<T_NodeType, T_ConnectionType>::
GetDirection(T_ConnectionType* pConnection) const
{
	const int fromIdx{ pConnection->GetFrom() };
	const int toIdx{ pConnection->GetTo() };

	const int dirIdx{ toIdx - fromIdx };

	// Man, this is really ugly
	if (dirIdx == 1)
		return Direction::Right;
	if (dirIdx == -1)
		return Direction::Left;
	if (dirIdx == m_GraphColumnsCount + 1)
		return Direction::TopRight;
	if (dirIdx == m_GraphColumnsCount - 1)
		return Direction::TopLeft;
	if (dirIdx == m_GraphColumnsCount)
		return Direction::Up;
	if (dirIdx == -(m_GraphColumnsCount + 1))
		return Direction::BottomRight;
	if (dirIdx == -(m_GraphColumnsCount - 1))
		return Direction::BottomLeft;
	if (dirIdx == -m_GraphColumnsCount)
		return Direction::Down;

	return Direction::None;
}

template <typename T_NodeType, typename T_ConnectionType>
bool JumpPointSearch<T_NodeType, T_ConnectionType>::IsNodeStillInGrid(int nodeIdx)
{
	return nodeIdx < (m_GraphColumnsCount* m_GraphRowsCount) - 1 && nodeIdx >= 0;
}

template <typename T_NodeType, typename T_ConnectionType>
bool JumpPointSearch<T_NodeType, T_ConnectionType>::HorizontalSearch(
	int nodeIdx,
	int dir,
	const std::vector<T_NodeType*>& grid,
	T_NodeType* pEndNode,
	T_NodeType* pStartNode,
	float parentGCost)
{
	int traverseIndex{ nodeIdx };
	float travelDistance{ parentGCost };
	T_ConnectionType* parentConnection{};

	while (true)
	{
		// search along the grid
		const int currIdx{ traverseIndex };
		const int nextIdx{ traverseIndex += dir };

		if (!IsNodeStillInGrid(nextIdx))
			return false;

		// check ahead if it's out of bound
		const int currCol{ currIdx % m_GraphColumnsCount };
		const int nextCol{ nextIdx % m_GraphColumnsCount };
		if (abs(currCol - nextCol) > 1)
			return false;

		// update parent node info
		parentConnection = m_pGraph->GetConnection(currIdx, nextIdx);
		travelDistance += parentConnection->GetCost();

		// check if the next node is the end node
		if (grid[nextIdx] == pEndNode)
		{
			NodeRecord endNode{};

			endNode.pNode = pEndNode;
			endNode.pConnection = parentConnection;
			endNode.parentDirection = dir < 0 ? Direction::Left : Direction::Right;
			endNode.gCost = travelDistance;
			endNode.hCost = 0.0f;

			m_OpenLists.emplace_back(endNode);

			return true;
		}

		// check for top and bottom obstacles
		int topIdx{ nextIdx + m_GraphColumnsCount };
		int bottomIdx{ nextIdx - m_GraphColumnsCount };

		// GetConnection() already return us nullptr if there's no connection to it
		T_ConnectionType* topNode{ IsNodeStillInGrid(topIdx) ?
			m_pGraph->GetConnection(nextIdx,topIdx) :
			nullptr };

		T_ConnectionType* bottomNode{ IsNodeStillInGrid(bottomIdx) ?
			m_pGraph->GetConnection(nextIdx,bottomIdx) :
			nullptr };

		bool foundNeighbour{};

		if (topNode)
		{
			// Check if this connection is blocked
			if (!topNode->IsValid())
			{
				// if it's a blockade then we add the forced neighbour in our open list
				// provide that the node itself is also not a blockade
				int topForcedNeighbourIdx{ nextIdx + m_GraphColumnsCount + dir };

				T_ConnectionType* topForcedNeighbour{ IsNodeStillInGrid(topForcedNeighbourIdx) ?
					m_pGraph->GetConnection(nextIdx,topForcedNeighbourIdx) :
					nullptr
				};

				if (topForcedNeighbour)
				{
					if (topForcedNeighbour->IsValid())
					{
						//NodeRecord interestingNode{};
						//interestingNode.pNode = topForcedNeighbour;
						//interestingNode.pConnection = m_pGraph->GetConnection(traverseIndex, topForcedNeighbourIdx);
						//interestingNode.parentDirection = (dir < 0 ? Direction::TopLeft : Direction::TopRight);

						//// TODO: find a way to utilize gCost
						//interestingNode.gCost = 0.0f;

						//interestingNode.hCost = GetHeuristicCost(topForcedNeighbour, pEndNode);

						//m_OpenLists.emplace_back(interestingNode);
						foundNeighbour = true;
					}
				}
			}
		}

		if (bottomNode)
		{
			// Check if this connection is blocked
			if (!bottomNode->IsValid())
			{
				// same with the top forced neighbour one
				int bottomForcedNeighbourIdx{ nextIdx - m_GraphColumnsCount + dir };

				T_ConnectionType* bottomForcedNeighbour{ IsNodeStillInGrid(bottomForcedNeighbourIdx) ?
					m_pGraph->GetConnection(nextIdx,bottomForcedNeighbourIdx) :
					nullptr
				};

				if (bottomForcedNeighbour)
				{
					if (bottomForcedNeighbour->IsValid())
					{
						//NodeRecord interestingNode{};
						//interestingNode.pNode = bottomForcedNeighbour;
						//interestingNode.pConnection = m_pGraph->GetConnection(traverseIndex, bottomForcedNeighbourIdx);
						//interestingNode.parentDirection = (dir < 0 ? Direction::BottomLeft : Direction::BottomRight);

						//// TODO: find a way to utilize gCost
						//interestingNode.gCost = 0.0f;

						//interestingNode.hCost = GetHeuristicCost(bottomForcedNeighbour, pEndNode);

						//m_OpenLists.emplace_back(interestingNode);
						foundNeighbour = true;
					}
				}
			}
		}


		if (foundNeighbour)
		{
			// add all the path available from this node
			if (dir < 0) // left
			{
				// check for top left, left, bottom left if the path is available
				// if so then add it to the open list
			}

			NodeRecord interestingNode{};
			interestingNode.pNode = grid[nextIdx];
			interestingNode.parentDirection = dir < 0 ? Direction::Left : Direction::Right;
			interestingNode.pConnection = parentConnection;
			interestingNode.hCost = GetHeuristicCost(grid[nextIdx], pEndNode);

			interestingNode.gCost = travelDistance;

			m_OpenLists.emplace_back(interestingNode);

			return true;
		}
	}
}

template <typename T_NodeType, typename T_ConnectionType>
bool JumpPointSearch<T_NodeType, T_ConnectionType>::VerticalSearch(
	int nodeIdx,
	int dir,
	const std::vector<T_NodeType*>& grid,
	T_NodeType* pEndNode,
	T_NodeType* pStartNode,
	float parentGCost)
{
	int traverseIndex{ nodeIdx };
	float travelDistance{ parentGCost };
	T_ConnectionType* parentConnection{};

	while (true)
	{
		// search along the grid
		const int currIdx{ traverseIndex };
		const int nextIdx{ traverseIndex += m_GraphColumnsCount * dir };

		if (!IsNodeStillInGrid(nextIdx))
			return false;

		// return if it's out of bound
		//if (traverseIndex % m_GraphRowsCount == 0)
		//	return false;
		// vertical search only out of bound when it's outside of the grid

		// update parent node info
		parentConnection = m_pGraph->GetConnection(currIdx, nextIdx);
		travelDistance += parentConnection->GetCost();

		// check if the next node is the end node
		if (grid[nextIdx] == pEndNode)
		{
			NodeRecord endNode{};

			endNode.pNode = pEndNode;
			endNode.pConnection = parentConnection;
			endNode.parentDirection = dir < 0 ? Direction::Down : Direction::Up;
			endNode.gCost = travelDistance;
			endNode.hCost = 0.0f;

			m_OpenLists.emplace_back(endNode);

			return true;
		}

		// check for left and right obstacles
		const int leftIdx{ nextIdx - 1 };
		const int rightIdx{ nextIdx + 1 };


		// GetConnection() already return us nullptr if there's no connection to it
		T_ConnectionType* leftNode{ IsNodeStillInGrid(leftIdx) ?
			m_pGraph->GetConnection(nextIdx,leftIdx) :
			nullptr };

		T_ConnectionType* rightNode{ IsNodeStillInGrid(rightIdx) ?
			m_pGraph->GetConnection(nextIdx,rightIdx) :
			nullptr };

		bool foundNeighbour{};

		if (leftNode)
		{
			if (!leftNode->IsValid())
			{
				// if it's a blockade then we add the forced neighbours in our open list
				// provide that the node itself is also not a blockade
				const int leftForcedNeighbourIdx{ nextIdx + (m_GraphColumnsCount * dir) - 1 };

				T_ConnectionType* leftForcedNeighbour{ IsNodeStillInGrid(leftForcedNeighbourIdx) ?
					m_pGraph->GetConnection(nextIdx,leftForcedNeighbourIdx) :
					nullptr
				};

				if (leftForcedNeighbour)
				{
					if (leftForcedNeighbour->IsValid())
					{
						//NodeRecord interestingNode{};
						//interestingNode.pNode = leftForcedNeighbour;
						//interestingNode.pConnection = m_pGraph->GetConnection(traverseIndex, leftForcedNeighbourIdx);
						//interestingNode.parentDirection = (dir < 0 ? Direction::BottomLeft : Direction::TopLeft);

						//// TODO: find a way to utilize gCost
						//interestingNode.gCost = 0.0f;

						//interestingNode.hCost = GetHeuristicCost(leftForcedNeighbour, pEndNode);

						//m_OpenLists.emplace_back(interestingNode);
						foundNeighbour = true;
					}
				}
			}
		}

		if (rightNode)
		{
			if (!rightNode->IsValid())
			{
				// same with the top forced neighbour one
				const int rightForcedNeighbourIdx{ nextIdx + (m_GraphColumnsCount * dir) + 1 };

				T_ConnectionType* rightForcedNeighbour{ IsNodeStillInGrid(rightForcedNeighbourIdx) ?
					m_pGraph->GetConnection(nextIdx,rightForcedNeighbourIdx) :
					nullptr
				};

				if (rightForcedNeighbour)
				{
					if (rightForcedNeighbour->IsValid())
					{
						//NodeRecord interestingNode{};
						//interestingNode.pNode = rightForcedNeighbour;
						//interestingNode.pConnection = m_pGraph->GetConnection(traverseIndex, rightForcedNeighbourIdx);
						//interestingNode.parentDirection = (dir < 0 ? Direction::BottomRight : Direction::TopRight);

						//// TODO: find a way to utilize gCost
						//interestingNode.gCost = 0.0f;

						//interestingNode.hCost = GetHeuristicCost(rightForcedNeighbour, pEndNode);

						//m_OpenLists.emplace_back(interestingNode);
						foundNeighbour = true;
					}
				}
			}
		}

		if (foundNeighbour)
		{
			NodeRecord interestingNode{};
			interestingNode.pNode = grid[nextIdx];
			interestingNode.pConnection = parentConnection;
			interestingNode.parentDirection = dir < 0 ? Direction::Down : Direction::Up;
			interestingNode.hCost = GetHeuristicCost(interestingNode.pNode, pEndNode);

			interestingNode.gCost = travelDistance;

			m_OpenLists.emplace_back(interestingNode);

			return true;
		}
	}
}

template <typename T_NodeType, typename T_ConnectionType>
bool JumpPointSearch<T_NodeType, T_ConnectionType>::DiagonalSearch(
	int nodeIdx,
	int horDir,
	int verDir,
	const std::vector<T_NodeType*>& grid,
	T_NodeType* pEndNode,
	T_NodeType* pStartNode,
	float parentGCost)
{
	int traverseIdx{ nodeIdx };
	float travelDistance{ parentGCost };

	while (true)
	{
		const bool foundHorForcedNeighbour{ HorizontalSearch(traverseIdx,horDir,grid,pEndNode,pStartNode,travelDistance) };
		const bool foundVerForcedNeighbour{ VerticalSearch(traverseIdx,verDir,grid,pEndNode,pStartNode,travelDistance) };

		// move the node by the horizontal value and vertical value
		const int currentIdx{ traverseIdx };
		const int nextIdx{ traverseIdx += (m_GraphColumnsCount * verDir) + horDir };

		T_ConnectionType* parentConnection{ m_pGraph->GetConnection(currentIdx,nextIdx) };

		travelDistance += parentConnection->GetCost();

		const int currentCol{ currentIdx % m_GraphColumnsCount };
		const int nextCol{ nextIdx % m_GraphColumnsCount };

		if (!IsNodeStillInGrid(nextIdx))
			return false;

		// check if out of bound
		if (abs(currentCol - nextCol) > 1)
			return false;

		if (grid[nextIdx] == pEndNode)
		{
			NodeRecord interestingNode{};
			interestingNode.pNode = pEndNode;
			interestingNode.pConnection = parentConnection;
			interestingNode.parentDirection = GetDirection(parentConnection);
			interestingNode.gCost = travelDistance;
			interestingNode.hCost = 0.0f;

			m_OpenLists.emplace_back(interestingNode);

			return true;
		}

		if (foundHorForcedNeighbour || foundVerForcedNeighbour)
		{
			NodeRecord interestingNode{};
			interestingNode.pNode = grid[traverseIdx];
			interestingNode.pConnection = parentConnection;
			interestingNode.gCost = travelDistance;
			interestingNode.hCost = GetHeuristicCost(interestingNode.pNode, pEndNode);
			interestingNode.parentDirection = GetDirection(parentConnection);

			m_OpenLists.emplace_back(interestingNode);

			return true;
		}

	}
}

