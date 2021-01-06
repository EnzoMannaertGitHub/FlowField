#include "stdafx.h"
#include "FlockingSteeringBehaviors.h"
#include "TheFlock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"
//*********************
//SEPARATION (FLOCKING)
void Seperation::SetFlock(Flock* pFlock)
{
	m_pFlock = pFlock;
}

SteeringOutput Seperation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	Elite::Vector2 totalVector{};

	SteeringOutput steering = {};

	if (m_pFlock->GetNrOfNeighBours() == 0)
	{
		return steering;
	}

	std::vector<SteeringAgent*> neighbours{ m_pFlock->GetNeighbours() };
	int nrOfNeighbours{ m_pFlock->GetNrOfNeighBours() };

	for (int i{} ; i < nrOfNeighbours ; i++)
	{
		float distance{Distance(neighbours[i]->GetPosition() , pAgent->GetPosition())};
		float invertedDistance{ pAgent->GetRadius() - distance };
		Elite::Vector2 vector{};
		vector = pAgent->GetPosition() - neighbours[i]->GetPosition();
		vector *= -invertedDistance;
		vector.Normalize();
		totalVector += vector;
	}

	totalVector /= (float)nrOfNeighbours;
	steering.LinearVelocity = totalVector;
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	return steering;

}



//*******************
//COHESION (FLOCKING)
void Cohesion::SetFlock(Flock* pFlock)
{
	m_pFlock = pFlock;
}

SteeringOutput Cohesion::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	m_Target = m_pFlock->GetAverageNeighborPos();
	return Seek::CalculateSteering(deltaT, pAgent);

	return steering;
}

//*************************
//VELOCITY MATCH (FLOCKING)
void VelocityMatch::SetFlock(Flock* pFlock)
{
	m_pFlock = pFlock;
}

SteeringOutput VelocityMatch::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	steering.LinearVelocity = m_pFlock->GetAverageNeighborVelocity();
	return steering;
}