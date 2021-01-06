//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_GraphTheory.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EEularianPath.h"

using namespace Elite;

//Destructor
App_GraphTheory::~App_GraphTheory()
{
	SAFE_DELETE(m_pGraph2d);
}

//Functions
void App_GraphTheory::Start()
{
	//Initialization of your application. If you want access to the physics world you will need to store it yourself.
	//----------- CAMERA ------------
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(80.f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(0, 0));
	DEBUGRENDERER2D->GetActiveCamera()->SetMoveLocked(false);
	DEBUGRENDERER2D->GetActiveCamera()->SetZoomLocked(false);

	m_pGraph2d = new Graph2D<GraphNode2D, GraphConnection2D>(false);
	m_pGraph2d->AddNode(new GraphNode2D(0, { 20 , 30 } ));
	m_pGraph2d->AddNode(new GraphNode2D(1, { -10 , -10 }));
	m_pGraph2d->AddConnection(new GraphConnection2D(0, 1));
}

void App_GraphTheory::Update(float deltaTime)
{
	m_pGraph2d->Update();
	m_pGraph2d->SetConnectionCostsToDistance();

	auto eulerFinder{ EulerianPath<GraphNode2D , GraphConnection2D>(m_pGraph2d) };
	auto eulerianity{ eulerFinder.IsEulerian() };
	auto eulerianPath{ eulerFinder.FindPath(eulerianity) };
	Elite::Color color{1,1,1};
	int nrOfNodes{m_pGraph2d->GetNrOfNodes()};

	switch (eulerianity)
	{
	case Elite::Eulerianity::notEulerian:
		color = { 0,0,1 };
		for (auto node : m_pGraph2d->GetAllNodes())
		{
			node->SetColor(color);
		}
		cout << "not eulerian \n";
		break;
	case Elite::Eulerianity::semiEulerian:
		for (auto node : eulerianPath)
		{
			std::cout << node->GetIndex() << ' ';
			node->SetColor(color);
			color.r -= color.r / nrOfNodes;
			color.g -= color.g / nrOfNodes;
			color.b -= color.b / nrOfNodes;
		}
		cout << "semi eulerian \n";
		break;
	case Elite::Eulerianity::eulerian:
		for (auto node : eulerianPath)
		{
			std::cout << node->GetIndex() << ' ';
			node->SetColor(color);
			color.r -= color.r / nrOfNodes;
			color.g -= color.g / nrOfNodes;
			color.b -= color.b / nrOfNodes;
		}
		cout << "eulerian \n";
		break;
	default:
		break;
	}
	std::cout << '\n';
	//------- UI --------
#ifdef PLATFORM_WINDOWS
#pragma region UI
	{
		//Setup
		int menuWidth = 150;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 90));
		ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::PushAllowKeyboardFocus(false);
		ImGui::SetWindowFocus();
		ImGui::PushItemWidth(70);
		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
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
		ImGui::Spacing();

		ImGui::Text("Graph Theory");
		ImGui::Spacing();
		ImGui::Spacing();

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
#pragma endregion
#endif
	

}

void App_GraphTheory::Render(float deltaTime) const
{
	m_GrpahRenderer.RenderGraph(m_pGraph2d , true , true);
}
