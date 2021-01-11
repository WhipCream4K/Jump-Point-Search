#include "stdafx.h"
#include "TheFlock.h"

#include "../App_Steering/SteeringAgent.h"
#include "../App_Steering/SteeringBehaviors.h"
#include "CombinedSteeringBehaviors.h"

using namespace Elite;

//Constructor & Destructor
Flock::Flock(
	int flockSize /*= 50*/,
	float worldSize /*= 100.f*/,
	SteeringAgent* pAgentToEvade /*= nullptr*/,
	bool trimWorld /*= false*/)

	: m_FlockSize{ flockSize }
	, m_NeighborhoodRadius{ 15 }
	, m_NrOfNeighbors{ 0 }
	, m_pAgentToEvade{ pAgentToEvade }
	, m_TrimWorld{ trimWorld }
	, m_WorldSize{ worldSize }
	, m_SpacePartition(worldSize, worldSize, 25, 25, flockSize)
{
	// !! Wouldn't work with smart pointer
	// can't create object using unpossessed object
	m_pCohesion = new Cohesion(this);
	m_pSeparation = new Separation(this);
	m_pAlignment = new Alignment(this);
	m_pWander = new Wander();
	m_pSeek = new Seek();
	m_pSeek->SetTarget(m_MouseTarget);

	m_pBlendedSteering = new BlendedSteering({
		{m_pCohesion,0.2f},
		{m_pSeparation,0.2f},
		{m_pAlignment,0.2f},
		{m_pWander,0.2f},
		{m_pSeek,1.0f}
		});


	m_pEvade = new Evade();
	m_pEvade->SetUseFleeRadius(true);
	TargetData evadeTarget{};
	evadeTarget.LinearVelocity = m_pAgentToEvade->GetLinearVelocity();
	evadeTarget.Position = m_pAgentToEvade->GetPosition();
	evadeTarget.Orientation = m_pAgentToEvade->GetOrientation();
	m_pEvade->SetTarget(evadeTarget);
	m_pEvade->SetFleeRadius(m_NeighborhoodRadius);

	m_pPrioritySteering = new PrioritySteering({ m_pEvade,m_pBlendedSteering });

	m_Agents = std::vector<SteeringAgent*>{ size_t(flockSize) };

	for (int i = 0; i < flockSize; ++i)
	{
		auto steerAgent = new SteeringAgent();
		Elite::Vector2 randPos{ float(rand() % int(worldSize)),float(rand() % int(worldSize)) };

		steerAgent->SetAutoOrient(true);
		steerAgent->SetMass(1.0f);
		steerAgent->SetBodyColor({ 1.0f,1.0f,0.0f });
		steerAgent->SetMaxLinearSpeed(50.0f);
		steerAgent->SetSteeringBehavior(m_pPrioritySteering);
		steerAgent->SetPosition(randPos);

		m_Agents[i] = steerAgent;
	}

	m_Neighbors = std::vector<SteeringAgent*>{ size_t(flockSize) };
}

Flock::~Flock()
{
	for (auto& agent : m_Agents)
		SAFE_DELETE(agent)

		SAFE_DELETE(m_pEvade)
		SAFE_DELETE(m_pSeek)
		SAFE_DELETE(m_pAlignment)
		SAFE_DELETE(m_pCohesion)
		SAFE_DELETE(m_pSeparation)
		SAFE_DELETE(m_pWander)
		SAFE_DELETE(m_pBlendedSteering)
		SAFE_DELETE(m_pPrioritySteering)
}

void Flock::Update(float deltaT)
{
	// loop over all the boids
	// register its neighbors
	// update it
	// trim it to the world

	//INPUT
	if (INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eLeft))
	{
		auto const mouseData = INPUTMANAGER->GetMouseData(InputType::eMouseButton, InputMouseButton::eLeft);
		m_MouseTarget.Position = DEBUGRENDERER2D->GetActiveCamera()->
			ConvertScreenToWorld({ static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y) });
		m_pSeek->SetTarget(m_MouseTarget);
	}

	TargetData evadeTarget{};
	evadeTarget.LinearVelocity = m_pAgentToEvade->GetLinearVelocity();
	evadeTarget.Position = m_pAgentToEvade->GetPosition();
	evadeTarget.Orientation = m_pAgentToEvade->GetOrientation();

	m_pEvade->SetTarget(evadeTarget);


	for (const auto& boi : m_Agents)
	{
		const auto agentPos{ boi->GetPosition() };

		if (m_UseSpacePartition)
		{
			m_SpacePartition.UpdateAgentCell(boi, agentPos);
			m_SpacePartition.RegisterNeighbors(boi,agentPos, m_NeighborhoodRadius);
			m_Neighbors = m_SpacePartition.GetNeighbors();
			m_NrOfNeighbors = m_SpacePartition.GetNrOfNeighbors();
		}
		else
		{
			RegisterNeighbors(boi);
		}
		
		boi->Update(deltaT); // only updates steering

		if (m_TrimWorld)
			boi->TrimToWorld(m_WorldSize);
	}
}

void Flock::Render(float deltaT)
{

	// IDK Don't follow this part
	// Could be really bad
	if (m_CanRenderBehaviour)
	{
		const auto sampleAgent{ m_Agents[0] };
		//sampleAgent->SetRenderBehavior(true);
		DEBUGRENDERER2D->DrawCircle(
			sampleAgent->GetPosition(),
			m_NeighborhoodRadius,
			{ 0.0f,1.0f,0.0f },
			0.4f);

		for (const auto& agent : m_Agents)
			agent->SetBodyColor({ 1.0f,1.0f,0.0f });

		
		Color debugColor{};

		if(m_UseSpacePartition)
		{
			const auto agentPos{ sampleAgent->GetPosition() };
			debugColor = Color(0.96f,0.59f,0.93f);
			m_SpacePartition.RegisterNeighbors(sampleAgent,agentPos, m_NeighborhoodRadius);
			m_Neighbors = m_SpacePartition.GetNeighbors();
			m_NrOfNeighbors = m_SpacePartition.GetNrOfNeighbors();

			// Render bounding box
			const Elite::Vector2 rect[4]
			{
				{agentPos.x - m_NeighborhoodRadius,agentPos.y - m_NeighborhoodRadius},
				{agentPos.x + m_NeighborhoodRadius,agentPos.y - m_NeighborhoodRadius},
				{agentPos.x + m_NeighborhoodRadius,agentPos.y + m_NeighborhoodRadius},
				{agentPos.x - m_NeighborhoodRadius,agentPos.y + m_NeighborhoodRadius}
			};
			
			DEBUGRENDERER2D->DrawPolygon(rect, 4, { 0.0f,0.0f,1.0f }, 0.4f);
			
			m_SpacePartition.RenderNeighbourHoodCells(sampleAgent, m_NeighborhoodRadius);
			m_SpacePartition.RenderCells();
		}
		else
		{
			debugColor = Color(0.0f, 1.0f, 1.0f);
			RegisterNeighbors(sampleAgent);
		}

		sampleAgent->SetBodyColor(debugColor);
		for (int i = 0; i < m_NrOfNeighbors; ++i)
			m_Neighbors[i]->SetBodyColor({ 0.2f,0.8f,0.0f });

		for (const auto& boi : m_Agents)
			boi->Render(deltaT);
		
	}
	else
	{
		m_Agents[0]->SetBodyColor({ 1.0f,1.0f,0.0f });
		m_Agents[0]->SetRenderBehavior(false);
		for (const auto& agent : m_Agents)
			agent->SetBodyColor({ 1.0f,1.0f,0.0f });
	}
}

void Flock::UpdateAndRenderUI()
{
	//Setup
	int menuWidth = 235;
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
	
	ImGui::Text("Flocking");
	ImGui::Spacing();

	ImGui::SliderFloat("Neighbourhood Radius ", &m_NeighborhoodRadius,0.0f,50.0f);
	ImGui::Spacing();

	// Implement checkboxes and sliders here
	ImGui::SliderFloat("Cohesion", GetWeight(m_pCohesion), 0.0f, 1.0f);
	ImGui::Spacing();

	ImGui::SliderFloat("Separation", GetWeight(m_pSeparation), 0.0f, 1.0f);
	ImGui::Spacing();

	ImGui::SliderFloat("Alignment", GetWeight(m_pAlignment), 0.0f, 1.0f);
	ImGui::Spacing();

	ImGui::SliderFloat("Wander", GetWeight(m_pWander), 0.0f, 1.0f);
	ImGui::Spacing();

	ImGui::SliderFloat("Seek", GetWeight(m_pSeek), 0.0f, 1.0f);
	ImGui::Spacing();

	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Checkbox(" Debug Rendering", &m_CanRenderBehaviour);

	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Checkbox(" Use SpacePartition", &m_UseSpacePartition);

	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();
}

void Flock::RegisterNeighbors(SteeringAgent* pAgent)
{
	// register the agents neighboring the currently evaluated agent
	// store how many they are, so you know which part of the vector to loop over
	m_NrOfNeighbors = 0;
	for (int i = 0; i < m_FlockSize; ++i)
	{
		if (m_Agents[i] != pAgent)
		{
			const auto vecToNeighbour{ m_Agents[i]->GetPosition() - pAgent->GetPosition() };
			if (vecToNeighbour.MagnitudeSquared() <= m_NeighborhoodRadius * m_NeighborhoodRadius)
				m_Neighbors[m_NrOfNeighbors++] = m_Agents[i];

		}
	}
}

Elite::Vector2 Flock::GetAverageNeighborPos() const
{
	Elite::Vector2 avgNeighbourPos{};
	if (m_NrOfNeighbors > 0)
	{
		for (int i = 0; i < m_NrOfNeighbors; ++i)
		{
			avgNeighbourPos += m_Neighbors[i]->GetPosition();
		}
		avgNeighbourPos /= float(m_NrOfNeighbors);
	}
	return avgNeighbourPos;
}

Elite::Vector2 Flock::GetAverageNeighborVelocity() const
{
	Elite::Vector2 avgNeighbourVel{};
	if (m_NrOfNeighbors > 0)
	{
		for (int i = 0; i < m_NrOfNeighbors; ++i)
		{
			avgNeighbourVel += m_Neighbors[i]->GetLinearVelocity();
		}
		avgNeighbourVel /= float(m_NrOfNeighbors);
	}

	return avgNeighbourVel;
}


float* Flock::GetWeight(ISteeringBehavior* pBehavior)
{
	if (m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->m_WeightedBehaviors;
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehavior](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehavior;
			}
		);

		if (it != weightedBehaviors.end())
			return &it->weight;
	}

	return nullptr;
}
