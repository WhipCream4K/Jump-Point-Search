#pragma once
#include <stack>

namespace Elite
{
	enum class Eulerianity
	{
		notEulerian,
		semiEulerian,
		eulerian,
	};

	template <class T_NodeType, class T_ConnectionType>
	class EulerianPath
	{
	public:

		EulerianPath(IGraph<T_NodeType, T_ConnectionType>* pGraph);

		Eulerianity IsEulerian() const;
		vector<T_NodeType*> FindPath(Eulerianity& eulerianity) const;

	private:
		void VisitAllNodesDFS(int startIdx, vector<bool>& visited) const;
		bool IsConnected() const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
	};

	template<class T_NodeType, class T_ConnectionType>
	inline EulerianPath<T_NodeType, T_ConnectionType>::EulerianPath(IGraph<T_NodeType, T_ConnectionType>* pGraph)
		: m_pGraph(pGraph)
	{
	}

	template<class T_NodeType, class T_ConnectionType>
	inline Eulerianity EulerianPath<T_NodeType, T_ConnectionType>::IsEulerian() const
	{
		// If the graph is not connected, there can be no Eulerian Trail
		if (!IsConnected())
			return Eulerianity::notEulerian;

		// Count nodes with odd degree 
		int nrOfNodes{ m_pGraph->GetNrOfNodes() };
		int oddCount{};

		for (int i = 0; i < nrOfNodes; ++i)
		{
			if (m_pGraph->IsNodeValid(i) && m_pGraph->GetNodeConnections(i).size() & 1) // calculate odd number using bitwise
				oddCount++;
		}

		// A connected graph with more than 2 nodes with an odd degree (an odd amount of connections) is not Eulerian
		if (oddCount > 2)
			return Eulerianity::notEulerian;

		// A connected graph with exactly 2 nodes with an odd degree is Semi-Eulerian (an Euler trail can be made, but only starting and ending in these 2 nodes)
		if (oddCount == 2 && nrOfNodes != 2)
			return Eulerianity::semiEulerian;

		// A connected graph with no odd nodes is Eulerian
		return Eulerianity::eulerian;
	}

	template<class T_NodeType, class T_ConnectionType>
	inline vector<T_NodeType*> EulerianPath<T_NodeType, T_ConnectionType>::FindPath(Eulerianity& eulerianity) const
	{
		if (!IsConnected())
			return {};

		// Get a copy of the graph because this algorithm involves removing edges
		auto graphCopy = m_pGraph->Clone();
		int nrOfNodes = graphCopy->GetNrOfNodes();
		std::vector<T_NodeType*> path{};

		// algorithm...

		auto generatePath{ [](
			const std::shared_ptr<IGraph<T_NodeType,T_ConnectionType>>& graphClone,
			int startNode)
		{
			std::vector<T_NodeType*> genPath;
			int currentTraverseIndex = startNode;

			genPath.emplace_back(graphClone->GetNode(startNode));
			
			while (!graphClone->GetNodeConnections(currentTraverseIndex).empty() &&
					!graphClone->IsEmpty())
			{
				auto traverseConnections{ graphClone->GetNodeConnections(currentTraverseIndex) };
				for (int i = 0; i < int(traverseConnections.size()); ++i)
				{
					int nextIndex{ traverseConnections.front()->GetTo() };
					if (graphClone->IsNodeValid(nextIndex))
					{
						genPath.emplace_back(graphClone->GetNode(nextIndex));
						currentTraverseIndex = i;
						break;
					}
				}

				graphClone->RemoveNode(currentTraverseIndex);
			}

			return genPath;
		} };

		if (eulerianity == Eulerianity::notEulerian) return path;

		if (eulerianity == Eulerianity::semiEulerian)
		{
			int startIndex{};
			for (int i = 0; i < nrOfNodes; ++i)
			{
				if (m_pGraph->IsNodeValid(i) &&
					m_pGraph->GetNodeConnections(i).size() & 1)
				{
					startIndex = i;
					break;
				}
			}

			path = generatePath(graphCopy, startIndex);
		}

		else if (eulerianity == Eulerianity::eulerian)
		{
			int startIndex{};
			const auto& allActiveNodes{ m_pGraph->GetAllActiveNodes() };
			int firstActiveIndex{ allActiveNodes[0]->GetIndex() };
			if(m_pGraph->IsNodeValid(firstActiveIndex))
				startIndex = firstActiveIndex;
			
			path = generatePath(graphCopy, startIndex);
		}

		return path;
	}

	template<class T_NodeType, class T_ConnectionType>
	inline void EulerianPath<T_NodeType, T_ConnectionType>::VisitAllNodesDFS(int startIdx, vector<bool>& visited) const
	{
		// mark the visited node
		visited[startIdx] = true;

		// recursively visit any valid connected nodes that were not visited before
		for (const auto& connection : m_pGraph->GetNodeConnections(startIdx))
		{
			if (m_pGraph->IsNodeValid(connection->GetTo()) && !visited[connection->GetTo()])
			{
				VisitAllNodesDFS(connection->GetTo(), visited);
			}
		}
	}

	template<class T_NodeType, class T_ConnectionType>
	inline bool EulerianPath<T_NodeType, T_ConnectionType>::IsConnected() const
	{
		const int nrOfNodes{ m_pGraph->GetNrOfNodes() };
		std::vector<bool> visitedNodes(nrOfNodes, false);

		// find a valid starting node that has connections
		int connectedIdx{ invalid_node_index };
		for (int i = 0; i < nrOfNodes; ++i)
		{
			if (m_pGraph->IsNodeValid(i))
			{
				if (!m_pGraph->GetNodeConnections(i).empty())
				{
					connectedIdx = i;
					break;
				}

				return false;
			}
		}

		// if no valid node could be found, return false
		if (connectedIdx == invalid_node_index)
			return false;

		// start a depth-first-search traversal from a node that has connections
		VisitAllNodesDFS(connectedIdx, visitedNodes);

		// if a node was never visited, this graph is not connected
		for (int i = 0; i < nrOfNodes; ++i)
		{
			if (m_pGraph->IsNodeValid(i) && !visitedNodes[i])
				return false;
		}

		return true;
	}

}