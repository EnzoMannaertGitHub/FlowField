#pragma once
#include <stack>

namespace Elite
{
	enum class Eulerianity
	{
		notEulerian,
		semiEulerian,
		eulerian,
	};

	template <class T_NodeType, class T_ConnectionType>
	class EulerianPath
	{
	public:

		EulerianPath(IGraph<T_NodeType, T_ConnectionType>* pGraph);

		Eulerianity IsEulerian() const;
		vector<T_NodeType*> FindPath(Eulerianity& eulerianity) const;

	private:
		void VisitAllNodesDFS(int startIdx, vector<bool>& visited) const;
		bool IsConnected() const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
	};

	template<class T_NodeType, class T_ConnectionType>
	inline EulerianPath<T_NodeType, T_ConnectionType>::EulerianPath(IGraph<T_NodeType, T_ConnectionType>* pGraph)
		: m_pGraph(pGraph)
	{
	}

	template<class T_NodeType, class T_ConnectionType>
	inline Eulerianity EulerianPath<T_NodeType, T_ConnectionType>::IsEulerian() const
	{
		// If the graph is not connected, there can be no Eulerian Trail
		if (IsConnected() == false)
			return Eulerianity::notEulerian;

		// Count nodes with odd degree 
		int nrOfNodes{m_pGraph->GetNrOfNodes()};
		int oddCount{ 0 };

		for (int i{} ; i < nrOfNodes ; i++)
		{
			if (m_pGraph->IsNodeValid(i) && (m_pGraph->GetNodeConnections(i).size() & 1) ) //& 1 => check of even is
			{
				oddCount++;
			}
		}

		// A connected graph with more than 2 nodes with an odd degree (an odd amount of connections) is not Eulerian
		if (oddCount > 2)
		{
			return Eulerianity::notEulerian;
		}
		// A connected graph with exactly 2 nodes with an odd degree is Semi-Eulerian (an Euler trail can be made, but only starting and ending in these 2 nodes)
		else if (oddCount == 2 && nrOfNodes != 2)
			return Eulerianity::semiEulerian;
		// A connected graph with no odd nodes is Eulerian
		else
		{
			return Eulerianity::eulerian;
		}

	}

	template<class T_NodeType, class T_ConnectionType>
	inline vector<T_NodeType*> EulerianPath<T_NodeType, T_ConnectionType>::FindPath(Eulerianity& eulerianity) const
	{
		// Get a copy of the graph because this algorithm involves removing edges
		auto graphCopy = m_pGraph->Clone();
		int nrOfNodes = graphCopy->GetNrOfNodes();
		auto path = vector<T_NodeType*>();
		auto currentNode{0};
		std::vector<int> stack{};

		if (eulerianity == Eulerianity::eulerian)
		{
			for (int i{}; i < nrOfNodes; i++)
			{
				if (m_pGraph->IsNodeValid(i))
				{
					currentNode = i;
				}
			}
		}
		else if (eulerianity == Eulerianity::semiEulerian)
		{
			for (int i{} ; i < nrOfNodes ; i++)
			{
				if (m_pGraph->IsNodeValid(i))
				{
					if (m_pGraph->GetNodeConnections(i).size() & 1)
					{
						currentNode = i;
						break;
					}
				}
			}
		}
		else
		{
			return path;
		}


		// algorithm...
		do
		{
			for (int i{}; i <= nrOfNodes; i++)
			{
				if (m_pGraph->IsNodeValid(i))
				{
					if (graphCopy->GetConnection(currentNode, i) != nullptr)
					{
						stack.push_back(currentNode);
						graphCopy->RemoveConnection(currentNode, i);
						currentNode = i;
					}
					else
					{
						if (stack.size() > 0)
						{
							path.push_back(m_pGraph->GetNode(currentNode));
							currentNode = stack.back();
							stack.pop_back();
						}
					}
				}
			}
		} while (stack.size() != 0); 
	
			path.push_back(m_pGraph->GetNode(currentNode));
		return path;	
	}

	template<class T_NodeType, class T_ConnectionType>
	inline void EulerianPath<T_NodeType, T_ConnectionType>::VisitAllNodesDFS(int startIdx, vector<bool>& visited) const
	{
		// mark the visited node
		visited[startIdx] = true;
		// recursively visit any valid connected nodes that were not visited before
		for (T_ConnectionType* connection : m_pGraph->GetNodeConnections(startIdx))
		{
			if (m_pGraph->IsNodeValid(connection->GetTo()) && !visited[connection->GetTo()])
			{
				VisitAllNodesDFS(connection->GetTo() , visited);
			}
		}
	}

	template<class T_NodeType, class T_ConnectionType>
	inline bool EulerianPath<T_NodeType, T_ConnectionType>::IsConnected() const
	{
		// find a valid starting node that has connections
		int nrOfnodes{m_pGraph->GetNrOfNodes()};
		std::vector<bool> visited(nrOfnodes, false);

		int connectedIndex{ invalid_node_index };
		for (int i{}; i < nrOfnodes; i++)
		{
			if (m_pGraph->IsNodeValid(i) && m_pGraph->GetNodeConnections(i).size() != 0)
			{
				connectedIndex = i;
				break;
			}
		}
		// if no valid node could be found, return false
		if (connectedIndex == invalid_node_index)
			return false;

		// start a depth-first-search traversal from a node that has connections
		VisitAllNodesDFS(connectedIndex, visited);

		// if a node was never visited, this graph is not connected
		for (int i{} ; i < nrOfnodes ; i++)
		{
			if (m_pGraph->IsNodeValid(i) && visited[i] == false)
				return false;
		}
		return true;
	}

}