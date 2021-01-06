#include "stdafx.h"
#include "TheFlock.h"

#include "../SteeringAgent.h"
#include "../SteeringBehaviors.h"
#include "CombinedSteeringBehaviors.h"

using namespace Elite;

//Constructor & Destructor
Flock::Flock(
	int flockSize /*= 50*/, 
	float worldSize /*= 100.f*/, 
	SteeringAgent* pAgentToEvade /*= nullptr*/, 
	bool trimWorld /*= false*/)

	: m_WorldSize{ worldSize }
	, m_FlockSize{ flockSize }
	, m_TrimWorld { trimWorld }
	, m_pAgentToEvade{pAgentToEvade}
	, m_NeighborhoodRadius{15}
	, m_NrOfNeighbors{0}
	, m_pCohesion{ new Cohesion{} }
	, m_pSeperation{ new Seperation{} }
	, m_PVelocityMatch{ new VelocityMatch{} }
	, m_pSeek{ new Seek{} }
	, m_pWander{ new Wander{} }
	, m_DebugRenderOn{false}
	, m_pEvade{ new Evade{} }
	, m_pNormalSeek{ new Seek{} }
	, m_pCelspace{ new CellSpace{ worldSize , worldSize , 25 , 25 , flockSize} }
	, m_DebugRenderOnPartitioning{false}
	, m_PartitioningOn{true}
{
	m_Agents.resize(flockSize);
	m_Neighbors.resize(flockSize - 1);

	m_pEvade->SetRadius(m_NeighborhoodRadius);
	m_pCohesion->SetFlock(this);
	m_pSeperation->SetFlock(this);
	m_PVelocityMatch->SetFlock(this);
	m_pBlendedSteering = new BlendedSteering{ { { m_pCohesion , 0.2f } , { m_pSeperation , 0.2f } , { m_PVelocityMatch , 0.2f } , { m_pSeek , 0.2f } , { m_pWander , 0.2f } } };
	m_pPrioritySteering = new PrioritySteering{ { m_pEvade , m_pBlendedSteering }  };
	for (int i = 0; i < flockSize; i++)
	{
		m_Agents[i] = new SteeringAgent{};
		float xPos{};
		float yPos{};
		xPos = randomFloat(worldSize);
		yPos = randomFloat(worldSize);

		Elite::Vector2 randVect{ xPos , yPos};
		m_Agents[i]->SetPosition(randVect);
		m_Agents[i]->SetSteeringBehavior(m_pPrioritySteering);
		m_Agents[i]->SetMaxLinearSpeed(m_Agents[i]->GetMaxLinearSpeed()* 3.f);
		m_pCelspace->AddAgent(m_Agents[i]);
	}

}

Flock::~Flock()
{
	for (SteeringAgent* pAgent : m_Agents)
	{
		SAFE_DELETE(pAgent);
	}
	SAFE_DELETE(m_pSeek);
	SAFE_DELETE(m_pWander);
	SAFE_DELETE(m_pSeperation);
	SAFE_DELETE(m_PVelocityMatch);
	SAFE_DELETE(m_pCohesion);
	SAFE_DELETE(m_pBlendedSteering);
	SAFE_DELETE(m_pEvade);
	SAFE_DELETE(m_pPrioritySteering);
	SAFE_DELETE(m_pNormalSeek);

}

void Flock::Update(float deltaT , TargetData target)
{
	m_pCelspace->SetDebugRendering(m_DebugRenderOnPartitioning);

	m_pNormalSeek->SetTarget(target);
	m_TrimWorld = true;
	TargetData evadeTarget{};
	//evadeTarget.LinearVelocity = m_pAgentToEvade->GetLinearVelocity();
	evadeTarget.Position = m_pAgentToEvade->GetPosition();
	m_pEvade->SetTarget(evadeTarget);

	// loop over all the boids
	// register its neighbors
	// update it
	// trim it to the world
	for (SteeringAgent* pAgent : m_Agents)
	{	
		m_pSeek->SetTarget(target);
		if (m_PartitioningOn)
		{
			pAgent->Update(deltaT);

			m_pCelspace->RegisterNeighbors(pAgent, m_NeighborhoodRadius);

			pAgent->TrimToWorld({ 0 , 0 }, { m_WorldSize , m_WorldSize });

			m_pCelspace->UpdateAgentCell(pAgent, pAgent->m_OldPos);
			pAgent->m_OldPos = pAgent->GetPosition();

		}
		else
		{
			pAgent->Update(deltaT);
			RegisterNeighbors(pAgent);

			pAgent->TrimToWorld({ 0 , 0 }, { m_WorldSize , m_WorldSize });
		}
	}
}

void Flock::Render(float deltaT)
{
	m_pCelspace->RenderCells();

	if (m_DebugRenderOn)
	{
		DEBUGRENDERER2D->DrawCircle(m_Agents[0]->GetPosition(), m_NeighborhoodRadius, { 0,0,1 }, 0.40f);
	}

	for (SteeringAgent* pAgent : m_Agents)
	{
		pAgent->Render(deltaT);
		pAgent->SetRenderBehavior(m_DebugRenderOn);
	}
	//Debug render
	if (m_DebugRenderOn)
	{
		DEBUGRENDERER2D->DrawCircle(m_Agents[0]->GetPosition(), m_NeighborhoodRadius, { 1 ,0 ,0 }, 0.40f);
	}

}

void Flock::UpdateAndRenderUI()
{
	//Setup
	int menuWidth = 235;
	int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
	int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
	bool windowActive = true;
	ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
	ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
	ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::PushAllowKeyboardFocus(false);

	//Elements
	ImGui::Text("CONTROLS");
	ImGui::Indent();
	ImGui::Text("LMB: place target");
	ImGui::Text("RMB: move cam.");
	ImGui::Text("Scrollwheel: zoom cam.");
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Text("STATS");
	ImGui::Indent();
	ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Flocking");
	ImGui::Spacing();

	// Implement checkboxes and sliders here

	ImGui::Checkbox("Enable partitioning", &m_PartitioningOn);
	ImGui::Spacing();
	ImGui::Checkbox("Debug render", &m_DebugRenderOn);
	ImGui::Spacing();
	ImGui::Checkbox("Debug render partioning", &m_DebugRenderOnPartitioning);
	ImGui::Spacing();


	ImGui::SliderFloat("Cohesion", &m_pBlendedSteering->m_WeightedBehaviors[0].weight, 0.f, 1.f, "%.2");
	ImGui::Spacing();
	ImGui::SliderFloat("Seperation", &m_pBlendedSteering->m_WeightedBehaviors[1].weight, 0.f, 1.f, "%.2");
	ImGui::Spacing();
	ImGui::SliderFloat("Allignment", &m_pBlendedSteering->m_WeightedBehaviors[2].weight, 0.f, 1.f, "%.2");
	ImGui::Spacing();
	ImGui::SliderFloat("Seek", &m_pBlendedSteering->m_WeightedBehaviors[3].weight, 0.f, 1.f, "%.2");
	ImGui::Spacing();
	ImGui::SliderFloat("Wander", &m_pBlendedSteering->m_WeightedBehaviors[4].weight, 0.f, 1.f, "%.2");


	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();
	
}

void Flock::RegisterNeighbors(SteeringAgent* pAgent)
{
	// register the agents neighboring the currently evaluated agent
	// store how many they are, so you know which part of the vector to loop over
	float distance{}; 
	m_NrOfNeighbors = 0;
	for (SteeringAgent* pAgent2 : m_Agents)
	{
		if (pAgent != pAgent2)
		{
			distance = Distance(pAgent->GetPosition(), pAgent2->GetPosition());
			if (distance <= m_NeighborhoodRadius)
			{
				pAgent2->SetBodyColor({ 1 , 0 , 1 });
				m_Neighbors[m_NrOfNeighbors] = pAgent2;
				m_NrOfNeighbors++;
			}
			else
			{
				pAgent2->SetBodyColor({ 1 , 1 , 0 });
			}
		}
	}
}

Elite::Vector2 Flock::GetAverageNeighborPos() const
{
	int nrOfNeighbours{ GetNrOfNeighBours() };
	Elite::Vector2 sum{};
	if (nrOfNeighbours == 0) return sum;
	for (int i{}; i < nrOfNeighbours; i++)
	{
		sum += GetNeighbours()[i]->GetPosition();
	}

	return sum / (float)nrOfNeighbours;
}

Elite::Vector2 Flock::GetAverageNeighborVelocity() const
{
	int nrOfNeighbours{ GetNrOfNeighBours() };
	Elite::Vector2 sum{};
	if (nrOfNeighbours == 0) return sum;
	for (int i{}; i < nrOfNeighbours; i++)
	{
		sum += GetNeighbours()[i]->GetLinearVelocity();
	}

	return sum / (float)nrOfNeighbours;
}

float* Flock::GetWeight(ISteeringBehavior* pBehavior) 
{
	if (m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->m_WeightedBehaviors;
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehavior](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehavior;
			}
		);

		if(it!= weightedBehaviors.end())
			return &it->weight;
	}

	return nullptr;
}

void Flock::SetAgentToEvade(SteeringAgent* pAgent)
{
	m_pAgentToEvade = pAgent;
}

const vector<SteeringAgent*>& Flock::GetNeighbours() const
{
	if (m_PartitioningOn)
	{
		return m_pCelspace->GetNeighbors();
	}
	else
	{
		return m_Neighbors;
	}
}

int Flock::GetNrOfNeighBours() const
{
	if (m_PartitioningOn)
	{
		return m_pCelspace->GetNrOfNeighbors();
	}
	else
	{
		return m_NrOfNeighbors;
	}
}


