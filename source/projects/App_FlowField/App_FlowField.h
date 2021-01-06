#ifndef ASTAR_APPLICATION_H
#define ASTAR_APPLICATION_H
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteInterfaces/EIApp.h"
#include "framework\EliteAI\EliteGraphs\EGridGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphEditor.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphRenderer.h"
#include "../App_Steering/SteeringHelpers.h"

struct FlowFieldNode
{
	struct FlowFieldNode(std::string arrow) :
		arrow {arrow}
	{

	}
	std::string arrow;
	Elite::Vector2 direction;
};
struct IntegrationFieldnode
{
	struct IntegrationFieldnode(Elite::GridTerrainNode* pGridNode) :
		pTerrainGridNode {pGridNode}
	{

	}
	Elite::GridTerrainNode* pTerrainGridNode;
	float bestCost = 200001;
};

//-----------------------------------------------------------------
// Application
//-----------------------------------------------------------------
class SteeringAgent;
class Seek;
class Arrive;
class App_FlowField final : public IApp
{
public:
	//Constructor & Destructor
	App_FlowField() = default;
	virtual ~App_FlowField();

	//App Functions
	void Start() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) const override;

private:
	//Datamembers
	std::vector<SteeringAgent*> m_pBlueAgents;
	std::vector<SteeringAgent*> m_pRedAgents;
	//SteeringAgent* m_pAgent = nullptr;
	Seek* m_pSeekBehavior = nullptr;
	Arrive* m_pArriveBehavior = nullptr;
	float m_AgentSpeed = 500.f;

	const bool ALLOW_DIAGONAL_MOVEMENT = true;
	Elite::Vector2 m_StartPosition = Elite::ZeroVector2;
	Elite::Vector2 m_TargetPosition = Elite::ZeroVector2;

	//Grid datamembers
	static const int COLUMNS = 20;
	static const int ROWS = 20;
	unsigned int m_SizeCell = 200;
	Elite::GridGraph<Elite::GridTerrainNode, Elite::GraphConnection>* m_pGridGraph;
	bool m_IsBlueSelected;
	std::vector<FlowFieldNode*> m_pFlowfieldBlue;
	std::vector<IntegrationFieldnode*> m_pIntegrationfieldBlue;
	
	std::vector<FlowFieldNode*> m_pFlowfieldRed;
	std::vector<IntegrationFieldnode*> m_pIntegrationfieldRed;

	//Pathfinding datamembers
	int startPathIdx = invalid_node_index;
	int endPathIdxBlue = invalid_node_index;
	int endPathIdxRed = invalid_node_index;
	std::vector<Elite::GridTerrainNode*> m_vPath;
	bool m_UpdatePath = true;

	//Editor and Visualisation
	Elite::EGraphEditor m_GraphEditor{};
	Elite::EGraphRenderer m_GraphRenderer{};

	//Debug rendering information
	bool m_bDrawGrid = true;
	bool m_bDrawNodeNumbers = false;
	bool m_bDrawConnections = false;
	bool m_bDrawConnectionsCosts = false;
	bool m_StartSelected = true;
	int m_SelectedHeuristic = 4;
	bool m_DrawFlowField = false;
	bool m_DrawIntegrationField = false;
	Elite::Heuristic m_pHeuristicFunction = Elite::HeuristicFunctions::Chebyshev;

	//Functions
	void MakeGridGraph();
	void UpdateImGui();
	void GenerateFlowField(Elite::GridTerrainNode* endNode);
	void GenerateIntegrationField(Elite::GridTerrainNode* endNode);
	//C++ make the class non-copyable
	App_FlowField(const App_FlowField&) = delete;
	App_FlowField& operator=(const App_FlowField&) = delete;
};
#endif