#pragma once

//#include "framework/EliteAI/EliteGraphs/EIGraph.h"
//namespace Elite
//{
//	template<typename T_NodeType, typename T_ConnectionType>
//	class Elite::IGraph<T_NodeType, T_ConnectionType>;
//}

#include "projects/Helpers.h"

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
		Left			= 0b1,
		Right			= 0b10,
		TopRight		= 0b100,
		TopLeft			= 0b1000,
		BottomRight		= 0b10000,
		BottomLeft		= 0b100000,
		Up				= 0b1000000,
		Down			= 0b10000000,
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
	std::vector<NodeForRender> GetJumpPoints();

	// For Debugging
	std::vector<SearchDirection> GetAllSearchDirection();

private:

	// weak reference to graph object
	Elite::IGraph<T_NodeType, T_ConnectionType>* m_pGraph;

	std::vector<NodeRecord> m_OpenLists;
	std::vector<NodeRecord> m_VisitedJumpPoints;

	// Debugging
	std::vector<SearchDirection> m_SearchDirections;

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

	std::vector<NodeRecord> GetRequiredPathFromNode(const NodeRecord& nodeRec, UINT requiredPath);
	void AddJumpPoint(const NodeRecord& rec);
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

	m_OpenLists.clear();
	m_VisitedJumpPoints.clear();

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

	NodeRecord startNode{};
	startNode.pNode = pStartNode;

	//m_VisitedJumpPoints.emplace_back(startNode);

	// HOT ZONE!!
	while (!m_OpenLists.empty())
	{
		// 0. determine the best node and direction to start with ( probably needed a heuristic test )
		const auto cIt{ std::min_element(m_OpenLists.begin(),m_OpenLists.end()) };
		NodeRecord jumpPoint{ *cIt };

		// 1. search HORIZONTALLY for an 'interesting' node
		// beware for the up and down blocked node
		// if found one, depends on the direction we are going to
		// add the diagonal node to the open list

		if (jumpPoint.pNode != pEndNode)
		{
			// remove the visited jump point out of the open list
			m_OpenLists.erase(cIt);
			m_VisitedJumpPoints.emplace_back(jumpPoint);

			int pointedIdx{ jumpPoint.pNode->GetIndex() };
			switch (jumpPoint.parentDirection)
			{
			case Direction::Left:
				HorizontalSearch(pointedIdx, -1, gridArray, pEndNode, pStartNode, jumpPoint.gCost);			break;
			case Direction::Right:
				HorizontalSearch(pointedIdx, 1, gridArray, pEndNode, pStartNode, jumpPoint.gCost);			break;
			case Direction::Up:
				VerticalSearch(pointedIdx, 1, gridArray, pEndNode, pStartNode, jumpPoint.gCost);			break;
			case Direction::Down:
				VerticalSearch(pointedIdx, -1, gridArray, pEndNode, pStartNode, jumpPoint.gCost);			break;
			case Direction::TopLeft:
				DiagonalSearch(pointedIdx, -1, 1, gridArray, pEndNode, pStartNode, jumpPoint.gCost);		break;
			case Direction::TopRight:
				DiagonalSearch(pointedIdx, 1, 1, gridArray, pEndNode, pStartNode, jumpPoint.gCost);			break;
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
			// since there's no link between jump points
			// we gonna use the direction of the parent connection to track back to the other jump point instead
			std::vector<T_NodeType*> path{};
			T_NodeType* pPointedNode{ jumpPoint.pNode };
			int traverseIdx{ jumpPoint.pNode->GetIndex() };
			Direction toParentDir{ GetDirection(jumpPoint.pConnection) };

			while (pPointedNode != pStartNode)
			{
				const auto parentJumpPoint = std::find_if(m_OpenLists.begin(), m_OpenLists.end(), [pPointedNode](const NodeRecord& nodeRec)
					{
						return pPointedNode == nodeRec.pNode;
					});

				const auto oldJumpPoint = std::find_if(m_VisitedJumpPoints.begin(), m_VisitedJumpPoints.end(), [pPointedNode](const NodeRecord& nodeRec)
					{
						return pPointedNode == nodeRec.pNode;
					});
			

				// check if this node is a jump point
				const bool shouldChangeDirection{ parentJumpPoint != m_OpenLists.end() || oldJumpPoint != m_VisitedJumpPoints.end() };

				if (shouldChangeDirection)
				{
					if (parentJumpPoint != m_OpenLists.end())
						toParentDir = GetDirection(parentJumpPoint->pConnection);
					else
						toParentDir = GetDirection(oldJumpPoint->pConnection);
				}

				// reverse
				switch (toParentDir)
				{
				case Direction::Left:
					traverseIdx += 1;
					break;
				case Direction::Right:
					traverseIdx -= 1;
					break;
				case Direction::Up:
					traverseIdx -= m_GraphColumnsCount;
					break;
				case Direction::Down:
					traverseIdx += m_GraphColumnsCount;
					break;
				case Direction::TopLeft:
					traverseIdx -= m_GraphColumnsCount - 1;
					break;
				case Direction::TopRight:
					traverseIdx -= m_GraphColumnsCount + 1;
					break;
				case Direction::BottomLeft:
					traverseIdx += m_GraphColumnsCount + 1;
					break;
				case Direction::BottomRight:
					traverseIdx += m_GraphColumnsCount - 1;
					break;
				default: break;
				}

				if(IsNodeStillInGrid(traverseIdx))
					pPointedNode = gridArray[traverseIdx];

				path.emplace_back(pPointedNode);
			}

			std::reverse(path.begin(), path.end());
			return path;
		}
	}



	return std::vector<T_NodeType*>{pStartNode};
}

template <typename T_NodeType, typename T_ConnectionType>
std::vector<NodeForRender> JumpPointSearch<T_NodeType, T_ConnectionType>::GetJumpPoints()
{
	std::vector<NodeForRender> out{};

	for (const auto& node : m_OpenLists)
	{
		out.emplace_back(
			NodeForRender{
				node.pNode->GetIndex(),
				node.gCost,
				node.hCost
			}
		);
	}

	return out;
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
	bool hasFoundFirstForcedNeighbour{};

	while (true)
	{
		// search along the grid
		const int currIdx{ traverseIndex };
		const int nextIdx{ traverseIndex += dir };
		const int beforeIdx{ currIdx - dir };

		if (!IsNodeStillInGrid(nextIdx))
			return false;

		// check ahead if next connection is out of bound or blocked
		parentConnection = m_pGraph->GetConnection(currIdx, nextIdx);

		if (!parentConnection)
			return false;

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
		int topIdx{ currIdx + m_GraphColumnsCount };
		int bottomIdx{ currIdx - m_GraphColumnsCount };

		// GetConnection() already return us nullptr if there's no connection to it
		T_ConnectionType* topNode{ IsNodeStillInGrid(topIdx) ?
			m_pGraph->GetConnection(currIdx,topIdx) :
			nullptr };

		T_ConnectionType* bottomNode{ IsNodeStillInGrid(bottomIdx) ?
			m_pGraph->GetConnection(currIdx,bottomIdx) :
			nullptr };

		bool foundNeighbour{};

		if (!topNode)
		{
			// Check if this connection is blocked
			//if (!topNode->IsValid())
			//{
				if(!hasFoundFirstForcedNeighbour)
				{
					// if it's a blockade then we add the forced neighbour in our open list
					// provide that the node itself is also not a blockade
					int topForcedNeighbourIdx{ currIdx + m_GraphColumnsCount + dir };

					T_ConnectionType* topForcedNeighbour{ IsNodeStillInGrid(topForcedNeighbourIdx) ?
						m_pGraph->GetConnection(currIdx,topForcedNeighbourIdx) :
						nullptr
					};

					if (topForcedNeighbour)
					{
						if (topForcedNeighbour->IsValid())
						{
							NodeRecord interestingNode{};
							interestingNode.pNode = grid[topForcedNeighbourIdx];
							interestingNode.pConnection = topForcedNeighbour;
							interestingNode.parentDirection = (dir < 0 ? Direction::TopLeft : Direction::TopRight);
							interestingNode.gCost = travelDistance + topForcedNeighbour->GetCost();
							interestingNode.hCost = GetHeuristicCost(interestingNode.pNode, pEndNode);

							m_OpenLists.emplace_back(interestingNode);

							hasFoundFirstForcedNeighbour = true;
							foundNeighbour = true;
						}
					}
				}
				else
				{
					// add ourself then return
					NodeRecord interestingNode{};
					interestingNode.pNode = grid[currIdx];
					interestingNode.pConnection = m_pGraph->GetConnection(beforeIdx, currIdx);
					interestingNode.parentDirection = dir < 0 ? Direction::Left : Direction::Right;
					interestingNode.gCost = travelDistance + interestingNode.pConnection->GetCost();
					interestingNode.hCost = GetHeuristicCost(interestingNode.pNode, pEndNode);

					m_OpenLists.emplace_back(interestingNode);
					return true;
				}
			//}
		}

		if (!bottomNode)
		{
			//// Check if this connection is blocked
			//if (!bottomNode->IsValid())
			//{
				if(!hasFoundFirstForcedNeighbour)
				{
					// same with the top forced neighbour one
					int bottomForcedNeighbourIdx{ currIdx - m_GraphColumnsCount + dir };

					T_ConnectionType* bottomForcedNeighbour{ IsNodeStillInGrid(bottomForcedNeighbourIdx) ?
						m_pGraph->GetConnection(currIdx,bottomForcedNeighbourIdx) :
						nullptr
					};

					if (bottomForcedNeighbour)
					{
						if (bottomForcedNeighbour->IsValid())
						{
							NodeRecord interestingNode{};
							interestingNode.pNode = grid[bottomForcedNeighbourIdx];
							interestingNode.pConnection = bottomForcedNeighbour;
							interestingNode.parentDirection = (dir < 0 ? Direction::BottomLeft : Direction::BottomRight);
							interestingNode.gCost = travelDistance + bottomForcedNeighbour->GetCost();
							interestingNode.hCost = GetHeuristicCost(interestingNode.pNode, pEndNode);

							m_OpenLists.emplace_back(interestingNode);
							hasFoundFirstForcedNeighbour = true;
							foundNeighbour = true;
						}
					}
				}
				else
				{
					// add ourself then return
					NodeRecord interestingNode{};
					interestingNode.pNode = grid[currIdx];
					interestingNode.pConnection = m_pGraph->GetConnection(beforeIdx, currIdx);
					interestingNode.parentDirection = dir < 0 ? Direction::Left : Direction::Right;
					interestingNode.gCost = travelDistance + interestingNode.pConnection->GetCost();
					interestingNode.hCost = GetHeuristicCost(interestingNode.pNode, pEndNode);

					m_OpenLists.emplace_back(interestingNode);
					return true;
				}
			//}
		}



//		if (foundNeighbour)
//		{
//			if (hasFoundFirstForcedNeighbour)
//			{
//
//			}
//#pragma region Temp
//			//NodeRecord interestingNode{};
//			//interestingNode.pNode = grid[currIdx];
//			//interestingNode.parentDirection = dir < 0 ? Direction::Left : Direction::Right;
//			//interestingNode.pConnection = m_pGraph->GetConnection(beforeIdx,currIdx);
//			//interestingNode.hCost = GetHeuristicCost(grid[currIdx], pEndNode);
//
//			//interestingNode.gCost = travelDistance;
//
//			//AddJumpPoint(interestingNode);
//			//
//			//// add all the path available from this node waited to be searched
//			//// pretty much also add the 'forced neighbours' that we found along the way to be jump points
//
//			//UINT requiredPath{};
//			//
//			//if (dir < 0) // left
//			//{
//			//	// check for top left, bottom left if the path is available
//			//	// if so then add it to the open list
//			//	requiredPath = UINT(Direction::TopLeft) | UINT(Direction::BottomLeft);
//			//}
//			//else // right
//			//{
//			//	// check for top right, bottom right
//			//	requiredPath = UINT(Direction::TopRight) | UINT(Direction::BottomRight);
//			//}
//
//			//auto connectedPath{ GetRequiredPathFromNode(interestingNode,requiredPath) };
//			//for (const auto& path : connectedPath)
//			//{
//			//	AddJumpPoint(path);
//			//}
//			//
//			//return true;
//#pragma endregion 
//		}

		travelDistance += parentConnection->GetCost();
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
	bool hasFoundFirstForcedNeighbour{};

	while (true)
	{
		// search along the grid
		const int currIdx{ traverseIndex };
		const int nextIdx{ traverseIndex += m_GraphColumnsCount * dir };
		const int beforeIdx{ currIdx - m_GraphColumnsCount * dir };

		if (!IsNodeStillInGrid(nextIdx))
			return false;

		// return if it's out of bound
		//if (traverseIndex % m_GraphRowsCount == 0)
		//	return false;
		// vertical search only out of bound when it's outside of the grid

		// update parent node info
		parentConnection = m_pGraph->GetConnection(currIdx, nextIdx);

		if (!parentConnection)
			return false;

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
		const int leftIdx{ currIdx - 1 };
		const int rightIdx{ currIdx + 1 };


		// GetConnection() already return us nullptr if there's no connection to it
		T_ConnectionType* leftNode{ IsNodeStillInGrid(leftIdx) ?
			m_pGraph->GetConnection(currIdx,leftIdx) :
			nullptr };

		T_ConnectionType* rightNode{ IsNodeStillInGrid(rightIdx) ?
			m_pGraph->GetConnection(currIdx,rightIdx) :
			nullptr };

		bool foundNeighbour{};

		if (!leftNode)
		{
			//if (!leftNode->IsValid())
			//{
				if(!hasFoundFirstForcedNeighbour)
				{
					// if it's a blockade then we add the forced neighbours in our open list
					// provide that the node itself is also not a blockade
					const int leftForcedNeighbourIdx{ currIdx + (m_GraphColumnsCount * dir) - 1 };

					T_ConnectionType* leftForcedNeighbour{ IsNodeStillInGrid(leftForcedNeighbourIdx) ?
						m_pGraph->GetConnection(currIdx,leftForcedNeighbourIdx) :
						nullptr
					};

					if (leftForcedNeighbour)
					{
						if (leftForcedNeighbour->IsValid())
						{
							NodeRecord interestingNode{};
							interestingNode.pNode = grid[leftForcedNeighbourIdx];
							interestingNode.pConnection = leftForcedNeighbour;
							interestingNode.parentDirection = (dir < 0 ? Direction::BottomLeft : Direction::TopLeft);
							interestingNode.gCost = travelDistance + leftForcedNeighbour->GetCost();
							interestingNode.hCost = GetHeuristicCost(interestingNode.pNode, pEndNode);

							m_OpenLists.emplace_back(interestingNode);
							foundNeighbour = true;
							hasFoundFirstForcedNeighbour = true;
						}
					}
				}
				else
				{
					NodeRecord interestingNode{};
					interestingNode.pNode = grid[currIdx];
					interestingNode.pConnection = m_pGraph->GetConnection(beforeIdx,currIdx);
					interestingNode.parentDirection = dir < 0 ? Direction::Down : Direction::Up;
					interestingNode.gCost = travelDistance + interestingNode.pConnection->GetCost();
					interestingNode.hCost = GetHeuristicCost(interestingNode.pNode, pEndNode);

					m_OpenLists.emplace_back(interestingNode);
					return true;
				}
			//}
		}

		if (!rightNode)
		{
			//if (!rightNode->IsValid())
			//{
				if(!hasFoundFirstForcedNeighbour)
				{
					// same with the top forced neighbour one
					const int rightForcedNeighbourIdx{ currIdx + (m_GraphColumnsCount * dir) + 1 };

					T_ConnectionType* rightForcedNeighbour{ IsNodeStillInGrid(rightForcedNeighbourIdx) ?
						m_pGraph->GetConnection(currIdx,rightForcedNeighbourIdx) :
						nullptr
					};

					if (rightForcedNeighbour)
					{
						if (rightForcedNeighbour->IsValid())
						{
							NodeRecord interestingNode{};
							interestingNode.pNode = grid[rightForcedNeighbourIdx];
							interestingNode.pConnection = rightForcedNeighbour;
							interestingNode.parentDirection = dir < 0 ? Direction::BottomRight : Direction::TopRight;
							interestingNode.gCost = travelDistance + rightForcedNeighbour->GetCost();
							interestingNode.hCost = GetHeuristicCost(interestingNode.pNode, pEndNode);

							m_OpenLists.emplace_back(interestingNode);
							foundNeighbour = true;
							hasFoundFirstForcedNeighbour = true;
						}
					}
				}
				else
				{
					NodeRecord interestingNode{};
					interestingNode.pNode = grid[currIdx];
					interestingNode.pConnection = m_pGraph->GetConnection(beforeIdx, currIdx);
					interestingNode.parentDirection = dir < 0 ? Direction::Down : Direction::Up;
					interestingNode.gCost = travelDistance + interestingNode.pConnection->GetCost();
					interestingNode.hCost = GetHeuristicCost(interestingNode.pNode, pEndNode);

					m_OpenLists.emplace_back(interestingNode);
					return true;
				}
			//}
		}

		//if (foundNeighbour)
		//{
		//	NodeRecord interestingNode{};
		//	interestingNode.pNode = grid[nextIdx];
		//	interestingNode.pConnection = parentConnection;
		//	interestingNode.parentDirection = dir < 0 ? Direction::Down : Direction::Up;
		//	interestingNode.hCost = GetHeuristicCost(interestingNode.pNode, pEndNode);

		//	interestingNode.gCost = travelDistance;

		//	m_OpenLists.emplace_back(interestingNode);

		//	// same with horizontal search
		//	// add all passable path to the open list to be searched
		//	UINT requiredPath{};
		//	if(dir < 0) // Down
		//	{
		//		// check for bottom left, bottom right
		//		requiredPath = UINT(Direction::BottomLeft) | UINT(Direction::BottomRight);
		//	}
		//	else // UP
		//	{
		//		// check for top left, top right
		//		requiredPath = UINT(Direction::TopLeft) | UINT(Direction::TopRight);
		//	}

		//	auto connectedPath{ GetRequiredPathFromNode(interestingNode,requiredPath) };
		//	for (const auto& path : connectedPath)
		//		m_OpenLists.emplace_back(path);
		//	

		//	return true;
		//}

		travelDistance += parentConnection->GetCost();
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
		const int beforeIdx{ currentIdx - (m_GraphColumnsCount * verDir) + horDir };

		if (!IsNodeStillInGrid(nextIdx))
			return false;

		T_ConnectionType* parentConnection{ m_pGraph->GetConnection(currentIdx,nextIdx) };

		if (!parentConnection)
			return false;

		// check if out of bound
		//if (abs(currentCol - nextCol) > 1)
		//	return false;

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

		// check for the diagonal forced neighbour

		if (foundHorForcedNeighbour || foundVerForcedNeighbour)
		{
			NodeRecord interestingNode{};
			interestingNode.pNode = grid[currentIdx];
			interestingNode.pConnection = parentConnection;
			interestingNode.gCost = travelDistance;
			interestingNode.hCost = GetHeuristicCost(interestingNode.pNode, pEndNode);
			interestingNode.parentDirection = GetDirection(parentConnection);

			m_OpenLists.emplace_back(interestingNode);

			return true;
		}

		travelDistance += parentConnection->GetCost();
	}
}

template <typename T_NodeType, typename T_ConnectionType>
std::vector<typename JumpPointSearch<T_NodeType, T_ConnectionType>::NodeRecord> JumpPointSearch<T_NodeType,
T_ConnectionType>::GetRequiredPathFromNode(const NodeRecord& nodeRec, UINT requiredPath)
{
	std::vector<NodeRecord> out{};
	
	for (const auto& connection : m_pGraph->GetNodeConnections(nodeRec.pNode->GetIndex()))
	{
		if(connection->IsValid())
		{
			const Direction toDirection{ GetDirection(connection) };
			if(requiredPath & toDirection)
			{
				NodeRecord path{};
				path.pNode = nodeRec.pNode;
				path.pConnection = nodeRec.pConnection;
				path.parentDirection = toDirection;
				path.gCost = nodeRec.gCost;
				path.hCost = nodeRec.hCost;

				out.emplace_back(path);
			}
		}
	}

	return out;
}

template <typename T_NodeType, typename T_ConnectionType>
void JumpPointSearch<T_NodeType, T_ConnectionType>::AddJumpPoint(const NodeRecord& rec)
{
	const auto fIt{ std::find(m_OpenLists.begin(),m_OpenLists.end(),rec) };
	if(fIt == m_OpenLists.end())
		m_OpenLists.emplace_back(rec);
	
}

