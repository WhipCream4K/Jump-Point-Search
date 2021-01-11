//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_PathfindingAStar.h"
#include "framework/EliteAI/EliteGraphs/EliteGraphAlgorithms/EAStar.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EBFS.h"

using namespace Elite;

//Destructor
App_PathfindingAStar::~App_PathfindingAStar()
{
	SAFE_DELETE(m_pGridGraph)
		//SAFE_DELETE(m_PathFinder)
}

//Functions
void App_PathfindingAStar::Start()
{
	//Set Camera
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(39.0f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(73.0f, 35.0f));
	//DEBUGRENDERER2D->GetActiveCamera()->SetMoveLocked(true);
	//DEBUGRENDERER2D->GetActiveCamera()->SetZoomLocked(true);

	//Create Graph
	MakeGridGraph();

	startPathIdx = 0;
	endPathIdx = 4;

	m_GCost.left = Elite::Color{ 0.0f,1.0f,1.0f }; // teal
	m_GCost.right = Elite::Color{ 1.0f,1.0f,0.0f }; // yellow

	m_FCost.left = Elite::Color{ 0.0f,1.0f,0.0f }; // green
	m_FCost.right = Elite::Color{ 1.0f,0.0f,0.0f }; // red

}

void App_PathfindingAStar::Update(float deltaTime)
{
	UNREFERENCED_PARAMETER(deltaTime);

	//INPUT
	bool const middleMousePressed = INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eMiddle);
	if (middleMousePressed)
	{
		MouseData mouseData = { INPUTMANAGER->GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eMiddle) };
		Elite::Vector2 mousePos = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ (float)mouseData.X, (float)mouseData.Y });

		//Find closest node to click pos
		int closestNode = m_pGridGraph->GetNodeIdxAtWorldPos(mousePos);
		if (m_StartSelected)
		{
			startPathIdx = closestNode;
			m_UpdatePath = true;
		}
		else
		{
			endPathIdx = closestNode;
			m_UpdatePath = true;
		}
	}

	//GRID INPUT
	bool hasGridChanged = m_GraphEditor.UpdateGraph(m_pGridGraph);
	if (hasGridChanged)
	{
		m_UpdatePath = true;
	}

	//IMGUI
	UpdateImGui();


	//CALCULATEPATH
	//If we have nodes and the target is not the startNode, find a path!
	if (m_UpdatePath
		&& startPathIdx != invalid_node_index
		&& endPathIdx != invalid_node_index
		&& startPathIdx != endPathIdx)
	{

		//BFS Pathfinding
		//auto pathfinder = BFS<GridTerrainNode, GraphConnection>(m_pGridGraph);
		//auto startNode = m_pGridGraph->GetNode(startPathIdx);
		//auto endNode = m_pGridGraph->GetNode(endPathIdx);

		// AStar PathFinding
		auto pathfinder = AStar<GridTerrainNode, GraphConnection>(m_pGridGraph, m_pHeuristicFunction);
		auto startNode = m_pGridGraph->GetNode(startPathIdx);
		auto endNode = m_pGridGraph->GetNode(endPathIdx);


		m_vPath = pathfinder.FindPath(startNode, endNode);
		m_VisitedNode = pathfinder.GetVisitedNodeIdx();

		m_UpdatePath = false;
		std::cout << "New Path Calculated" << std::endl;
	}
}

void App_PathfindingAStar::Render(float deltaTime) const
{
	UNREFERENCED_PARAMETER(deltaTime);
	//Render grid
	m_GraphRenderer.RenderGraph(
		m_pGridGraph,
		m_bDrawGrid,
		m_bDrawNodeNumbers,
		m_bDrawConnections,
		m_bDrawConnectionsCosts
	);

	//Render start node on top if applicable
	if (startPathIdx != invalid_node_index)
	{
		m_GraphRenderer.RenderHighlightedGrid(m_pGridGraph, { m_pGridGraph->GetNode(startPathIdx) }, START_NODE_COLOR);
	}

	//Render end node on top if applicable
	if (endPathIdx != invalid_node_index)
	{
		m_GraphRenderer.RenderHighlightedGrid(m_pGridGraph, { m_pGridGraph->GetNode(endPathIdx) }, END_NODE_COLOR);
	}

	//render path below if applicable
	if (!m_vPath.empty())
	{
		m_GraphRenderer.RenderHighlightedGrid(m_pGridGraph, m_vPath);
	}

	if (m_ShowVisitedNode)
	{
		// Render Visited Node
		if (!m_VisitedNode.empty())
		{
			auto CmpNodeForRender{ [this](const NodeForRender& left,const NodeForRender& right)
			{
				if (m_SelectedShownGradient == 0)
					return left.costSoFar < right.costSoFar;

				return left.fCost < right.fCost;
			} };

			const float halfCellSize{ 15.0f / 2.0f };
			const auto maxValueNeeded{ *std::max_element(m_VisitedNode.begin(),m_VisitedNode.end(),CmpNodeForRender) };
			const auto minValueNeeded{ *std::min_element(m_VisitedNode.begin(),m_VisitedNode.end(),CmpNodeForRender) };

			float maxAlphaCost{};
			float minAlphaCost{};

			Elite::Color gradientLeft{};
			Elite::Color gradientRight{};

			if (m_SelectedShownGradient == 0)
			{
				gradientLeft = m_GCost.left;
				gradientRight = m_GCost.right;

				maxAlphaCost = maxValueNeeded.costSoFar;
				minAlphaCost = minValueNeeded.costSoFar;
			}
			else
			{
				gradientLeft = m_FCost.left;
				gradientRight = m_FCost.right;

				maxAlphaCost = maxValueNeeded.fCost;
				minAlphaCost = minValueNeeded.fCost;
			}

			const float rangeValue{ maxAlphaCost - minAlphaCost };
			
			for (const auto& node : m_VisitedNode)
			{
				const auto nodePos{ m_pGridGraph->GetNodeWorldPos(node.nodeIdx) };

				Elite::Vector2 verts[4]
				{
					Elite::Vector2(nodePos.x - halfCellSize, nodePos.y - halfCellSize),
					Elite::Vector2(nodePos.x - halfCellSize, nodePos.y + halfCellSize),
					Elite::Vector2(nodePos.x + halfCellSize, nodePos.y + halfCellSize),
					Elite::Vector2(nodePos.x + halfCellSize, nodePos.y - halfCellSize)
				};

				const float hueValue{ m_SelectedShownGradient == 0 ? node.costSoFar : node.fCost };
				const auto endColor{ GetGradientColor((hueValue - minAlphaCost) / rangeValue,gradientLeft,gradientRight) };

				DEBUGRENDERER2D->DrawSolidPolygon(verts, 4, endColor, -1.0f);
				if (m_ShowAbsoluteValue)
					DEBUGRENDERER2D->DrawString(nodePos, "%.1f", hueValue);

			}

			// Test Color
			//Elite::Vector2 verts[4]
			//{
			//	Elite::Vector2(-15.0f, 0.0f),
			//	Elite::Vector2(-15.0f, 15.0f),
			//	Elite::Vector2(0.0f, 15.0f),
			//	Elite::Vector2(0.0f, 0.0f)
			//};

			//DEBUGRENDERER2D->DrawSolidPolygon(verts, 4, gradientLeft, -1.0f);
		}

	}
}

void App_PathfindingAStar::MakeGridGraph()
{
	m_pGridGraph = new GridGraph<GridTerrainNode, GraphConnection>(
		COLUMNS,
		ROWS,
		m_SizeCell,
		false,
		false,
		1.f,
		1.5f);
	//m_pGridGraph->IsolateNode(6);
	//m_pGridGraph->GetNode(7)->SetTerrainType(TerrainType::Mud);
}

void App_PathfindingAStar::UpdateImGui()
{
#ifdef PLATFORM_WINDOWS
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

		ImGui::Text("A* Pathfinding");
		ImGui::Spacing();

		ImGui::Text("Middle Mouse");
		ImGui::Text("controls");
		std::string buttonText{ "" };
		if (m_StartSelected)
			buttonText += "Start Node";
		else
			buttonText += "End Node";

		if (ImGui::Button(buttonText.c_str()))
		{
			m_StartSelected = !m_StartSelected;
		}

		ImGui::Checkbox("Grid", &m_bDrawGrid);
		ImGui::Checkbox("NodeNumbers", &m_bDrawNodeNumbers);
		ImGui::Checkbox("Connections", &m_bDrawConnections);
		ImGui::Checkbox("Connections Costs", &m_bDrawConnectionsCosts);
		if (ImGui::Combo("", &m_SelectedHeuristic, "Manhattan\0Euclidean\0SqrtEuclidean\0Octile\0Chebyshev", 4))
		{
			switch (m_SelectedHeuristic)
			{
			case 0:
				m_pHeuristicFunction = HeuristicFunctions::Manhattan;
				break;
			case 1:
				m_pHeuristicFunction = HeuristicFunctions::Euclidean;
				break;
			case 2:
				m_pHeuristicFunction = HeuristicFunctions::SqrtEuclidean;
				break;
			case 3:
				m_pHeuristicFunction = HeuristicFunctions::Octile;
				break;
			case 4:
				m_pHeuristicFunction = HeuristicFunctions::Chebyshev;
				break;
			default:
				m_pHeuristicFunction = HeuristicFunctions::Chebyshev;
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

	if (m_ShowDebugOptions)
	{
		ImGui::Begin("AStar Debug Options");
		ImGui::Checkbox("Show Visited Node", &m_ShowVisitedNode);
		ImGui::Combo("", &m_SelectedShownGradient, "GCost\0FCost\0\0", 2);
		ImGui::Checkbox("Show Absolute Value", &m_ShowAbsoluteValue);

		ImVec4 colorHigh{};
		ImVec4 colorLow{};

		switch (m_SelectedShownGradient)
		{
		case 0:

			colorHigh = ImVec4{
				m_GCost.right.r,
				m_GCost.right.g,
				m_GCost.right.b,
				m_GCost.right.a
			};

			colorLow = ImVec4{
				m_GCost.left.r,
				m_GCost.left.g,
				m_GCost.left.b,
				m_GCost.left.a
			};

			break;

		case 1:

			colorHigh = ImVec4{
				m_FCost.right.r,
				m_FCost.right.g,
				m_FCost.right.b,
				m_FCost.right.a
			};

			colorLow = ImVec4{
				m_FCost.left.r,
				m_FCost.left.g,
				m_FCost.left.b,
				m_FCost.left.a
			};

			break;
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::ColorButton(colorHigh);
		ImGui::Text("High");
		ImGui::ColorButton(colorLow);
		ImGui::Text("Low");


		ImGui::End();
	}

#pragma endregion
#endif
}

//https://codereview.stackexchange.com/questions/167583/color-scale-in-range
Elite::Color App_PathfindingAStar::GetGradientColor(float value, const Elite::Color& c1, const Elite::Color& c2) const
{
	const float remap{ 1.0f - value };
	Elite::Color rangeLeft{ c1.r * remap,c1.g * remap,c1.b * remap,c1.a * remap };
	Elite::Color rangeRight{ c2.r * value,c2.g * value,c2.b * value,c2.a * value };
	return Elite::Color{
		rangeLeft.r + rangeRight.r,
		rangeLeft.g + rangeRight.g,
		rangeLeft.b + rangeRight.b,
		rangeLeft.a + rangeRight.a
	};
}
