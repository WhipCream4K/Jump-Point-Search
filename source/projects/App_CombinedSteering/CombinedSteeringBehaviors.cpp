#include "stdafx.h"
#include "CombinedSteeringBehaviors.h"
#include <algorithm>
#include "../App_Steering/SteeringAgent.h"

BlendedSteering::BlendedSteering(vector<WeightedBehavior> weightedBehaviors)
	:m_WeightedBehaviors(weightedBehaviors)
{
};

//****************
//BLENDED STEERING
SteeringOutput BlendedSteering::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput blendedSteering = {};
	auto totalWeight = 0.f;

	for (auto weightedBehavior : m_WeightedBehaviors)
	{
		auto steering = weightedBehavior.pBehavior->CalculateSteering(deltaT, pAgent);
		blendedSteering.LinearVelocity += weightedBehavior.weight * steering.LinearVelocity;
		blendedSteering.AngularVelocity += weightedBehavior.weight * steering.AngularVelocity;

		totalWeight += weightedBehavior.weight;
	}

	if (totalWeight > 0.f)
	{
		auto scale = 1.f / totalWeight;
		blendedSteering *= scale;
	}

	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), blendedSteering.LinearVelocity, 7, { 0, 1, 1 }, 0.40f);

	return blendedSteering;
}

void BlendedSteering::PrintDebugOptions(int actorTag)
{
	const auto header{ GetHeader(actorTag) };
	for(auto& behaviour : m_WeightedBehaviors)
	{
		ImGui::Text(behaviour.pBehavior->GetBehaviourName());
		ImGui::SliderFloat("",&behaviour.weight, 0.0f, 1.0f);
		ImGui::Spacing();
	}
}

//*****************
//PRIORITY STEERING
SteeringOutput PrioritySteering::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	for (auto pBehavior : m_PriorityBehaviors)
	{
		steering = pBehavior->CalculateSteering(deltaT, pAgent);

		if (steering.IsValid)
			break;
	}

	//If non of the behavior return a valid output, last behavior is returned
	return steering;
}

void PrioritySteering::PrintDebugOptions(int actorTag)
{
	for (const auto& behavior : m_PriorityBehaviors)
		behavior->PrintDebugOptions(actorTag);
}
