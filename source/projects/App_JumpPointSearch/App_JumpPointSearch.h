#pragma once

#include "framework/EliteInterfaces/EIApp.h"
#include "framework/EliteAI/EliteGraphs/EGridGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphEditor.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphRenderer.h"

class App_JumpPointSearch : public IApp
{
public:

	App_JumpPointSearch() = default;
	virtual ~App_JumpPointSearch();

	void Start() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) const override;

private:

	// Data Members

	// Grid data members
	Elite::GridGraph<Elite::GridTerrainNode, Elite::GraphConnection>* m_pGridGraph;
	Elite::EGraphEditor m_GraphEditor;
	Elite::EGraphRenderer m_GraphRenderer;
	static constexpr int m_Columns{50};
	static constexpr int m_Rows{50};
	static constexpr int m_CellSize{15};

	bool m_IsGridDrawn{ true };
	bool m_IsNodeNumbersDrawn{};
	bool m_IsConnectionsDrawn{};
	bool m_IsConnectionCostsDrawn{};

	// PathFinding data members
	int startPathIdx{};
	int endPathIdx{};
	
	// Interface
	bool m_IsStartNodeSelected{};

	
	bool m_ShouldUpdatePath{};
	
	void UpdateImGui();
};

