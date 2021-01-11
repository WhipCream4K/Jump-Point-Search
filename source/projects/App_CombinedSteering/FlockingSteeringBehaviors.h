#pragma once
#include "../App_Steering/SteeringBehaviors.h"

class Flock;
class IFlocking : public ISteeringBehavior
{
public:

	IFlocking(Flock* pSelf)
		: m_pFlockObj(pSelf)
	{
	}

	virtual ~IFlocking() = default;
	void SetNeighbours(const std::vector<SteeringAgent*>& neighbours) { m_Neighbours = &neighbours; }

protected:

	Flock* m_pFlockObj; // Behaviour doesn't own this flock object
	std::vector<SteeringAgent*>const * m_Neighbours{};
};

//SEPARATION - FLOCKING
//*********************
class Separation : public IFlocking
{
public:

	Separation(Flock* pSelf)
		: IFlocking(pSelf)
	{
	}

	~Separation() override = default;
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
	const char* GetBehaviourName() override { return m_Name; }

private:
	static constexpr const char* m_Name{ "Separation" };
};

//COHESION - FLOCKING
//*******************
class Cohesion : public IFlocking
{
public:

	Cohesion(Flock* pSelf)
		: IFlocking(pSelf)
	{
	}

	~Cohesion() override = default;
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
	const char* GetBehaviourName() override { return m_Name; }
private:
	static constexpr const char* m_Name{ "Cohesion" };
};

//VELOCITY MATCH - FLOCKING
//************************
class Alignment : public IFlocking
{
public:

	Alignment(Flock* pFlock)
		: IFlocking(pFlock)
	{
	}

	~Alignment() override = default;
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
	const char* GetBehaviourName() override { return m_Name; }
private:
	static constexpr const char* m_Name{ "Alignment" };
};