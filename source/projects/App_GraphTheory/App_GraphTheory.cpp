//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_GraphTheory.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EEularianPath.h"

using namespace Elite;

//Destructor
App_GraphTheory::~App_GraphTheory()
{
	SAFE_DELETE(m_pGraph2D)
	SAFE_DELETE(m_pEulerFinder)
}

//Functions
void App_GraphTheory::Start()
{
	//Initialization of your application. If you want access to the physics world you will need to store it yourself.
	//----------- CAMERA ------------
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(80.f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(0, 0));
	DEBUGRENDERER2D->GetActiveCamera()->SetMoveLocked(false);
	DEBUGRENDERER2D->GetActiveCamera()->SetZoomLocked(false);

	// Initialize Graph
	m_pGraph2D = new Graph2D<GraphNode2D, GraphConnection2D>{false};
	m_pGraph2D->AddNode(new GraphNode2D(0, { 20.0f,30.0f }));
	m_pGraph2D->AddNode(new GraphNode2D(1, { -10.0f,-10.0f }));
	m_pGraph2D->AddConnection(new GraphConnection2D(0, 1));

	m_pEulerFinder = new EulerianPath<GraphNode2D, GraphConnection2D>(m_pGraph2D);
}

void App_GraphTheory::Update(float deltaTime)
{
	m_pGraph2D->Update();
	m_pGraph2D->SetConnectionCostsToDistance();

	if(m_FindEulerPath)
	{
		auto eulerianity{ m_pEulerFinder->IsEulerian() };
		const auto path{ m_pEulerFinder->FindPath(eulerianity) };
	}
	//switch (eulerianity)
	//{
	//case Eulerianity::notEulerian: break;
	//case Eulerianity::semiEulerian: break;
	//case Eulerianity::eulerian: 

	//	path = m_pEulerFinder->FindPath(eulerianity);

	//	break;
	//default: ;
	//}
	//const auto eulerFinder{ EulerianPath<GraphNode2D,GraphConnection2D>(m_pGraph2D) };
	//const auto eulerianity{ eulerFinder.IsEulerian() };
	//switch (eulerianity)
	//{
	//case Eulerianity::notEulerian:	std::cout << "not Eulerian" << '\n'; break;
	//case Eulerianity::semiEulerian:	std::cout << "semi Eulerian" << '\n';	break;
	//case Eulerianity::eulerian:		std::cout << "eulerian\n"; break;
	//}

	//------- UI --------
#ifdef PLATFORM_WINDOWS
#pragma region UI
	{
		//Setup
		int menuWidth = 150;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 90));
		ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::PushAllowKeyboardFocus(false);
		ImGui::SetWindowFocus();
		ImGui::PushItemWidth(70);
		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("Graph Theory");
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Separator();
		ImGui::Checkbox("Find Euler Path", &m_FindEulerPath);

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
#pragma endregion
#endif
	

}

void App_GraphTheory::Render(float deltaTime) const
{
	//m_pGraph2D->Render();
	m_GraphRenderer.RenderGraph(m_pGraph2D, true, true);
}
