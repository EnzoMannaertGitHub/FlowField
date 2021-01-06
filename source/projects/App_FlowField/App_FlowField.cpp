//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_FlowField.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EFlowField.h"

#include "../App_Steering/SteeringAgent.h"
#include "../App_Steering/SteeringBehaviors.h"

using namespace Elite;

//Destructor
App_FlowField::~App_FlowField()
{
	SAFE_DELETE(m_pGridGraph);
}

//Functions
void App_FlowField::Start()
{
	m_IsBlueSelected = true;
	//Set Camera
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(1400.f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(1000, 1250.f));

	//Create Graph
	MakeGridGraph();

	for (auto n : m_pGridGraph->GetAllNodes() )
	{
		m_pIntegrationfieldBlue.push_back(new IntegrationFieldnode{n});
		m_pIntegrationfieldRed.push_back(new IntegrationFieldnode{n});
	}

	endPathIdxBlue = 25;
	endPathIdxRed = 75;

	GenerateFlowField(m_pGridGraph->GetNode(endPathIdxBlue));
	GenerateIntegrationField(m_pGridGraph->GetNode(endPathIdxBlue));

	m_pFlowfieldRed = m_pFlowfieldBlue;

	//----------- AGENT ------------
	m_pSeekBehavior = new Seek();
	m_pArriveBehavior = new Arrive();

	//blue agents
	int amountOfBlueAgents{ 50 };
	for (int i{}; i < amountOfBlueAgents; i++)
	{
		m_pBlueAgents.push_back(new SteeringAgent());
		m_pBlueAgents[i]->SetPosition({Elite::randomFloat(0 , float(m_SizeCell) * COLUMNS), Elite::randomFloat(0 , float(m_SizeCell) * ROWS) });
		m_pBlueAgents[i]->SetSteeringBehavior(m_pSeekBehavior);
		m_pBlueAgents[i]->SetMaxLinearSpeed( Elite::randomFloat(m_AgentSpeed - 10 ,m_AgentSpeed));
		m_pBlueAgents[i]->SetAutoOrient(true);
		m_pBlueAgents[i]->SetMass(0.1f);
		m_pBlueAgents[i]->SetRadius(75.f);
		m_pBlueAgents[i]->SetBodyColor(Elite::Color{0.f,1.f,1.f});
	}

	//Red Agents
	int amountOfRedAgents{ 50 };
	for (int i{}; i < amountOfRedAgents; i++)
	{
		m_pRedAgents.push_back(new SteeringAgent());
		m_pRedAgents[i]->SetPosition({ Elite::randomFloat(0 , float(m_SizeCell) * COLUMNS), Elite::randomFloat(0 , float(m_SizeCell) * ROWS) });
		m_pRedAgents[i]->SetSteeringBehavior(m_pSeekBehavior);
		m_pRedAgents[i]->SetMaxLinearSpeed(Elite::randomFloat(m_AgentSpeed - 10, m_AgentSpeed));
		m_pRedAgents[i]->SetAutoOrient(true);
		m_pRedAgents[i]->SetMass(0.1f);
		m_pRedAgents[i]->SetRadius(75.f);
		m_pRedAgents[i]->SetBodyColor(Elite::Color{ 1.f,0.f,0.f });
	}
}

void App_FlowField::Update(float deltaTime)
{
	UNREFERENCED_PARAMETER(deltaTime);

	//INPUT
	bool const middleMousePressed = INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eMiddle);
	if (middleMousePressed)
	{
		MouseData mouseData = { INPUTMANAGER->GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eMiddle) };
		Elite::Vector2 mousePos = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ (float)mouseData.X, (float)mouseData.Y });
		//Find closest node to click pos
		int closestNode = m_pGridGraph->GetNodeFromWorldPos(mousePos);
		if (m_StartSelected)
		{
			if (m_IsBlueSelected)
			{
			endPathIdxBlue = closestNode;
			m_UpdatePath = true;
			}
			else
			{
				endPathIdxRed = closestNode;
				m_UpdatePath = true;
			}
		}
		else
		{
			if (m_IsBlueSelected)
			{
				endPathIdxBlue = closestNode;
				m_UpdatePath = true;
			}
			else
			{
				endPathIdxRed = closestNode;
				m_UpdatePath = true;
			}
		}
	}

	//GRID INPUT
	bool hasGridChanged = m_GraphEditor.UpdateGraph(m_pGridGraph);
	if (hasGridChanged)
	{
		m_UpdatePath = true;
	}

	//IMGUI
	UpdateImGui();

	for (int i{}; i < (int)m_pBlueAgents.size(); i++)
	{
		int idx = invalid_node_index;
		int NodeIdx{};

		if (m_pBlueAgents[i]->GetPosition().x < 0 || m_pBlueAgents[i]->GetPosition().y < 0)
		{
			NodeIdx = idx;
		}

		int r, c;

		c = int(m_pBlueAgents[i]->GetPosition().x / m_SizeCell);
		r = int(m_pBlueAgents[i]->GetPosition().y / m_SizeCell);

		idx = r * COLUMNS + c;
		m_pBlueAgents[i]->SetLinearVelocity(m_pFlowfieldBlue[idx]->direction * m_AgentSpeed);
		m_pBlueAgents[i]->Update(deltaTime);
	}

	for (int i{}; i < (int)m_pRedAgents.size(); i++)
	{
		int idx = invalid_node_index;
		int NodeIdx{};

		if (m_pRedAgents[i]->GetPosition().x < 0 || m_pRedAgents[i]->GetPosition().y < 0)
		{
			NodeIdx = idx;
		}

		int r, c;

		c = int(m_pRedAgents[i]->GetPosition().x / m_SizeCell);
		r = int(m_pRedAgents[i]->GetPosition().y / m_SizeCell);

		idx = r * COLUMNS + c;

		m_pRedAgents[i]->SetLinearVelocity(m_pFlowfieldRed[idx]->direction * m_AgentSpeed);
		m_pRedAgents[i]->Update(deltaTime);
	}


	//Calculate flowfield field
	if (m_IsBlueSelected)
	{
		auto endNode = m_pGridGraph->GetNode(endPathIdxBlue);
		m_pGridGraph->GetNode(endNode->GetIndex())->SetTerrainType(TerrainType::goal);
		GenerateFlowField(endNode);
		GenerateIntegrationField(endNode);
		Render(deltaTime);

		m_UpdatePath = false;
	}
	else
	{
		auto endNode = m_pGridGraph->GetNode(endPathIdxRed);
		m_pGridGraph->GetNode(endNode->GetIndex())->SetTerrainType(TerrainType::goal);
		GenerateFlowField(endNode);
		GenerateIntegrationField(endNode);
		Render(deltaTime);

		m_UpdatePath = false;
	}
	std::cout << std::boolalpha << m_IsBlueSelected << std::endl;
}

void App_FlowField::Render(float deltaTime) const
{
	for (int i{}; i < (int)m_pBlueAgents.size(); i++)
	{
		m_pBlueAgents[i]->Render(deltaTime);
	}	
	for (int i{}; i < (int)m_pRedAgents.size(); i++)
	{
		m_pRedAgents[i]->Render(deltaTime);
	}

	UNREFERENCED_PARAMETER(deltaTime);
	//Render grid
	m_GraphRenderer.RenderGraph(
		m_pGridGraph, 
		m_bDrawGrid, 
		m_bDrawNodeNumbers, 
		m_bDrawConnections, 
		m_bDrawConnectionsCosts
	);
	
	//Render start node on top if applicable
	if (startPathIdx != invalid_node_index)
	{
		m_GraphRenderer.RenderHighlightedGrid(m_pGridGraph, { m_pGridGraph->GetNode(startPathIdx) }, START_NODE_COLOR);
	}

	//Render end node on top if applicable
	if (endPathIdxBlue != invalid_node_index)
	{
		m_GraphRenderer.RenderHighlightedGrid(m_pGridGraph, { m_pGridGraph->GetNode(endPathIdxBlue) }, END_NODE_COLOR_BLUE);
		m_GraphRenderer.RenderHighlightedGrid(m_pGridGraph, { m_pGridGraph->GetNode(endPathIdxRed) }, END_NODE_COLOR_RED);
	}
	
	//render path below if applicable
	if (m_vPath.size() > 0)
	{
		m_GraphRenderer.RenderHighlightedGrid(m_pGridGraph, m_vPath);
	}

	//render integrationfield
	if (m_DrawIntegrationField)
	{
		if (m_IsBlueSelected)
		{
			auto nodes = m_pGridGraph->GetAllNodes();
			int row{};
			int col{};
			int idx{ 0 };
			for (auto n : nodes)
			{

				float cost = m_pIntegrationfieldBlue[idx]->bestCost;
				DEBUGRENDERER2D->DrawString({ m_pGridGraph->GetNodePos(n->GetIndex()).x + (m_SizeCell * col) + m_SizeCell / 4.f , m_pGridGraph->GetNodePos(n->GetIndex()).y + (m_SizeCell * row) + m_SizeCell / 2.f }, std::to_string(int(cost)).c_str());
				idx++;
				col++;
				if (col >= COLUMNS)
				{
					col = 0;
					row++;
				}

			}
		}
		else
		{
			auto nodes = m_pGridGraph->GetAllNodes();
			int row{};
			int col{};
			int idx{ 0 };
			for (auto n : nodes)
			{

				float cost = m_pIntegrationfieldRed[idx]->bestCost;
				DEBUGRENDERER2D->DrawString({ m_pGridGraph->GetNodePos(n->GetIndex()).x + (m_SizeCell * col) + m_SizeCell / 4.f , m_pGridGraph->GetNodePos(n->GetIndex()).y + (m_SizeCell * row) + m_SizeCell / 2.f }, std::to_string(int(cost)).c_str());
				idx++;
				col++;
				if (col >= COLUMNS)
				{
					col = 0;
					row++;
				}

			}
		}
	}
	//render flowfield
	if (m_DrawFlowField)
	{
		if (m_IsBlueSelected)
		{
			auto nodes = m_pGridGraph->GetAllNodes();
			int row{};
			int col{};
			int idx{ 0 };
			for (auto n : nodes)
			{
				DEBUGRENDERER2D->DrawString({ m_pGridGraph->GetNodePos(n->GetIndex()).x + (m_SizeCell * col) + m_SizeCell / 4.f , m_pGridGraph->GetNodePos(n->GetIndex()).y + (m_SizeCell * row) + m_SizeCell / 2.f }, m_pFlowfieldBlue[idx]->arrow.c_str());
				idx++;
				col++;
				if (col >= COLUMNS)
				{
					col = 0;
					row++;
				}

			}
		}
		else
		{
			auto nodes = m_pGridGraph->GetAllNodes();
			int row{};
			int col{};
			int idx{ 0 };
			for (auto n : nodes)
			{
				DEBUGRENDERER2D->DrawString({ m_pGridGraph->GetNodePos(n->GetIndex()).x + (m_SizeCell * col) + m_SizeCell / 4.f , m_pGridGraph->GetNodePos(n->GetIndex()).y + (m_SizeCell * row) + m_SizeCell / 2.f }, m_pFlowfieldRed[idx]->arrow.c_str());
				idx++;
				col++;
				if (col >= COLUMNS)
				{
					col = 0;
					row++;
				}

			}
		}
	}

}

void App_FlowField::MakeGridGraph()
{
	m_pGridGraph = new GridGraph<GridTerrainNode, GraphConnection>(COLUMNS, ROWS, m_SizeCell, false, true, 1.f, 2.f);
}

void App_FlowField::UpdateImGui()
{
#ifdef PLATFORM_WINDOWS
#pragma region UI
	//UI
	{
		//Setup
		int menuWidth = 200;
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
		ImGui::Text("LMB: target");
		ImGui::Text("RMB: start");
		ImGui::Unindent();

		/*Spacing*/ImGui::Spacing();ImGui::Separator();ImGui::Spacing();ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		/*Spacing*/ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();

		ImGui::Text("Flow field");
		ImGui::Checkbox("Blue agents", &m_IsBlueSelected);
		ImGui::Spacing();

		ImGui::Text("Middle Mouse");
		ImGui::Text("controls");
		std::string buttonText{""};
		if (m_StartSelected)
			buttonText += "Start Node";
		else
			buttonText += "End Node";

		if (ImGui::Button(buttonText.c_str()))
		{
			m_StartSelected = !m_StartSelected;
		}

		ImGui::Checkbox("Grid", &m_bDrawGrid);
		ImGui::Checkbox("NodeNumbers", &m_bDrawNodeNumbers);
		ImGui::Checkbox("Connections", &m_bDrawConnections);
		ImGui::Checkbox("Connections Costs", &m_bDrawConnectionsCosts);

		ImGui::Spacing();
		ImGui::Text("FlowField debug info");
		ImGui::Checkbox("Draw flowfield", &m_DrawFlowField);
		ImGui::Checkbox("Draw integration field", &m_DrawIntegrationField); 


		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
#pragma endregion
#endif
}

void App_FlowField::GenerateIntegrationField(Elite::GridTerrainNode* endNode)
{
	std::vector<IntegrationFieldnode*> integrationfield;

	if (m_IsBlueSelected)
		integrationfield = m_pIntegrationfieldBlue;
	else
		integrationfield = m_pIntegrationfieldRed;

	//reset integration field best costs
	for (auto n : integrationfield)
	{
		n->bestCost = (int)TerrainType::Water;
	}

	std::vector<IntegrationFieldnode*> openList; //connections to be checked
	std::vector<IntegrationFieldnode*> closedList; //connections already checked

	IntegrationFieldnode* end = new IntegrationFieldnode{endNode};
	end->bestCost = 0;
	integrationfield[end->pTerrainGridNode->GetIndex()]->bestCost = 0;
	openList.push_back(end);

	//loop while we have nodes in openlist
	while (openList.size() > 0)
	{
		//loop over all nodes in openlist
		IntegrationFieldnode* currentNode = openList[0];

			//loop over each connection of every node
			for (auto c : m_pGridGraph->GetConnections(currentNode->pTerrainGridNode->GetIndex()))
			{
				IntegrationFieldnode* neighbourNode = integrationfield[(m_pGridGraph->GetNode(c->GetTo())->GetIndex())];
		
				int nodeCost = int(m_pGridGraph->GetNode(c->GetTo())->GetTerrainType());
				nodeCost += int(currentNode->bestCost * m_pGridGraph->GetConnection(c->GetFrom(),c->GetTo())->GetCost());
				if (nodeCost < neighbourNode->bestCost)
				{
					neighbourNode->bestCost = float(nodeCost);
				}

				auto closedListIT = std::find_if(closedList.begin(), closedList.end(), [neighbourNode](IntegrationFieldnode* pNode)
					{
						return pNode == neighbourNode;
					});

				if (closedListIT == closedList.end())
					openList.push_back(neighbourNode);
			}
			closedList.push_back(currentNode);
			openList.erase(std::remove(openList.begin(), openList.end(), currentNode), openList.end());
	}

	if (m_IsBlueSelected)
		m_pIntegrationfieldBlue = integrationfield;
	else
		m_pIntegrationfieldRed = integrationfield;
}

void App_FlowField::GenerateFlowField(Elite::GridTerrainNode* endNode)
{

	if (m_IsBlueSelected)
		m_pFlowfieldBlue.clear();
	else
		m_pFlowfieldRed.clear();

	bool isEnd{ false };
	auto nodes = m_pGridGraph->GetAllNodes();

	//loop over all the nodes
	for (auto n : nodes)
	{
		int lowestCostIndex{};
		float lowestCostvalue{ int(TerrainType::Water) }; //water is 200001

		//loop over all neighbouring nodes
		for (auto c : m_pGridGraph->GetNodeConnections(n->GetIndex()))
		{
			//check if to node is the end node
			if (m_pGridGraph->GetNode(c->GetFrom()) == endNode)
			{
				lowestCostIndex = m_pGridGraph->GetNode(c->GetFrom())->GetIndex();
				isEnd = true;
				break;
			}

			//get neighbour with best connectionCost
			//float currentCost = c->GetCost();
			float currentCost{};

			if (m_IsBlueSelected)
				currentCost = m_pIntegrationfieldBlue[m_pGridGraph->GetNode(c->GetTo())->GetIndex()]->bestCost;
			else
				currentCost = m_pIntegrationfieldRed[m_pGridGraph->GetNode(c->GetTo())->GetIndex()]->bestCost;


			if (currentCost < lowestCostvalue)
			{
				lowestCostvalue = currentCost;
				lowestCostIndex = c->GetTo();
			}
		}



		if (isEnd)
		{
			if (m_IsBlueSelected)
				m_pFlowfieldBlue.push_back(new FlowFieldNode{ "O" });
			else
				m_pFlowfieldRed.push_back(new FlowFieldNode{ "O" });

			isEnd = false;
		}
		else
		{
			FlowFieldNode *f;

			Elite::Vector2 currentpos = m_pGridGraph->GetNodePos(n->GetIndex());
			Elite::Vector2 destinationPos = m_pGridGraph->GetNodePos(lowestCostIndex);
			Elite::Vector2 direction = destinationPos - currentpos;

#pragma region Debug info
			if (destinationPos.x > currentpos.x && destinationPos.y > currentpos.y)
			{
				f = new FlowFieldNode{ "/>" };
			}
			else if (destinationPos.x > currentpos.x && destinationPos.y < currentpos.y)
			{
				f = new FlowFieldNode{ "\\>" };
			}
			else if (destinationPos.x > currentpos.x)
			{
				f = new FlowFieldNode{ ">" };
			}

			else if (destinationPos.x < currentpos.x && destinationPos.y > currentpos.y)
			{
				f = new FlowFieldNode{ "<\\" };
			}
			else if (destinationPos.x < currentpos.x && destinationPos.y < currentpos.y)
			{
				f = new FlowFieldNode{ "</" };
			}
			else if (destinationPos.x < currentpos.x)
			{
				f = new FlowFieldNode{ "<" };
			}

			else if (destinationPos.y > currentpos.y)
			{
				f = new FlowFieldNode{ "^" };
			}
			else if (destinationPos.y < currentpos.y)
			{
				f = new FlowFieldNode{ "V" };
			}
			else
			{
				f = new FlowFieldNode{ "?" };
			}
#pragma endregion

			f->direction = Elite::GetNormalized(direction);

			if (m_IsBlueSelected)
				m_pFlowfieldBlue.push_back(f);
			else
				m_pFlowfieldRed.push_back(f);
		}
	}
}