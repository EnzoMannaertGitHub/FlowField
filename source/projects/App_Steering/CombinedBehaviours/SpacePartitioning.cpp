#include "stdafx.h"
#include "SpacePartitioning.h"
#include "projects\App_Steering\SteeringAgent.h"

// --- Cell ---
// ------------
Cell::Cell(float left, float bottom, float width, float height)
{
	boundingBox.bottomLeft = { left, bottom };
	boundingBox.width = width;
	boundingBox.height = height;
}

std::vector<Elite::Vector2> Cell::GetRectPoints() const
{
	auto left = boundingBox.bottomLeft.x;
	auto bottom = boundingBox.bottomLeft.y;
	auto width = boundingBox.width;
	auto height = boundingBox.height;

	std::vector<Elite::Vector2> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(float width, float height, int rows, int cols, int maxEntities)
	: m_SpaceWidth(width)
	, m_SpaceHeight(height)
	, m_NrOfRows(rows)
	, m_NrOfCols(cols)
	, m_Neighbors(maxEntities)
	, m_NrOfNeighbors()
	, m_CellWidth{ m_SpaceWidth / m_NrOfCols }
	, m_CellHeight{ m_SpaceHeight / m_NrOfRows }
	, m_DebugRendeingOn{ false }
{
	for (int i{}; i < cols; i++)
	{
		for (int j{}; j < rows; j++)
		{
			m_Cells.push_back(Cell{ i * m_CellWidth , j * m_CellHeight , m_CellWidth , m_CellHeight });
		}
	}

	m_Neighbors.resize(maxEntities - 1);
}

void CellSpace::AddAgent(SteeringAgent* agent)
{
	m_Cells[PositionToIndex(agent->GetPosition())].agents.push_back(agent);
}

void CellSpace::UpdateAgentCell(SteeringAgent* agent, const Elite::Vector2& oldPos)
{

	int oldIndex{ PositionToIndex(oldPos) };
	int newIndex{ PositionToIndex(agent->GetPosition()) };

	if (oldIndex != newIndex)
	{
		m_Cells[oldIndex].agents.remove(agent);
		m_Cells[newIndex].agents.push_back(agent);
	}

}

void CellSpace::RegisterNeighbors(const SteeringAgent* pAgent, float queryRadius)
{
	Elite::Vector2 pos{ pAgent->GetPosition() };
	m_QueryRect = Elite::Rect{ Elite::Vector2{ pos.x - queryRadius , pos.y - queryRadius } , queryRadius * 2.f , queryRadius * 2.f };

	m_NrOfNeighbors = 0;
	//out of boundaries check
	if (m_QueryRect.bottomLeft.x < 0)
	{
		m_QueryRect.bottomLeft.x = 1;
	}
	if (m_QueryRect.bottomLeft.y <0)
	{
		m_QueryRect.bottomLeft.y = 1;
	}
	if ( (m_QueryRect.bottomLeft.x + m_QueryRect.width) > m_SpaceWidth )
	{
		m_QueryRect.bottomLeft.x = (m_SpaceWidth-1) - m_QueryRect.width;
	}
	if ((m_QueryRect.bottomLeft.y + m_QueryRect.height) > m_SpaceHeight)
	{
		m_QueryRect.bottomLeft.y = (m_SpaceHeight - 1) - m_QueryRect.height;
	}

	int bottomleftIndex = PositionToIndex(m_QueryRect.bottomLeft);
	int topRightIndex = PositionToIndex(Elite::Vector2{ m_QueryRect.bottomLeft.x + m_QueryRect.width , m_QueryRect.bottomLeft.y + m_QueryRect.height });
	int topLeftIndex = PositionToIndex(Elite::Vector2{ m_QueryRect.bottomLeft.x  , m_QueryRect.bottomLeft.y + m_QueryRect.height });

	if (bottomleftIndex < 0 || topRightIndex < 0)
	{
		std::cout << "index is negatief \n";
	}

	int height{ topLeftIndex - bottomleftIndex };
	int heightCount{};
	for (int i{ bottomleftIndex }; i <= topRightIndex; i++)
	{

		//register neighbours
		for (SteeringAgent* pAgent2 : m_Cells[i].agents)
		{
			float distance{};

			if (pAgent != pAgent2)
			{
				distance = Distance(pos, pAgent2->GetPosition());
				if (distance <= queryRadius)
				{
					//Debug render
					if (m_DebugRendeingOn)
					{
					pAgent2->SetBodyColor({ 1 , 0 , 1 });
					}
					m_Neighbors[m_NrOfNeighbors] = pAgent2;
					m_NrOfNeighbors++;
				}
				else
				{
					//Debug render
						pAgent2->SetBodyColor({ 1 , 1 , 0 });
				}
			}
		}

		//Debug render
		if (m_DebugRendeingOn)
		{
			Elite::Polygon cell{ m_Cells[i].GetRectPoints() };
			DEBUGRENDERER2D->DrawPolygon(&cell, { 1,0,1 }, 0.3f);
		}
		if (heightCount == height || height == 0)
		{
			bottomleftIndex += m_NrOfCols;
			i = bottomleftIndex - 1;
			heightCount = 0;
		}
		else
		{
			heightCount++;
		}
	}
}

void CellSpace::RenderCells() const
{
	if (m_DebugRendeingOn == true)
	{
		for (Cell pCell : m_Cells)
		{
			DEBUGRENDERER2D->DrawSegment(pCell.GetRectPoints()[0], pCell.GetRectPoints()[1], { 1,0,0 });
			DEBUGRENDERER2D->DrawSegment(pCell.GetRectPoints()[1], pCell.GetRectPoints()[2], { 1,0,0 });
			DEBUGRENDERER2D->DrawSegment(pCell.GetRectPoints()[2], pCell.GetRectPoints()[3], { 1,0,0 });
			DEBUGRENDERER2D->DrawSegment(pCell.GetRectPoints()[3], pCell.GetRectPoints()[0], { 1,0,0 });

			Elite::Polygon cell{ pCell.GetRectPoints() };

			DEBUGRENDERER2D->DrawString(cell.GetCenterPoint(), std::to_string(pCell.agents.size()).c_str());

			DEBUGRENDERER2D->DrawSegment(m_QueryRect.bottomLeft, Elite::Vector2{ m_QueryRect.bottomLeft.x + m_QueryRect.width , m_QueryRect.bottomLeft.y }, { 0,0,1 });
			DEBUGRENDERER2D->DrawSegment(m_QueryRect.bottomLeft, Elite::Vector2{ m_QueryRect.bottomLeft.x , m_QueryRect.bottomLeft.y + m_QueryRect.height }, { 0,0,1 });

			DEBUGRENDERER2D->DrawSegment(Elite::Vector2{ m_QueryRect.bottomLeft.x + m_QueryRect.width , m_QueryRect.bottomLeft.y }, Elite::Vector2{ m_QueryRect.bottomLeft.x + m_QueryRect.width , m_QueryRect.bottomLeft.y + m_QueryRect.height }, { 0,0,1 });
			DEBUGRENDERER2D->DrawSegment(Elite::Vector2{ m_QueryRect.bottomLeft.x + m_QueryRect.width, m_QueryRect.bottomLeft.y + m_QueryRect.height }, Elite::Vector2{ m_QueryRect.bottomLeft.x , m_QueryRect.bottomLeft.y + m_QueryRect.height }, { 0,0,1 });

		}

		/*	for (Cell pCell : overlappingCells)
			{
				DEBUGRENDERER2D->DrawSegment(Elite::Vector2{ pCell.boundingBox.bottomLeft.x , pCell.boundingBox.bottomLeft.y },
					Elite::Vector2{ pCell.boundingBox.bottomLeft.x + pCell.boundingBox.width, pCell.boundingBox.bottomLeft.y }, { 1,0,1 });

				DEBUGRENDERER2D->DrawSegment(Elite::Vector2{ pCell.boundingBox.bottomLeft.x , pCell.boundingBox.bottomLeft.y },
					Elite::Vector2{ pCell.boundingBox.bottomLeft.x , pCell.boundingBox.bottomLeft.y + pCell.boundingBox.height }, { 1,0,1 });

				DEBUGRENDERER2D->DrawSegment(Elite::Vector2{ pCell.boundingBox.bottomLeft.x + pCell.boundingBox.width , pCell.boundingBox.bottomLeft.y },
					Elite::Vector2{ pCell.boundingBox.bottomLeft.x + pCell.boundingBox.width , pCell.boundingBox.bottomLeft.y + pCell.boundingBox.height }, { 1,0,1 });

				DEBUGRENDERER2D->DrawSegment(Elite::Vector2{ pCell.boundingBox.bottomLeft.x + pCell.boundingBox.width , pCell.boundingBox.bottomLeft.y + pCell.boundingBox.height },
					Elite::Vector2{ pCell.boundingBox.bottomLeft.x , pCell.boundingBox.bottomLeft.y + pCell.boundingBox.height }, { 1,0,1 });


			}*/
	}
}

int CellSpace::PositionToIndex(const Elite::Vector2 pos) const
{
	int col = int(pos.x / m_CellWidth) % m_NrOfCols;
	int row = int(pos.y / m_CellHeight) % m_NrOfRows;

	return row + (m_NrOfCols * col);
}

void CellSpace::SetDebugRendering(bool canRender)
{
	m_DebugRendeingOn = canRender;
}
