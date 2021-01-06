#pragma once
#include "../SteeringHelpers.h"
#include "FlockingSteeringBehaviors.h"
#include "SpacePartitioning.h"

class ISteeringBehavior;
class SteeringAgent;
class BlendedSteering;
class PrioritySteering;

class Flock
{
public:
	Flock(
		int flockSize = 2000, 
		float worldSize = 200.f, 
		SteeringAgent* pAgentToEvade = nullptr, 
		bool trimWorld = false);

	~Flock();

	void Update(float deltaT , TargetData target);
	void UpdateAndRenderUI();
	void Render(float deltaT);

	void RegisterNeighbors(SteeringAgent* pAgent);
	const vector<SteeringAgent*>& GetNeighbors() const { return m_Neighbors; }
	void SetAgentToEvade(SteeringAgent* pAgent);

	Elite::Vector2 GetAverageNeighborPos() const;
	Elite::Vector2 GetAverageNeighborVelocity() const;
	const vector<SteeringAgent*>& GetNeighbours() const;
	int GetNrOfNeighBours() const;

private:
	// flock agents
	int m_FlockSize = 0;
	vector<SteeringAgent*> m_Agents;
	// neighborhood agents
	vector<SteeringAgent*> m_Neighbors;
	float m_NeighborhoodRadius = 10.f;
	int m_NrOfNeighbors = 0;

	// evade target
	SteeringAgent* m_pAgentToEvade;

	// world info
	bool m_TrimWorld = true;
	float m_WorldSize = 0.f;
	
	// steering Behaviors
	Cohesion* m_pCohesion;
	Seperation* m_pSeperation;
	VelocityMatch* m_PVelocityMatch;
	Seek* m_pSeek;
	Seek* m_pNormalSeek;

	Wander* m_pWander;
	Evade* m_pEvade;

	BlendedSteering* m_pBlendedSteering = nullptr;
	PrioritySteering* m_pPrioritySteering = nullptr;

	bool m_DebugRenderOn;
	bool m_DebugRenderOnPartitioning;
	bool m_PartitioningOn;

	// private functions
	float* GetWeight(ISteeringBehavior* pBehaviour);

	CellSpace *m_pCelspace;

private:
	Flock(const Flock& other);
	Flock& operator=(const Flock& other);
};