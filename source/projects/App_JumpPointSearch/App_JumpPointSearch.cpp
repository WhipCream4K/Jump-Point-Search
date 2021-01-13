#include "stdafx.h"
#include "App_JumpPointSearch.h"
#include "framework/EliteAI/EliteGraphs/EliteGraphAlgorithms/EJumpPointSearch.h"

App_JumpPointSearch::~App_JumpPointSearch()
{
	SAFE_DELETE(m_pGridGraph)
}

void App_JumpPointSearch::Start()
{
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(39.0f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(73.0f, 35.0f));

	// Make Grid Graph
	m_pGridGraph = new Elite::GridGraph<Elite::GridTerrainNode, Elite::GraphConnection>(
		m_Columns,
		m_Rows,
		m_CellSize,
		false,
		true, // will test with diagonally enable because jump point search utilize diagonal search
		1.0f,
		1.0f // uniform graph
		);

	startPathIdx = 10;
	endPathIdx = 20;
}

void App_JumpPointSearch::Update(float deltaTime)
{

	UNREFERENCED_PARAMETER(deltaTime);
	
	// User Input
	const bool isMiddleMousePressed{ INPUTMANAGER->IsMouseButtonUp(Elite::InputMouseButton::eMiddle) };
	if(isMiddleMousePressed)
	{
		Elite::MouseData mouseData{ INPUTMANAGER->GetMouseData(Elite::InputType::eMouseButton,Elite::InputMouseButton::eMiddle) };
		Elite::Vector2 mousePos{ DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({float(mouseData.X),float(mouseData.Y)}) };

		const int clickedNode{ m_pGridGraph->GetNodeIdxAtWorldPos(mousePos) };
		if(m_IsStartNodeSelected)
		{
			startPathIdx = clickedNode;
			m_ShouldUpdatePath = true;
		}
		else
		{
			endPathIdx = clickedNode;
			m_ShouldUpdatePath = true;
		}
	}

	// Grid Input
	const bool hasGridChanged{ m_GraphEditor.UpdateGraph(m_pGridGraph) };
	if (hasGridChanged)
		m_ShouldUpdatePath = true;
	
	// IMGUI
	UpdateImGui();

	auto pathfinder{ JumpPointSearch<Elite::GridTerrainNode,Elite::GraphConnection>(m_pGridGraph,m_pHeuristicFunction,m_Columns,m_Rows) };
	m_vPath = pathfinder.FindPath(
		m_pGridGraph->GetNode(startPathIdx),
		m_pGridGraph->GetNode(endPathIdx)
	);
}

void App_JumpPointSearch::Render(float deltaTime) const
{
	UNREFERENCED_PARAMETER(deltaTime);

	m_GraphRenderer.RenderGraph(
		m_pGridGraph,
		m_IsGridDrawn,
		m_IsNodeNumbersDrawn,
		m_IsConnectionsDrawn,
		m_IsConnectionCostsDrawn
	);

	// RenderStartNode
	if (startPathIdx != invalid_node_index)
		m_GraphRenderer.RenderHighlightedGrid(m_pGridGraph, { m_pGridGraph->GetNode(startPathIdx) }, START_NODE_COLOR);
	if (endPathIdx != invalid_node_index)
		m_GraphRenderer.RenderHighlightedGrid(m_pGridGraph, { m_pGridGraph->GetNode(endPathIdx) }, END_NODE_COLOR);

	if (!m_vPath.empty())
		m_GraphRenderer.RenderHighlightedGrid(m_pGridGraph, m_vPath, Elite::Color{ 0.0f,0.0f,1.0f });
}

void App_JumpPointSearch::UpdateImGui()
{
	
}
