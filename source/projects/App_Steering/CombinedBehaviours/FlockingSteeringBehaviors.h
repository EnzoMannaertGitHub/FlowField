#pragma once
#include "../SteeringBehaviors.h"
//SEPARATION - FLOCKING
//*********************
class Flock;
class Seperation : public Seek
{
public:
	Seperation() = default;
	virtual ~Seperation() = default;
	void SetFlock(Flock* pFlock);

	//Seek Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
private:
	Flock* m_pFlock;
};

//COHESION - FLOCKING
//*******************
class Flock;
class Cohesion : public Seek
{
public:
	Cohesion() = default;
	virtual ~Cohesion() = default;
	void SetFlock(Flock* pFlock);

	//Seek Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
private:
	Flock* m_pFlock;
};

//VELOCITY MATCH - FLOCKING
//************************
class Flock;
class VelocityMatch : public Seek
{
public:
	VelocityMatch() = default;
	virtual ~VelocityMatch() = default;
	void SetFlock(Flock* pFlock);

	//Seek Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
private:
	Flock* m_pFlock;
};