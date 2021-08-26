//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_CombinedSteering.h"
#include "../App_Steering/SteeringAgent.h"
#include "CombinedSteeringBehaviors.h"
#include "projects\App_Steering\Obstacle.h"

App_CombinedSteering::~App_CombinedSteering()
{
	SAFE_DELETE(m_pWander);
	SAFE_DELETE(m_pSeek);
	SAFE_DELETE(m_pBlendSteering);
	SAFE_DELETE(m_pDrunkSeekAgent);
}

void App_CombinedSteering::Start()
{
	m_pDrunkSeekAgent = new SteeringAgent();
	
	m_pWander = new Wander();
	m_pSeek = new Seek();
	m_pWander->SetWanderOffset(0);
	
	m_pBlendSteering = new BlendedSteering(
	{
		{ m_pWander,0.5f },
		{ m_pSeek,0.5f }	
	});
	
	m_pDrunkSeekAgent->SetSteeringBehavior(m_pBlendSteering);
	m_pDrunkSeekAgent->SetAutoOrient(true);
	m_pDrunkSeekAgent->SetMass(1.0f);
	m_pDrunkSeekAgent->SetMaxLinearSpeed(7.0f);
}

void App_CombinedSteering::Update(float deltaTime)
{
	//INPUT
	if (INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eLeft) && m_VisualizeMouseTarget)
	{
		auto const mouseData = INPUTMANAGER->GetMouseData(InputType::eMouseButton, InputMouseButton::eLeft);
		m_MouseTarget.Position = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y) });
			
		if (m_pSeek)
			m_pSeek->SetTarget(m_MouseTarget);
	}

#ifdef PLATFORM_WINDOWS
	#pragma region UI
	//UI
	{
		//Setup
		int const menuWidth = 235;
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
		ImGui::Text("LMB: place target");
		ImGui::Text("RMB: move cam.");
		ImGui::Text("Scrollwheel: zoom cam.");
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

		ImGui::Text("Flocking");
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Checkbox("Debug Rendering", &m_CanDebugRender);
		ImGui::Checkbox("Trim World", &m_TrimWorld);
		if (m_TrimWorld)
		{
			ImGui::SliderFloat("Trim Size", &m_TrimWorldSize, 0.f, 500.f, "%1.");
		}
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
	#pragma endregion
#endif

	if (m_pDrunkSeekAgent)
	{
		m_pDrunkSeekAgent->SetRenderBehavior(m_CanDebugRender);

		if (m_pBlendSteering)
			m_pBlendSteering->PrintDebugOptions(0);

		m_pDrunkSeekAgent->Update(deltaTime);
		if (m_TrimWorld)
			m_pDrunkSeekAgent->TrimToWorld(m_TrimWorldSize);
	}
}

void App_CombinedSteering::Render(float deltaTime) const
{
	if (m_pDrunkSeekAgent)
		m_pDrunkSeekAgent->Render(deltaTime);

	if (m_TrimWorld)
	{
		std::vector<Elite::Vector2> points =
		{
			{ -m_TrimWorldSize,m_TrimWorldSize },
			{ m_TrimWorldSize,m_TrimWorldSize },
			{ m_TrimWorldSize,-m_TrimWorldSize },
			{-m_TrimWorldSize,-m_TrimWorldSize }
		};
		DEBUGRENDERER2D->DrawPolygon(&points[0], 4, { 1,0,0,1 }, 0.4f);
	}

	//Render Target
	if(m_VisualizeMouseTarget)
		DEBUGRENDERER2D->DrawSolidCircle(m_MouseTarget.Position, 0.3f, { 0.f,0.f }, { 1.f,0.f,0.f },-0.8f);
}
