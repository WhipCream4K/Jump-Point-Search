#include "stdafx.h"
#include "SpacePartitioning.h"
#include "projects\App_Steering\SteeringAgent.h"

// --- Cell ---
// ------------
Cell::Cell(float left, float bottom, float width, float height)
{
	boundingBox.bottomLeft = Elite::Vector2{ left, bottom };
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
	: m_Cells(rows * cols)
	, m_SpaceWidth(width)
	, m_SpaceHeight(height)
	, m_NrOfRows(rows)
	, m_NrOfCols(cols)
	, m_Neighbors(maxEntities)
	, m_NrOfNeighbors(0)
{
	m_CellHeight = height * 2.0f / float(rows);
	m_CellWidth = width * 2.0f / float(cols);

	// Register cells bounding boxes
	Elite::Vector2 bottomLeft{ -width,height - m_CellHeight };

	for (int y = 0; y < rows; ++y)
	{
		for (int x = 0; x < cols; ++x)
		{
			m_Cells[x + (y * cols)].boundingBox = { bottomLeft,m_CellWidth,m_CellHeight };
			bottomLeft.x += m_CellWidth;
		}

		bottomLeft.y -= m_CellHeight;
		bottomLeft.x = -width;
	}
}

void CellSpace::AddAgent(SteeringAgent* agent)
{
	const auto agentPos{ agent->GetPosition() };
	const int index{ PositionToIndex(agentPos) };
	if(index > 0)
	{
		auto& cell{ m_Cells[index].agents };
		const auto it = std::find(cell.begin(), cell.end(), agent);
		if (it == cell.end())
			cell.emplace_back(agent);
	}
}

void CellSpace::UpdateAgentCell(SteeringAgent* agent, const Elite::Vector2& oldPos)
{
	const int oldCellIndex{ agent->GetOldCellIndex() };
	const auto currentPos{ agent->GetPosition() };
	const int currentCellIndex{ PositionToIndex(currentPos)};

	if(oldCellIndex >= 0 && currentCellIndex >= 0)
	{
		if (oldCellIndex != currentCellIndex)
		{
			// Delete the agent from the old cell and add in to new one
			m_Cells[currentCellIndex].agents.emplace_back(agent);
			m_Cells[oldCellIndex].agents.remove(agent);
		}

		//ImGui::Value("OldPosX", oldPos.x);
		//ImGui::Value("OldPosY", oldPos.y);

		//ImGui::Value("Current PosX", currentPos.x);
		//ImGui::Value("Current PosY", currentPos.y);
		//
		//ImGui::Value("Old Index: ", oldCellIndex);
		//ImGui::Value("Current Index: ", currentCellIndex);

		agent->SetOldCellIndex(currentCellIndex);
	}


}

void CellSpace::RegisterNeighbors(SteeringAgent* pAgent, const Elite::Vector2& targetPos, float queryRadius)
{
	m_NrOfNeighbors = 0;
	const auto currentCellIndex{ PositionToIndex(targetPos) };

	// Just that one condition where the query radius is inside one cell
	if (queryRadius <= m_CellWidth)
	{
		const auto currentAgentsInCell{ m_Cells[currentCellIndex].agents };

		for (const auto& agent : currentAgentsInCell)
		{
			if(pAgent != agent)
				m_Neighbors[m_NrOfNeighbors++] = agent;
		}

		return;
	}

	const auto maxSurroundCellIndex{ abs(int(floorf((queryRadius / m_CellWidth)))) };
	const auto maxCells{ (m_NrOfCols * m_NrOfRows) - 1 };

	for (int y = -maxSurroundCellIndex; y <= maxSurroundCellIndex; ++y)
	{
		for (int x = -maxSurroundCellIndex; x <= maxSurroundCellIndex; ++x)
		{
			const int traverseIndex{ (currentCellIndex + x) + (m_NrOfCols * y) };
			if (traverseIndex >= 0 && traverseIndex < maxCells)
			{
				const auto& agentsInCell{ m_Cells[traverseIndex].agents };
				if(agentsInCell.empty()) continue;
				
				for (const auto& agent : agentsInCell)
				{
					if(agent != pAgent)
						m_Neighbors[m_NrOfNeighbors++] = agent;
				}
			}
		}
	}
}

void CellSpace::RenderCells() const
{
	const auto debugRenderer{ DEBUGRENDERER2D };
	for (const auto& cell : m_Cells)
	{
		const auto rectPoints{ cell.GetRectPoints() };
		debugRenderer->DrawPolygon(
			rectPoints.data(),
			int(rectPoints.size()),
			{ 1.0f,0.0f,0.0f },
			0.4f);

		const auto agentCount{ std::to_string(cell.agents.size()) };
		const Elite::Vector2 topRight{ rectPoints[1] };
		debugRenderer->DrawString(topRight, agentCount.c_str());
	}
}

void CellSpace::RenderNeighbourHoodCells(SteeringAgent* pAgent, float queryRadius)
{
	const int currentCellIndex{ PositionToIndex(pAgent->GetPosition()) };
	
	const auto maxSurroundCellIndex{ abs(int(floorf((queryRadius / m_CellWidth)))) };
	const auto maxCells{ (m_NrOfCols * m_NrOfRows) - 1 };

	for (int y = -maxSurroundCellIndex; y <= maxSurroundCellIndex; ++y)
	{
		for (int x = -maxSurroundCellIndex; x <= maxSurroundCellIndex; ++x)
		{
			const int traverseIndex{ (currentCellIndex + x) + (m_NrOfCols * y) };
			if (traverseIndex >= 0 && traverseIndex < maxCells)
			{
				DEBUGRENDERER2D->DrawSolidPolygon(
					m_Cells[traverseIndex].GetRectPoints().data(),
					4,
					{ 0.92f,0.94f,0.56f,0.5f },
					0.4f);
			}
		}
	}
}

int CellSpace::PositionToIndex(const Elite::Vector2 pos) const
{
	int index{};
	
	const int currCellCol{ int(((m_SpaceWidth) + pos.x) / m_CellWidth) };
	const int currCellRow{ int(((m_SpaceHeight) - pos.y) / m_CellHeight) };

	if (currCellRow > m_NrOfRows - 1 || currCellCol > m_NrOfCols - 1)
		index = -1;
	else
		index = currCellCol + (m_NrOfCols * currCellRow);

	return index;
}
