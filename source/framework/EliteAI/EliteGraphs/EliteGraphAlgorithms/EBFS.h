#pragma once

namespace Elite 
{
	template <class T_NodeType, class T_ConnectionType>
	class BFS
	{
	public:
		BFS(IGraph<T_NodeType, T_ConnectionType>* pGraph);

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);
	private:
		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
	};

	template <class T_NodeType, class T_ConnectionType>
	BFS<T_NodeType, T_ConnectionType>::BFS(IGraph<T_NodeType, T_ConnectionType>* pGraph)
		: m_pGraph(pGraph)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> BFS<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode)
	{
		std::queue<T_NodeType*> openList{};
		std::map<T_NodeType*, T_NodeType*> closeList{};

		openList.emplace(pStartNode);

		while (!openList.empty())
		{
			T_NodeType* currentNode{ openList.front() };
			openList.pop();

			// that one case if the current node is the end node
			if(currentNode == pDestinationNode)
				break;
			
			for (const auto& connection : m_pGraph->GetNodeConnections(currentNode->GetIndex()))
			{
				T_NodeType* nextNode{ m_pGraph->GetNode(connection->GetTo()) };
				if(closeList.find(nextNode) == closeList.end())
				{
					openList.push(nextNode);
					// tracked back we we came from
					closeList[nextNode] = currentNode;
				}
			}
		}
		
		std::vector<T_NodeType*> path{};
		T_NodeType* currentNode{ pDestinationNode };
		while(currentNode != pStartNode)
		{
			path.emplace_back(currentNode);
			currentNode = closeList[currentNode];
		}

		path.emplace_back(pStartNode);
		std::reverse(path.begin(), path.end());
		
		return path;
	}
}

