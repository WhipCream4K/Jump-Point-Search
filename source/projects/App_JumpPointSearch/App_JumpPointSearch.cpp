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

	if(m_ShouldUpdatePath && 
		startPathIdx != invalid_node_index &&
		endPathIdx != invalid_node_index &&
		startPathIdx != endPathIdx)
	{
		auto pathfinder{ JumpPointSearch<Elite::GridTerrainNode,Elite::GraphConnection>(m_pGridGraph,m_pHeuristicFunction,m_Columns,m_Rows) };
		m_vPath = pathfinder.FindPath(
			m_pGridGraph->GetNode(startPathIdx),
			m_pGridGraph->GetNode(endPathIdx)
		);

		m_JumpPoints = pathfinder.GetJumpPoints();
		
		m_ShouldUpdatePath = false;
		std::cout << "New Path Calculated\n";
	}
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

	if(m_ShowJumpPoints)
	{
		if(!m_vPath.empty())
		{
			const float halfCellSize{ m_CellSize / 2.0f };
			const Elite::Color JPColor{ 1.0f,1.0f,0.0f };
			
			for (const auto& jumpPoint : m_JumpPoints)
			{
				const auto nodePos{ m_pGridGraph->GetNodeWorldPos(jumpPoint.nodeIdx) };

				Elite::Vector2 verts[4]
				{
					Elite::Vector2(nodePos.x - halfCellSize, nodePos.y - halfCellSize),
					Elite::Vector2(nodePos.x - halfCellSize, nodePos.y + halfCellSize),
					Elite::Vector2(nodePos.x + halfCellSize, nodePos.y + halfCellSize),
					Elite::Vector2(nodePos.x + halfCellSize, nodePos.y - halfCellSize)
				};

				DEBUGRENDERER2D->DrawSolidPolygon(verts, 4, JPColor, -1.0f);
				DEBUGRENDERER2D->DrawDirection(nodePos, { 1.0f,1.0f }, 10.0f, { 1.0f,0.0f,0.0f }, -1.0f);
			}
		}
	}
}

void App_JumpPointSearch::UpdateImGui()
{
	
//#ifdef PLATFORM_WINDOWS
#pragma region UI
	//UI
	{
		//Setup
		int menuWidth = 115;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
		ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::PushAllowKeyboardFocus(false);

		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Text("LMB: target");
		ImGui::Text("RMB: start");
		ImGui::Unindent();

		/*Spacing*/ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		/*Spacing*/ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();

		ImGui::Text("Jump Point Search");
		ImGui::Spacing();

		ImGui::Text("Middle Mouse");
		ImGui::Text("controls");
		std::string buttonText{ "" };
		if (m_IsStartNodeSelected)
			buttonText += "Start Node";
		else
			buttonText += "End Node";

		if (ImGui::Button(buttonText.c_str()))
		{
			m_IsStartNodeSelected = !m_IsStartNodeSelected;
		}

		ImGui::Checkbox("Grid", &m_IsGridDrawn);
		ImGui::Checkbox("NodeNumbers", &m_IsNodeNumbersDrawn);
		ImGui::Checkbox("Connections", &m_IsConnectionsDrawn);
		ImGui::Checkbox("Connections Costs", &m_IsConnectionCostsDrawn);
		if (ImGui::Combo("", &m_SelectedHeuristic, "Manhattan\0Euclidean\0SqrtEuclidean\0Octile\0Chebyshev", 4))
		{
			switch (m_SelectedHeuristic)
			{
			case 0:
				m_pHeuristicFunction = Elite::HeuristicFunctions::Manhattan;
				break;
			case 1:
				m_pHeuristicFunction = Elite::HeuristicFunctions::Euclidean;
				break;
			case 2:
				m_pHeuristicFunction = Elite::HeuristicFunctions::SqrtEuclidean;
				break;
			case 3:
				m_pHeuristicFunction = Elite::HeuristicFunctions::Octile;
				break;
			case 4:
				m_pHeuristicFunction = Elite::HeuristicFunctions::Chebyshev;
				break;
			default:
				m_pHeuristicFunction = Elite::HeuristicFunctions::Chebyshev;
				break;
			}
		}
		ImGui::Spacing();

		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Checkbox("Show Debug Options", &m_ShowDebugOptions);

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}

	if(m_ShowDebugOptions)
	{
		ImGui::Begin("JPS Debug Options");
		ImGui::Checkbox("Show Jump Points", &m_ShowJumpPoints);
		ImGui::End();
	}
}
