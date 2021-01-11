/*=============================================================================*/
// Copyright 2017-2018 Elite Engine
// Authors: Matthieu Delaere, Thomas Goussaert
/*=============================================================================*/
// SteeringBehaviors.h: SteeringBehaviors interface and different implementations
/*=============================================================================*/
#ifndef ELITE_STEERINGBEHAVIORS
#define ELITE_STEERINGBEHAVIORS

//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "SteeringHelpers.h"
class SteeringAgent;
using namespace Elite;

#pragma region **ISTEERINGBEHAVIOR** (BASE)
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	virtual SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) = 0;

	//Seek Functions
	void SetTarget(const TargetData& target) { m_Target = target; }

	template<class T, typename std::enable_if<std::is_base_of<ISteeringBehavior, T>::value>::type* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

	virtual void PrintDebugOptions(int actorTag) {}
	std::string GetHeader(int actorTag) const;
	
	virtual const char* GetBehaviourName() { return nullptr; }

protected:
	TargetData m_Target;
};
#pragma endregion

///////////////////////////////////////
//SEEK
//****
class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() override = default;

	//Seek Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
	const char* GetBehaviourName() override { return m_Name; }

private:
	
	static constexpr const char* m_Name{"Seek"};
};

//////////////////////////
//WANDER
//******
class Wander : public Seek
{
public:

	Wander() = default;
	virtual ~Wander() override = default;

	//Wander Behavior
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
	auto SetWanderOffset(float offset) -> void { m_Offset = offset; }

	void PrintDebugOptions(int actorTag) override;

	const char* GetBehaviourName() override { return m_Name; }

private:

	Elite::Vector2 m_WanderTarget;
	float m_Offset = 6.0f;
	float m_Radius = 4.0f;
	float m_MaxJitter = 1.0f;

	static constexpr const char* m_Name{ "Wander" };
};

class Flee : public ISteeringBehavior
{
public: 
	Flee() = default;
	virtual ~Flee() override = default;
	
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
	void SetFleeRadius(float radius) { m_FleeRadius = radius; }
	void SetUseFleeRadius(bool state) { m_UseFleeRadius = state; }

	void PrintDebugOptions(int actorTag) override;
	
protected:
	bool m_UseFleeRadius = true;
	float m_FleeRadius = 5.0f;
private:
	Elite::Vector2 CalculateFleeOutput(const Elite::Vector2& targetPos,const SteeringAgent& agent);
};

class Pursuit : public Seek
{
public: 
	Pursuit() = default;
	virtual ~Pursuit() override = default;
	
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
};

class Evade : public Flee
{
public:

	Evade() = default;
	virtual ~Evade() override = default;

	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

	void PrintDebugOptions(int actorTag) override { Flee::PrintDebugOptions(actorTag); }
	void SetUseEvadePlus(bool state) { m_UseEvadePlus = state; }

private:

	bool m_UseEvadePlus{};
};

class Arrive : public ISteeringBehavior
{
public:
	
	Arrive() = default;
	virtual ~Arrive() override = default;

	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
	void PrintDebugOptions(int actorTag) override;
	void SetSlowRadius(float value) { m_SlowRadius = value; }
	void SetTargetRadius(float value) { m_TargetRadius = value; }
	
private:
	
	float m_SlowRadius = 5.0f;
	float m_TargetRadius = 1.0f;
};

class Face : public ISteeringBehavior
{
public:
	
	Face() = default;
	virtual ~Face() override = default;

	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
};

#endif


