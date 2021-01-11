#pragma once
#include "projects/Helpers.h"

namespace Elite
{

	template <class T_NodeType, class T_ConnectionType>
	class AStar
	{
	public:
		AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

		// stores the optimal connection to a node and its total costs related to the start and end node of the path
		struct NodeRecord
		{
			T_NodeType* pNode = nullptr;
			T_ConnectionType* pConnection = nullptr;
			float costSoFar = 0.f; // accumulated g-costs of all the connections leading up to this one
			float estimatedTotalCost = 0.f; // f-cost (= costSoFar + h-cost)

			bool operator==(const NodeRecord& other) const
			{
				return pNode == other.pNode
					&& pConnection == other.pConnection
					&& costSoFar == other.costSoFar
					&& estimatedTotalCost == other.estimatedTotalCost;
			}

			bool operator<(const NodeRecord& other) const
			{
				return estimatedTotalCost < other.estimatedTotalCost;
			}
		};

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);
		std::vector<NodeForRender> GetVisitedNodeIdx();

	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		std::vector<NodeRecord> m_CloseList;
		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template <class T_NodeType, class T_ConnectionType>
	AStar<T_NodeType, T_ConnectionType>::AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph(pGraph)
		, m_HeuristicFunction(hFunction)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> AStar<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pGoalNode)
	{
		m_CloseList.clear();

		if (pStartNode != pGoalNode)
		{
			std::vector<NodeRecord> openList{};
			std::vector<NodeRecord> closeList{};

			NodeRecord start{};
			start.pNode = pStartNode;
			start.estimatedTotalCost = GetHeuristicCost(pStartNode, pGoalNode);

			openList.emplace_back(start);
			typename std::vector<NodeRecord>::iterator shortestNodeRecIt{};
			NodeRecord currentNodeRec{};

#pragma region First Style
			/*
			while(!openList.empty())
			{
				// find the least gCost(the shortest path) from openlist node
				shortestNodeRecIt = std::min_element(openList.begin(),openList.end());
				currentNodeRec = *shortestNodeRecIt;

				closeList.emplace_back(currentNodeRec);

				// early exit
				if(currentNodeRec.pNode == pGoalNode)
				{
					std::vector<T_NodeType*> path{};
					while(currentNodeRec.pNode != pStartNode)
					{
						path.emplace_back(currentNodeRec.pNode);
						T_NodeType* connectedNode{ m_pGraph->GetNode(currentNodeRec.pConnection->GetFrom()) };
						currentNodeRec = *std::find_if(closeList.begin(), closeList.end(), [&connectedNode](const NodeRecord& rec)
							{
								return rec.pNode == connectedNode;
							});
					}

					path.emplace_back(pStartNode);

					return path;
				}

				openList.erase(shortestNodeRecIt);

				const auto& neighbours{ m_pGraph->GetNodeConnections(currentNodeRec.pNode->GetIndex()) };
				for (const auto& connection : neighbours)
				{
					if(connection->IsValid())
					{
						// if this connection is already in the closelist then continue
						auto cIt = std::find_if(closeList.begin(), closeList.end(), [&connection](const NodeRecord& rec)
							{
								return rec.pConnection == connection;
							});

						const bool isExistInCloseList{ cIt != closeList.end() };

						if(isExistInCloseList)
							continue;

						T_NodeType* pointedNode{ m_pGraph->GetNode(connection->GetTo()) };

						// Check if the connection lead to the node existed already in closelist
						float costToThisNodeSoFar{currentNodeRec.costSoFar +
							GetHeuristicCost(currentNodeRec.pNode,pointedNode)};

						cIt = std::find_if(openList.begin(), openList.end(), [&connection](const NodeRecord& rec)
							{
								return rec.pConnection == connection;
							});

						const bool isExistInOpenList{ cIt != openList.end() };

						if(costToThisNodeSoFar < currentNodeRec.costSoFar ||
							!isExistInOpenList)
						{
							const float estimatedTotalCost{ GetHeuristicCost(pointedNode,pGoalNode) };

							NodeRecord newNodeRec{
								pointedNode,
								connection,
								costToThisNodeSoFar,
								estimatedTotalCost };

							openList.emplace_back(newNodeRec);
						}
					}
				}
			}
		*/
#pragma endregion

			auto IsNodeExistInList{ [](const std::vector<NodeRecord>& list,T_NodeType* node)
			{
				auto fIt = std::find_if(list.begin(),list.end(),[&node](const NodeRecord& rec)
				{
					return rec.pNode == node;
				});

				return std::make_pair(fIt != list.end(), fIt);
			} };

			while (!openList.empty())
			{
				const auto cIt = std::min_element(openList.begin(), openList.end());
				currentNodeRec = *cIt;

				openList.erase(cIt);
				closeList.emplace_back(currentNodeRec);

				if (currentNodeRec.pNode != pGoalNode)
				{
					const auto& neighbours{ m_pGraph->GetNodeConnections(currentNodeRec.pNode->GetIndex()) };
					for (const auto& connection : neighbours)
					{
						if (connection->IsValid())
						{
							T_NodeType* pointedNode{ m_pGraph->GetNode(connection->GetTo()) };

							const float costSoFar{ currentNodeRec.costSoFar /*+ GetHeuristicCost(currentNodeRec.pNode,pointedNode)*/ + connection->GetCost() };

							// TODO: This one might cause problem because it doesn't take connection cost into account
							const float connectionFCost{ costSoFar + GetHeuristicCost(pointedNode,pGoalNode) };
	
							auto fIt = std::find_if(closeList.begin(), closeList.end(),
								[&pointedNode](const NodeRecord& rec)
								{
									return rec.pNode == pointedNode;
								});

							if (fIt != closeList.end())
							{
								if (costSoFar < fIt->costSoFar)
									closeList.erase(fIt);
								continue;
							}

							fIt = std::find_if(openList.begin(), openList.end(),
								[&pointedNode](const NodeRecord& rec)
								{
									return rec.pNode == pointedNode;
								});

							if (fIt != openList.end())
							{
								if (costSoFar < fIt->costSoFar)
									openList.erase(fIt);
								continue;
							}
							
							NodeRecord traverseNode{
										pointedNode,
										connection,
										costSoFar,
										connectionFCost
							};

							openList.emplace_back(traverseNode);
						}
					}
				}
				else
				{
					std::vector<T_NodeType*> path{};
					while (currentNodeRec.pNode != pStartNode)
					{
						path.emplace_back(currentNodeRec.pNode);
						T_NodeType* connectedNode{ m_pGraph->GetNode(currentNodeRec.pConnection->GetFrom()) };
						currentNodeRec = *std::find_if(closeList.begin(), closeList.end(),
							[&connectedNode](const NodeRecord& rec)
							{
								return rec.pNode == connectedNode;
							});
					}

					path.emplace_back(pStartNode);
					std::reverse(path.begin(), path.end());

					m_CloseList = std::move(closeList);

					return path;
				}
			}

		}

		return vector<T_NodeType*>{pStartNode};
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<NodeForRender> AStar<T_NodeType, T_ConnectionType>::GetVisitedNodeIdx()
	{
		std::vector<NodeForRender> out{};

		for (const auto& node : m_CloseList)
		{
			out.emplace_back(NodeForRender{
				node.pNode->GetIndex(),
				node.costSoFar,
				node.estimatedTotalCost
				});
		}

		return out;
	}

	template <class T_NodeType, class T_ConnectionType>
	float Elite::AStar<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
}
