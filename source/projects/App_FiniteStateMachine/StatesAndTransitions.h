/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// StatesAndTransitions.h: Implementation of the state/transition classes
/*=============================================================================*/
#ifndef ELITE_APPLICATION_FSM_STATES_TRANSITIONS
#define ELITE_APPLICATION_FSM_STATES_TRANSITIONS

#include "../Shared/Agario/AgarioAgent.h"
#include "../Shared/Agario/AgarioFood.h"
#include "../App_Steering/SteeringBehaviors.h"

// STATES
//
class WanderState : public Elite::FSMState
{
public:
	WanderState() = default;
	void OnEnter(Blackboard* pBlackboard) override
	{
		AgarioAgent* pAgent{};
		bool dataAvailable{ pBlackboard->GetData("Agent", pAgent) };

		if (!dataAvailable)
			return;
		if (!pAgent)
			return;

		pAgent->SetToWander();
	}
};

class SeekState : public Elite::FSMState
{
public:
	SeekState() = default;
	void OnEnter(Blackboard* pBlackboard) override
	{
		AgarioAgent* pAgent{};
		bool isDataAvailable{ pBlackboard->GetData("Agent",pAgent) };
		if(isDataAvailable)
		{
			Elite::Vector2 seekTarget{};
			isDataAvailable = pBlackboard->GetData("SeekTarget", seekTarget);
			if(isDataAvailable && seekTarget.Magnitude() > 0) // Bad
				pAgent->SetToSeek(seekTarget);
			
		}
	}
};

// TRANSITION
//
class HasFoodCloseby : public Elite::FSMTransition
{
public:
	HasFoodCloseby(float checkRadius)
		: FSMTransition()
		, m_CheckRadius(checkRadius)
	{
	}

	bool ToTransition(Blackboard* pBlackboard) const override
	{
		// 1. Get our agent from the blackboard
		// 2. Get food info from the blackboard
		// 3. Check if food is closeby

		AgarioAgent* pAgent{};
		pBlackboard->GetData("Agent", pAgent);

		std::vector<AgarioFood*> pFoodVec{};
		pBlackboard->GetData("FoodVec", pFoodVec);

		for (const auto& food : pFoodVec)
		{
			Elite::Vector2 vecToFood{ pAgent->GetPosition() - food->GetPosition() };
			if(Elite::DistanceSquared(pAgent->GetPosition(),food->GetPosition()) > m_CheckRadius * m_CheckRadius)
				continue;
			pBlackboard->ChangeData("SeekTarget", food->GetPosition());
			return true;
		}

		//pAgent->SetToWander();
		return false;
	}

private:
	float m_CheckRadius{};
};

class NoTarget : public Elite::FSMTransition
{
public:
	NoTarget() = default;

	bool ToTransition(Blackboard* pBlackboard) const override
	{
		AgarioAgent* pAgent{};
		bool isDataAvailable{ pBlackboard->GetData("Agent",pAgent) };
		if(isDataAvailable)
		{
			const float epsilon{ 0.2f };
			Elite::Vector2 seekTarget{};
			pBlackboard->GetData("SeekTarget", seekTarget);
			const float sqrtDiff{ Elite::DistanceSquared(pAgent->GetPosition(),seekTarget) };
			if (sqrtDiff > epsilon * epsilon)
				return false;
			return true;
		}
	}
};

#endif