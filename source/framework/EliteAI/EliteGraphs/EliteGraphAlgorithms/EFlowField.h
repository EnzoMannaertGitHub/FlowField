#pragma once

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class FlowField
	{
	public:
		FlowField(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

		// stores the optimal connection to a node and its total costs related to the start and end node of the path
		struct NodeRecord
		{
			T_NodeType* pNode = nullptr;
			T_ConnectionType* pConnection = nullptr;
			float costSoFar = 0.f; // accumulated g-costs of all the connections leading up to this one
			float estimatedTotalCost = 0.f; // f-cost (= costSoFar + h-cost)
		
			bool operator==(const NodeRecord& other) const
			{
				return pNode == other.pNode
					&& pConnection == other.pConnection
					&& costSoFar == other.costSoFar
					&& estimatedTotalCost == other.estimatedTotalCost;
			};

			bool operator<(const NodeRecord& other) const
			{
				return estimatedTotalCost < other.estimatedTotalCost;
			};
		};

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);

	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template <class T_NodeType, class T_ConnectionType>
	FlowField<T_NodeType, T_ConnectionType>::FlowField(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph(pGraph)
		, m_HeuristicFunction(hFunction)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> FlowField<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pGoalNode)
	{
		//integration field maken 
		std::vector<T_NodeType*> path;
		std::vector<NodeRecord> openList; //connections to be checked
		std::vector<NodeRecord> closedList; //connections already checked
		NodeRecord currentnode;

		NodeRecord startRecord = NodeRecord{ pStartNode , nullptr , 0.f , GetHeuristicCost(pStartNode , pGoalNode) };
		currentnode = startRecord;
		openList.push_back(startRecord);

		while (!openList.empty())
		{
			//lowest f cost
			currentnode = *std::min_element(openList.begin(), openList.end());
			
			//check if that node leads to end node
			if (currentnode.pConnection != nullptr && m_pGraph->GetNode(currentnode.pConnection->GetTo()) == pGoalNode)
			{
				break;
			}

			//loop over all connections
			for (auto con : m_pGraph->GetNodeConnections(currentnode.pNode->GetIndex()))
			{
				//calculate totalcostSofar
				float totalCostSofar{};
				totalCostSofar += currentnode.costSoFar + con->GetCost();

				//check if connections leads to one already in closedlist
				std::vector<NodeRecord>::iterator closedIt = std::find_if(closedList.begin(), closedList.end(), [con, this](NodeRecord first)
					{
						return  first.pNode == m_pGraph->GetNode(con->GetTo());
					});

				//check if connections leads to one already in openlist
				std::vector<NodeRecord>::iterator openIt = std::find_if(openList.begin(), openList.end(), [con, this](NodeRecord first)
					{
						return  first.pNode == m_pGraph->GetNode(con->GetTo());
					});

				if ( closedIt != closedList.end() )
				{
					//is deze connection niet goedkoper verwijder dan
					if (closedIt->costSoFar > totalCostSofar)
					{
						closedList.erase(closedIt);
					}
					continue;
				}
				else if(openIt != openList.end())
				{
					//is deze connection niet goedkoper verwijder dan
					if (openIt->costSoFar > totalCostSofar)
					{
						openList.erase(openIt);
					}
					continue;
				}

				//add new noderecord to openlist
				NodeRecord newNode = NodeRecord{ m_pGraph->GetNode( con->GetTo() ) , con , totalCostSofar , GetHeuristicCost(m_pGraph->GetNode(con->GetTo()) , pGoalNode) + totalCostSofar };
				openList.push_back(newNode);
			}
			openList.erase(std::remove(openList.begin(), openList.end(), currentnode), openList.end());

			closedList.push_back(currentnode);
		}

		//start backtracking

		while (currentnode.pNode != pStartNode)
		{
			path.push_back(currentnode.pNode);
			currentnode = *std::find_if(closedList.begin(), closedList.end(), [=](NodeRecord node) {
				return node.pNode == m_pGraph->GetNode(currentnode.pConnection->GetFrom());
				});
		}

		path.push_back(pStartNode);
		std::reverse(path.begin(), path.end());

		return path;
	}

	template <class T_NodeType, class T_ConnectionType>
	float Elite::FlowField<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
}