#include "stdafx.h"
#include "App_JumpPointSearch.h"

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
		false,
		1.0f,
		1.5f
		);
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

		int clickedNode{ m_pGridGraph->GetNodeIdxAtWorldPos(mousePos) };
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
}

void App_JumpPointSearch::UpdateImGui()
{
	
}
