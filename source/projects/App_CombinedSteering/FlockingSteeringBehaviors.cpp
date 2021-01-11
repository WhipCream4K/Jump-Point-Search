#include "stdafx.h"
#include "FlockingSteeringBehaviors.h"
#include "TheFlock.h"
#include "../App_Steering/SteeringAgent.h"
#include "../App_Steering/SteeringHelpers.h"

//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	const auto& neighbours{ m_pFlockObj->GetNeighbors() };
	const int nrNeighbours{ m_pFlockObj->GetNrOfNeighbors() };
	if (nrNeighbours > 0)
	{
		Elite::Vector2 avgPosFromThisAgent{};
		for (int i = 0; i < nrNeighbours; ++i)
		{
			avgPosFromThisAgent += pAgent->GetPosition() - neighbours[i]->GetPosition();
		}
		const float magnitude{ avgPosFromThisAgent.Normalize() };
		if (magnitude > 0)
		{
			steering.LinearVelocity = avgPosFromThisAgent / magnitude;
			steering.LinearVelocity.Normalize();
			steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
		}
	}

	return steering;
}

//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	const int nrNeighbour{ m_pFlockObj->GetNrOfNeighbors() };
	if (nrNeighbour > 0)
	{
		const Elite::Vector2 averagePos{ m_pFlockObj->GetAverageNeighborPos() };

		steering.LinearVelocity = averagePos - pAgent->GetPosition();
		steering.LinearVelocity.Normalize();
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	}

	return steering;
}

//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput Alignment::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	const int nrNeighbours{ m_pFlockObj->GetNrOfNeighbors() };
	if (nrNeighbours > 0)
	{
		const auto avgVelocity{ m_pFlockObj->GetAverageNeighborVelocity() };

		steering.LinearVelocity = avgVelocity;
		steering.LinearVelocity.Normalize();
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

		return steering;
	}

	return steering;
}