#pragma once

#include "framework/EliteAI/EliteGraphs/EIGraph.h"
//namespace Elite
//{
//	template<typename T_NodeType, typename T_ConnectionType>
//	class Elite::IGraph<T_NodeType, T_ConnectionType>;
//}

template<typename T_NodeType,typename T_ConnectionType>
class JumpPointSearch
{
public:

	JumpPointSearch(Elite::IGraph<T_NodeType,T_ConnectionType>* pGraph);

	// Utilities
	vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pEndNode);

private:

	int m_GraphColumnsCount;
	int m_GraphRowsCount;
};

template <typename T_NodeType, typename T_ConnectionType>
JumpPointSearch<T_NodeType, T_ConnectionType>::JumpPointSearch(Elite::IGraph<T_NodeType, T_ConnectionType>* pGraph)
{
}

template <typename T_NodeType, typename T_ConnectionType>
vector<T_NodeType*> JumpPointSearch<T_NodeType, T_ConnectionType>::FindPath(
	T_NodeType* pStartNode,
	T_NodeType* pEndNode)
{
	
}
