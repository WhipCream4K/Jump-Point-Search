//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_Flocking.h"
#include "../App_Steering/SteeringAgent.h"
#include "TheFlock.h"

//Destructor
App_Flocking::~App_Flocking()
{	
	SAFE_DELETE(m_pFlock)
	SAFE_DELETE(m_pWandering)
	SAFE_DELETE(m_pEvadeAgent)
}

//Functions
void App_Flocking::Start()
{
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(55.0f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(m_TrimWorldSize / 1.5f, m_TrimWorldSize / 2));

	m_pWandering = new Wander();
	
	m_pEvadeAgent = new SteeringAgent();
	m_pEvadeAgent->SetAutoOrient(true);
	m_pEvadeAgent->SetMaxLinearSpeed(5.0f);
	m_pEvadeAgent->SetBodyColor({ 1.0f,0.0f,0.0f });
	m_pEvadeAgent->SetPosition({ 50.0f,50.0f });
	m_pEvadeAgent->SetSteeringBehavior(m_pWandering);
	
	m_pFlock = new Flock(4000,100,m_pEvadeAgent,true);
}

void App_Flocking::Update(float deltaTime)
{
	//INPUT
	if (INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eLeft) && m_VisualizeMouseTarget)
	{
		auto const mouseData = INPUTMANAGER->GetMouseData(InputType::eMouseButton, InputMouseButton::eLeft);
		m_MouseTarget.Position = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y) });
	}
	
	m_pWandering->PrintDebugOptions(0);
	m_pFlock->UpdateAndRenderUI();
	
	m_pEvadeAgent->Update(deltaTime);
	m_pEvadeAgent->TrimToWorld(m_TrimWorldSize);
	m_pFlock->Update(deltaTime);
}

void App_Flocking::Render(float deltaTime) const
{
	vector<Elite::Vector2> points =
	{
		{ -m_TrimWorldSize,m_TrimWorldSize },
		{ m_TrimWorldSize,m_TrimWorldSize },
		{ m_TrimWorldSize,-m_TrimWorldSize },
		{-m_TrimWorldSize,-m_TrimWorldSize }
	};
	DEBUGRENDERER2D->DrawPolygon(&points[0], 4, { 1,0,0,1 }, 0.4f);

	//Render Target
	if(m_VisualizeMouseTarget)
		DEBUGRENDERER2D->DrawSolidCircle(m_MouseTarget.Position, 0.3f, { 0.f,0.f }, { 1.f,0.f,0.f },-0.8f);

	m_pEvadeAgent->Render(deltaTime);
	m_pFlock->Render(deltaTime);
}
