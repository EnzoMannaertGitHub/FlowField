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

class TheFlock;
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
	virtual ~Seek() = default;

	//Seek Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
};

//////////////////////////
//WANDER
//******
class Wander : public Seek
{
public:
	Wander() = default;
	virtual ~Wander() = default;

	//Wander Behavior
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

	void SetWanderOffset(float offset) { m_Offset = offset; }
	void SetWanderRadius(float radius) { m_Radius = radius; }
	void SetMaxAngleChange(float rad) { m_AngleChange = rad; }

protected:
	float m_Offset = 6.4f; //Offset (agent direction)
	float m_Radius = 4.f; //WanderRadius
	float m_AngleChange = ToRadians(45); //Max WanderAngle change per frame
	float m_WanderAngle = 0.f; //Internal
};

//////////////////////////
//FLEE
//******
class Flee : public Seek
{
public:
	Flee() = default;
	virtual ~Flee() = default;

	//Flee Behavior
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

private:
	float m_FleeRadius = 15.f;
};

//////////////////////////
//ARRIVE
//******
class Arrive : public Seek
{
public:
	Arrive() = default;
	virtual ~Arrive() = default;

	//Flee Behavior
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
};

class Face : public Seek
{
public:
	Face() = default;
	virtual ~Face() = default;

	//Flee Behavior
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
};
class Pursuit : public Seek
{
public:
	Pursuit() = default;
	virtual ~Pursuit() = default;

	//Flee Behavior
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
};
class Evade : public Flee
{
public:
	Evade() = default;
	virtual ~Evade() = default;
	
	//Flee Behavior
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
	void SetRadius(float radius);
private:
	float m_Radius;
};
#endif


