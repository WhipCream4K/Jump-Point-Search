//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "SteeringAgent.h"

//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	steering.LinearVelocity = (m_Target).Position - pAgent->GetPosition();
	//Desired Velocity
	steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	//Rescale to Max Speed

	//DEBUG RENDERING
	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(),
			steering.LinearVelocity,
			5,
			{ 0, 1, 0 },
			0.4f);


	return steering;
}

//WANDER (base> SEEK)
//******
SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	const float halfJitter{ m_MaxJitter / 2.0f };
	const Elite::Vector2 randomOffset{
		Elite::Vector2{
			randomFloat(-halfJitter, halfJitter),
			randomFloat(-halfJitter, halfJitter)
		}
	};

	m_WanderTarget += randomOffset;
	m_WanderTarget.Normalize();
	m_WanderTarget *= m_Radius;

	// Add offset
	Elite::Vector2 offset{ pAgent->GetLinearVelocity() };
	offset.Normalize();
	offset *= m_Offset;

	m_Target = TargetData{ pAgent->GetPosition() + offset + m_WanderTarget };

	if (pAgent->CanRenderBehavior())
	{
		Elite::Vector2 pos{ pAgent->GetPosition() };
		DEBUGRENDERER2D->DrawSegment(pos,
			pos + offset,
			Color{ 0.0f, 0.0f, 1.0f, 0.5f },
			0.4f);
		DEBUGRENDERER2D->DrawCircle(pos + offset,
			m_Radius,
			Color{ 0.0f, 0.0f, 1.0f, 0.5f },
			0.4f);
		DEBUGRENDERER2D->DrawSolidCircle(pos + offset + m_WanderTarget,
			0.5f,
			Elite::Vector2{ 0.0f, 0.0f, },
			Color{ 1.0f, 0.0f, 0.0f, 0.5f },
			0.3f);
	}

	return Seek::CalculateSteering(deltaT, pAgent);
}


void Wander::PrintDebugOptions(int actorTag)
{
	const auto header{ GetHeader(actorTag) };

	ImGui::Begin(header.c_str());
	ImGui::Text("Wander Behaviour");
	ImGui::SliderFloat("Offset", &m_Offset, 0.0f, 100.0f);
	ImGui::SliderFloat("Radius", &m_Radius, 0.0f, 100.0f);
	ImGui::SliderFloat("MaxJitter", &m_MaxJitter, 0.0f, 5.0f);

	ImGui::End();
}

SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	const bool canDrawDebug{ pAgent->CanRenderBehavior() };
	if (m_UseFleeRadius)
	{
		// Sees if the agent is in our flee radius
		const Elite::Vector2 distanceFromAgent{
			m_Target.Position - pAgent->GetPosition()
		};
		const bool isInFleeRadius{
			distanceFromAgent.MagnitudeSquared() <=
			m_FleeRadius * m_FleeRadius
		};

		if (canDrawDebug)
		{
			Elite::Vector2 agentPos{ pAgent->GetPosition() };
			DEBUGRENDERER2D->DrawCircle(agentPos,
				m_FleeRadius,
				Color{ 0.0f, 0.0f, 1.0f, 0.5f },
				0.4f);
		}

		if (!isInFleeRadius)
		{
			steering.IsValid = false;
			return steering;
		}
	}

	// Then calculate the opposite of Seeking
	steering.LinearVelocity = pAgent->GetPosition() - m_Target.Position;
	//Desired Velocity
	steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	//Rescale to Max Speed

	if (canDrawDebug)
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(),
			steering.LinearVelocity,
			5,
			{ 0, 1, 0 },
			0.4f);

	return steering;
}

void Flee::PrintDebugOptions(int actorTag)
{
	const auto header{ GetHeader(actorTag) };

	ImGui::Begin(header.c_str());
	ImGui::Checkbox("UseFleeRadius", &m_UseFleeRadius);
	if (m_UseFleeRadius)
		ImGui::SliderFloat("FleeRadius", &m_FleeRadius, 0.0f, 50.0f);

	ImGui::End();
}

Elite::Vector2 Flee::CalculateFleeOutput(
	const Elite::Vector2& targetPos,
	const SteeringAgent& agent)
{
	Elite::Vector2 output{};
	output = targetPos - agent.GetPosition();
	output.Normalize();
	output *= -agent.GetMaxLinearSpeed();
	return output;
}

SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	// Calculate Future position
	SteeringOutput steering;

	const float distanceToTarget{
		(m_Target.Position - pAgent->GetPosition()).Magnitude()
	};
	const float dynamicFactor{ distanceToTarget / pAgent->GetMaxLinearSpeed() };

	const Elite::Vector2 futurePos{
		m_Target.Position + m_Target.LinearVelocity * dynamicFactor
	};

	m_Target = TargetData{ futurePos };

	return Seek::CalculateSteering(deltaT, pAgent);
}

SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	// Calculate Future position
	SteeringOutput steering;

	const float distanceToTarget{
		(m_Target.Position - pAgent->GetPosition()).Magnitude()
	};
	const float dynamicFactor{ distanceToTarget / pAgent->GetMaxLinearSpeed() };

	const Elite::Vector2 futurePos{
		m_Target.Position + m_Target.LinearVelocity * dynamicFactor
	};

	const bool isNearTarget{ distanceToTarget < m_FleeRadius };
	const bool isFutureInRangeSqr{Elite::DistanceSquared(pAgent->GetPosition(),futurePos) <= 
		m_FleeRadius * m_FleeRadius};

	if(isFutureInRangeSqr)
	{
		m_Target = TargetData{ futurePos };
	}
	else
	{
		if(isNearTarget)
		{
			return Flee::CalculateSteering(deltaT, pAgent);
		}
	}

	return Flee::CalculateSteering(deltaT, pAgent);
}

std::string ISteeringBehavior::GetHeader(int actorTag) const
{
	const auto actorNum{ std::to_string(actorTag) };
	return std::string("ACTOR " + actorNum);
}

SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	Elite::Vector2 vecToTarget{ m_Target.Position - pAgent->GetPosition() };
	float distanceToTarget{ vecToTarget.Normalize() };

	if (distanceToTarget < m_TargetRadius)
		steering.LinearVelocity = Elite::Vector2();
	
	else if (distanceToTarget < m_SlowRadius)
		steering.LinearVelocity = vecToTarget * pAgent->GetMaxLinearSpeed()
		* (distanceToTarget / m_SlowRadius);
	else
		steering.LinearVelocity = vecToTarget * pAgent->GetMaxLinearSpeed();

	//DEBUG RENDERING
	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(),
			steering.LinearVelocity,
			5,
			{ 0, 1, 0 },
			0.4f);
		DEBUGRENDERER2D->DrawCircle(m_Target.Position,
			m_SlowRadius,
			Color(0.0f, 0.0f, 1.0f),
			0.4f);
	}

	return steering;
}

void Arrive::PrintDebugOptions(int actorTag)
{
	const auto header{ GetHeader(actorTag) };
	ImGui::Begin(header.c_str());
	ImGui::SliderFloat("ArriveRadius", &m_SlowRadius, 1.0f, 100.0f);

	ImGui::End();
}

SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	pAgent->SetAutoOrient(false); // Manually rotate the agent

	const auto agentPos{ pAgent->GetPosition() };
	auto vecToTarget{ (m_Target.Position - agentPos) };
	vecToTarget.Normalize();
	const auto halfPI{ float(M_PI / 2.0) };
	const auto agentOrient{ pAgent->GetOrientation() };
	
	const auto vectorOffset{ agentOrient - halfPI };
	const Elite::Vector2 headingVec{ cosf(vectorOffset), sinf(vectorOffset) };
	const float targetAngle{ (atan2f(vecToTarget.y, vecToTarget.x) + halfPI) };
	const float diffAngle{ targetAngle - (atan2f(headingVec.y,headingVec.x) + halfPI)  };

	//const float forceTurnRight{ targetAngle > float(M_PI) ? -1.0f : 1.0f };
	//const float diffAngle{ acosf(Elite::Dot(vecToTarget,headingVec)) };
	steering.AngularVelocity = diffAngle * pAgent->GetMaxAngularSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(),
			headingVec,
			5.0f,
			Color(0, 1, 0),
			0.4f);

		DEBUGRENDERER2D->DrawSegment(agentPos, vecToTarget, Color(1.0f, 0.0f, 0.0f), 0.4f);
	}

	ImGui::Begin("DiffAngle");
	const auto text{ std::to_string(diffAngle) };
	ImGui::Text(text.c_str());
	ImGui::End();
	
	return steering;
}
