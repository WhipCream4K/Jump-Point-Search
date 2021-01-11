#ifndef ASTAR_APPLICATION_H
#define ASTAR_APPLICATION_H
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteInterfaces/EIApp.h"
#include "framework\EliteAI\EliteGraphs\EGridGraph.h"
//#include "framework/EliteAI/EliteGraphs/EliteGraphAlgorithms/EAStar.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphEditor.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphRenderer.h"
#include "projects/Helpers.h"


//-----------------------------------------------------------------
// Application
//-----------------------------------------------------------------
class App_PathfindingAStar final : public IApp
{
public:
	//Constructor & Destructor
	App_PathfindingAStar() = default;
	virtual ~App_PathfindingAStar();

	//App Functions
	void Start() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) const override;

private:
	//Datamembers
	const bool ALLOW_DIAGONAL_MOVEMENT = true;
	Elite::Vector2 m_StartPosition = Elite::ZeroVector2;
	Elite::Vector2 m_TargetPosition = Elite::ZeroVector2;

	//Grid datamembers
	static const int COLUMNS = 50;
	static const int ROWS = 50;
	unsigned int m_SizeCell = 15;
	Elite::GridGraph<Elite::GridTerrainNode, Elite::GraphConnection>* m_pGridGraph;


	//Pathfinding datamembers
	int startPathIdx = invalid_node_index;
	int endPathIdx = invalid_node_index;
	std::vector<Elite::GridTerrainNode*> m_vPath;
	bool m_UpdatePath = true;

	//Editor and Visualisation
	Elite::EGraphEditor m_GraphEditor{};
	Elite::EGraphRenderer m_GraphRenderer{};

	//Debug rendering information
	struct GradientColor
	{
		Elite::Color left{};
		Elite::Color right{};
	};

	GradientColor m_GCost;
	GradientColor m_FCost;
	
	bool m_bDrawGrid = true;
	bool m_bDrawNodeNumbers = false;
	bool m_bDrawConnections = false;
	bool m_bDrawConnectionsCosts = false;
	bool m_StartSelected = true;
	bool m_ShowVisitedNode = true;
	bool m_ShowAbsoluteValue = false;
	bool m_ShowDebugOptions = true;
	int m_SelectedHeuristic = 4;
	int m_SelectedShownGradient = 0;
	Elite::Heuristic m_pHeuristicFunction = Elite::HeuristicFunctions::Chebyshev;
	std::vector<NodeForRender> m_VisitedNode;


	//Functions
	void MakeGridGraph();
	void UpdateImGui();
	Elite::Color GetGradientColor(float value, const Elite::Color& c1, const Elite::Color& c2) const;

	//C++ make the class non-copyable
	App_PathfindingAStar(const App_PathfindingAStar&) = delete;
	App_PathfindingAStar& operator=(const App_PathfindingAStar&) = delete;
};
#endif