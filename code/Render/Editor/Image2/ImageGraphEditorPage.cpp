#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Editor/IDocument.h"
#include "Editor/IEditorPageSite.h"
#include "Render/Editor/Edge.h"
#include "Render/Editor/Image2/IImgStep.h"
#include "Render/Editor/Image2/ImageGraphAsset.h"
#include "Render/Editor/Image2/ImageGraphEditorPage.h"
#include "Render/Editor/Image2/ImgInput.h"
#include "Render/Editor/Image2/ImgOutput.h"
#include "Render/Editor/Image2/ImgPass.h"
#include "Render/Editor/Image2/ImgTargetSet.h"
#include "Ui/Application.h"
#include "Ui/Container.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/TableLayout.h"
#include "Ui/Graph/DefaultNodeShape.h"
#include "Ui/Graph/Edge.h"
#include "Ui/Graph/EdgeConnectEvent.h"
#include "Ui/Graph/EdgeDisconnectEvent.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/InputNodeShape.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/NodeMovedEvent.h"
#include "Ui/Graph/OutputNodeShape.h"
#include "Ui/Graph/Pin.h"
#include "Ui/Graph/SelectEvent.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImageGraphEditorPage", ImageGraphEditorPage, editor::IEditorPage)

ImageGraphEditorPage::ImageGraphEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document)
:	m_editor(editor)
,	m_site(site)
,	m_document(document)
{
}

bool ImageGraphEditorPage::create(ui::Container* parent)
{
	m_imageGraph = m_document->getObject< ImageGraphAsset >(0);
	if (!m_imageGraph)
		return false;

	Ref< ui::Container > container = new ui::Container();
	container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%", 0, 0));

	m_editorGraph = new ui::GraphControl();
	m_editorGraph->create(container);
	m_editorGraph->addEventHandler< ui::MouseButtonDownEvent >(this, &ImageGraphEditorPage::eventButtonDown);
	m_editorGraph->addEventHandler< ui::SelectEvent >(this, &ImageGraphEditorPage::eventSelect);
	m_editorGraph->addEventHandler< ui::NodeMovedEvent >(this, &ImageGraphEditorPage::eventNodeMoved);
	m_editorGraph->addEventHandler< ui::EdgeConnectEvent >(this, &ImageGraphEditorPage::eventEdgeConnect);
	m_editorGraph->addEventHandler< ui::EdgeDisconnectEvent >(this, &ImageGraphEditorPage::eventEdgeDisconnect);

	m_menuPopup = new ui::Menu();
	Ref< ui::MenuItem > menuItemCreate = new ui::MenuItem(L"Create...");
	menuItemCreate->add(new ui::MenuItem(ui::Command(L"ImageGraph.Editor.AddInput"), L"Input"));
	menuItemCreate->add(new ui::MenuItem(ui::Command(L"ImageGraph.Editor.AddPass"), L"Pass"));
	menuItemCreate->add(new ui::MenuItem(ui::Command(L"ImageGraph.Editor.AddTarget"), L"Target"));
	m_menuPopup->add(menuItemCreate);
	m_menuPopup->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), L"Delete"));

	createEditorGraph();
	return true;
}

void ImageGraphEditorPage::destroy()
{
	safeDestroy(m_editorGraph);
}

bool ImageGraphEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return false;
}

bool ImageGraphEditorPage::handleCommand(const ui::Command& command)
{
	if (command == L"Editor.PropertiesChanging")
	{
		m_document->push();
	}
	else if (command == L"Editor.PropertiesChanged")
	{
		// refreshGraph();
		// updateGraph();
	}
	else if (command == L"Editor.Cut" || command == L"Editor.Copy")
	{
		// RefArray< ui::Node > selectedNodes;
		// if (m_editorGraph->getSelectedNodes(selectedNodes) > 0)
		// {
		// 	// Also copy edges which are affected by selected nodes.
		// 	RefArray< ui::Edge > selectedEdges;
		// 	m_editorGraph->getConnectedEdges(selectedNodes, true, selectedEdges);

		// 	Ref< ImageGraphEditorClipboardData > data = new ImageGraphEditorClipboardData();

		// 	ui::Rect bounds(0, 0, 0, 0);
		// 	for (auto i = selectedNodes.begin(); i != selectedNodes.end(); ++i)
		// 	{
		// 		Ref< Node > shaderNode = (*i)->getData< Node >(L"SHADERNODE");
		// 		T_ASSERT(shaderNode);
		// 		data->addNode(shaderNode);

		// 		if (i != selectedNodes.begin())
		// 		{
		// 			ui::Rect rc = (*i)->calculateRect();
		// 			bounds.left = std::min(bounds.left, rc.left);
		// 			bounds.top = std::min(bounds.top, rc.top);
		// 			bounds.right = std::max(bounds.right, rc.right);
		// 			bounds.bottom = std::max(bounds.bottom, rc.bottom);
		// 		}
		// 		else
		// 			bounds = (*i)->calculateRect();
		// 	}

		// 	data->setBounds(bounds);

		// 	for (auto selectedEdge : selectedEdges)
		// 	{
		// 		Ref< Edge > edge = selectedEdge->getData< Edge >(L"SHADEREDGE");
		// 		T_ASSERT(edge);
		// 		data->addEdge(edge);
		// 	}

		// 	ui::Application::getInstance()->getClipboard()->setObject(data);

		// 	// Remove edges and nodes from graphs if user cuts.
		// 	if (command == L"Editor.Cut")
		// 	{
		// 		// Save undo state.
		// 		m_document->push();

		// 		// Remove edges which are connected to any selected node, not only those who connects to both selected end nodes.
		// 		selectedEdges.resize(0);
		// 		m_editorGraph->getConnectedEdges(selectedNodes, false, selectedEdges);

		// 		for (auto selectedEdge : selectedEdges)
		// 		{
		// 			m_imageGraph->removeEdge(selectedEdge->getData< Edge >(L"SHADEREDGE"));
		// 			m_editorGraph->removeEdge(selectedEdge);
		// 		}

		// 		for (auto selectedNode : selectedNodes)
		// 		{
		// 			m_imageGraph->removeNode(selectedNode->getData< Node >(L"SHADERNODE"));
		// 			m_editorGraph->removeNode(selectedNode);
		// 		}
		// 	}
		// }
	}
	else if (command == L"Editor.Paste")
	{
		// Ref< ImageGraphEditorClipboardData > data = dynamic_type_cast< ImageGraphEditorClipboardData* >(
		// 	ui::Application::getInstance()->getClipboard()->getObject()
		// );
		// if (data)
		// {
		// 	// Save undo state.
		// 	m_document->push();

		// 	const ui::Rect& bounds = data->getBounds();

		// 	ui::Rect rcClient = m_editorGraph->getInnerRect();
		// 	ui::Point center = m_editorGraph->clientToVirtual(rcClient.getCenter());

		// 	for (RefArray< Node >::const_iterator i = data->getNodes().begin(); i != data->getNodes().end(); ++i)
		// 	{
		// 		// Create new unique instance ID.
		// 		(*i)->setId(Guid::create());

		// 		// Place node in view.
		// 		std::pair< int, int > position = (*i)->getPosition();
		// 		position.first = ui::invdpi96(center.x + ui::dpi96(position.first) - bounds.left - bounds.getWidth() / 2);
		// 		position.second = ui::invdpi96(center.y + ui::dpi96(position.second) - bounds.top - bounds.getHeight() / 2);
		// 		(*i)->setPosition(position);

		// 		// Add node to graph.
		// 		m_imageGraph->addNode(*i);
		// 	}

		// 	for (RefArray< Edge >::const_iterator i = data->getEdges().begin(); i != data->getEdges().end(); ++i)
		// 		m_imageGraph->addEdge(*i);

		// 	createEditorNodes(
		// 		data->getNodes(),
		// 		data->getEdges()
		// 	);
		// 	updateGraph();
		// }
	}
	else if (command == L"Editor.SelectAll")
	{
		m_editorGraph->selectAllNodes();
		// updateGraph();
	}
	else if (command == L"Editor.Delete")
	{
		// RefArray< ui::Node > nodes;
		// if (m_editorGraph->getSelectedNodes(nodes) <= 0)
		// 	return false;

		// // Save undo state.
		// m_document->push();

		// // Remove edges first which are connected to selected nodes.
		// RefArray< ui::Edge > edges;
		// m_editorGraph->getConnectedEdges(nodes, false, edges);

		// for (RefArray< ui::Edge >::iterator i = edges.begin(); i != edges.end(); ++i)
		// {
		// 	ui::Edge* editorEdge = *i;
		// 	Ref< Edge > edge = editorEdge->getData< Edge >(L"SHADEREDGE");

		// 	m_editorGraph->removeEdge(editorEdge);
		// 	m_imageGraph->removeEdge(edge);
		// }

		// for (RefArray< ui::Node >::iterator i = nodes.begin(); i != nodes.end(); ++i)
		// {
		// 	ui::Node* editorNode = *i;
		// 	Ref< Node > shaderNode = editorNode->getData< Node >(L"SHADERNODE");

		// 	m_editorGraph->removeNode(editorNode);
		// 	m_imageGraph->removeNode(shaderNode);
		// }

		// updateGraph();
	}
	else if (command == L"Editor.Undo")
	{
		if (m_document->undo())
		{
			m_imageGraph = m_document->getObject< ImageGraphAsset >(0);
			T_ASSERT(m_imageGraph);

			// createEditorGraph();
		}
	}
	else if (command == L"Editor.Redo")
	{
		if (m_document->redo())
		{
			m_imageGraph = m_document->getObject< ImageGraphAsset >(0);
			T_ASSERT(m_imageGraph);

			// createEditorGraph();
		}
	}
	else if (command == L"ImageGraph.Editor.Center")
	{
		m_editorGraph->center();
	}
	else if (command == L"ImageGraph.Editor.AlignLeft")
	{
		m_document->push();
		m_editorGraph->alignNodes(ui::GraphControl::AnLeft);
	}
	else if (command == L"ImageGraph.Editor.AlignRight")
	{
		m_document->push();
		m_editorGraph->alignNodes(ui::GraphControl::AnRight);
	}
	else if (command == L"ImageGraph.Editor.AlignTop")
	{
		m_document->push();
		m_editorGraph->alignNodes(ui::GraphControl::AnTop);
	}
	else if (command == L"ImageGraph.Editor.AlignBottom")
	{
		m_document->push();
		m_editorGraph->alignNodes(ui::GraphControl::AnBottom);
	}
	else if (command == L"ImageGraph.Editor.EvenSpaceVertically")
	{
		m_document->push();
		m_editorGraph->evenSpace(ui::GraphControl::EsVertically);
	}
	else if (command == L"ImageGraph.Editor.EvenSpaceHorizontally")
	{
		m_document->push();
		m_editorGraph->evenSpace(ui::GraphControl::EsHorizontally);
	}
	else if (command == L"ImageGraph.Editor.AddInput")
	{
		// Create image graph input.
		Ref< ImgInput > input = new ImgInput();
		m_imageGraph->addNode(input);

		// Create node in graph control.
		m_editorGraph->addNode(createEditorNode(input));
	}
	else if (command == L"ImageGraph.Editor.AddPass")
	{
		// Create image graph pass.
		Ref< ImgPass > pass = new ImgPass();
		m_imageGraph->addNode(pass);

		// Create node in graph control.
		m_editorGraph->addNode(createEditorNode(pass));
	}
	else if (command == L"ImageGraph.Editor.AddTarget")
	{
		// Create image graph target set.
		Ref< ImgTargetSet > targetSet = new ImgTargetSet();
		m_imageGraph->addNode(targetSet);

		// Create node in graph control.
		m_editorGraph->addNode(createEditorNode(targetSet));
	}
	else
		return false;

	m_editorGraph->update();
	return true;
}

void ImageGraphEditorPage::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

Ref< ui::Node > ImageGraphEditorPage::createEditorNode(Node* node) const
{
	Ref< ui::Node > editorNode;

	const std::pair< int, int >& p = node->getPosition();
	const ui::Point position(ui::dpi96(p.first), ui::dpi96(p.second));

	if (auto input = dynamic_type_cast< ImgInput* >(node))
	{
		editorNode = new ui::Node(
			L"Input",
			input->getTargetSetId(),
			position,
			new ui::InputNodeShape(m_editorGraph)
		);
		editorNode->createOutputPin(L"Output");
	}
	else if (auto output = dynamic_type_cast< ImgOutput* >(node))
	{
		editorNode = new ui::Node(
			L"Output",
			L"",
			position,
			new ui::OutputNodeShape(m_editorGraph)
		);
		editorNode->createInputPin(L"Input", false);
	}
	else if (auto pass = dynamic_type_cast< ImgPass* >(node))
	{
		editorNode = new ui::Node(
			L"Pass",
			L"",
			position,
			new ui::DefaultNodeShape(m_editorGraph, ui::DefaultNodeShape::StDefault)
		);

		std::set< std::wstring > inputs;
		for (const auto step : pass->getSteps())
			step->getInputs(inputs);
		for (const auto& input : inputs)
			editorNode->createInputPin(input, false);

		editorNode->createOutputPin(L"Output");
	}
	else if (auto targetSet = dynamic_type_cast< ImgTargetSet* >(node))
	{
		editorNode = new ui::Node(
			L"TargetSet",
			targetSet->getTargetSetDesc().id,
			position,
			new ui::DefaultNodeShape(m_editorGraph, ui::DefaultNodeShape::StDefault)
		);
		editorNode->createInputPin(L"Input", false);
		editorNode->createOutputPin(L"Output");
	}
	else
	{
		T_FATAL_ERROR;
		return nullptr;
	}

	editorNode->setData(L"IMGNODE", node);
	return editorNode;
}

void ImageGraphEditorPage::createEditorGraph()
{
	// Keep a map from image graph nodes to editor nodes.
	std::map< Ref< Node >, Ref< ui::Node > > nodeMap;

	// Create editor nodes for each image graph node.
	for (auto node : m_imageGraph->getNodes())
	{
		Ref< ui::Node > editorNode = createEditorNode(node);
		m_editorGraph->addNode(editorNode);
		nodeMap[node] = editorNode;
	}

	// Create editor edges for each shader edge.
	for (auto edge : m_imageGraph->getEdges())
	{
		const OutputPin* sourcePin = edge->getSource();
		if (!sourcePin)
		{
			log::warning << L"Invalid edge, no source pin." << Endl;
			continue;
		}

		const InputPin* destinationPin = edge->getDestination();
		if (!destinationPin)
		{
			log::warning << L"Invalid edge, no destination pin." << Endl;
			continue;
		}

		ui::Node* editorSourceNode = nodeMap[sourcePin->getNode()];
		if (!editorSourceNode)
		{
			log::warning << L"Invalid pin, no editor source node found of pin \"" << sourcePin->getName() << L"\"." << Endl;
			continue;
		}

		ui::Node* editorDestinationNode = nodeMap[destinationPin->getNode()];
		if (!editorDestinationNode)
		{
			log::warning << L"Invalid pin, no editor destination node found of pin \"" << destinationPin->getName() << L"\"." << Endl;
			continue;
		}

		ui::Pin* editorSourcePin = editorSourceNode->findOutputPin(sourcePin->getName());
		if (!editorSourcePin)
		{
			log::warning << L"Unable to find editor source pin \"" << sourcePin->getName() << L"\"." << Endl;
			continue;
		}

		ui::Pin* editorDestinationPin = editorDestinationNode->findInputPin(destinationPin->getName());
		if (!editorDestinationPin)
		{
			log::warning << L"Unable to find editor destination pin \"" << destinationPin->getName() << L"\"." << Endl;
			continue;
		}

		Ref< ui::Edge > editorEdge = new ui::Edge(editorSourcePin, editorDestinationPin);
		editorEdge->setData(L"IMGEDGE", edge);

		m_editorGraph->addEdge(editorEdge);
	}

	m_editorGraph->center();
	m_editorGraph->update();
}

void ImageGraphEditorPage::eventButtonDown(ui::MouseButtonDownEvent* event)
{
	if (event->getButton() != ui::MbtRight)
		return;

	const ui::MenuItem* selected = m_menuPopup->showModal(m_editorGraph, event->getPosition());
	if (!selected)
		return;

	const ui::Command& command = selected->getCommand();
	handleCommand(command);
}

void ImageGraphEditorPage::eventSelect(ui::SelectEvent* event)
{
	RefArray< ui::Node > nodes;
	if (m_editorGraph->getSelectedNodes(nodes) == 1)
	{
		Node* node = nodes[0]->getData< Node >(L"IMGNODE");
		if (node)
		{
			m_site->setPropertyObject(node);
			return;
		}
	}
	m_site->setPropertyObject(nullptr);
}

void ImageGraphEditorPage::eventNodeMoved(ui::NodeMovedEvent* event)
{
	const ui::Node* editorNode = event->getNode();
	Node* node = editorNode->getData< Node >(L"IMGNODE");

	// Get dpi agnostic position.
	ui::Point position = editorNode->getPosition();
	position.x = ui::invdpi96(position.x);
	position.y = ui::invdpi96(position.y);

	// Save position in node.
	node->setPosition(std::make_pair(
		position.x,
		position.y
	));
}

void ImageGraphEditorPage::eventEdgeConnect(ui::EdgeConnectEvent* event)
{
	Ref< ui::Edge > editorEdge = event->getEdge();
	Ref< ui::Pin > editorSourcePin = editorEdge->getSourcePin();
	T_ASSERT(editorSourcePin);

	ui::Pin* editorDestinationPin = editorEdge->getDestinationPin();
	T_ASSERT(editorDestinationPin);

	Node* sourceNode = editorSourcePin->getNode()->getData< Node >(L"IMGNODE");
	T_ASSERT(sourceNode);

	Node* destinationNode = editorDestinationPin->getNode()->getData< Node >(L"IMGNODE");
	T_ASSERT(destinationNode);

	const OutputPin* sourcePin = sourceNode->findOutputPin(editorSourcePin->getName());
	T_ASSERT(sourcePin);

	const InputPin* destinationPin = destinationNode->findInputPin(editorDestinationPin->getName());
	T_ASSERT(destinationPin);

	// Replace existing edge.
	Ref< Edge > edge = m_imageGraph->findEdge(destinationPin);
	if (edge)
	{
		m_imageGraph->removeEdge(edge);

		RefArray< ui::Edge > editorEdges;
		m_editorGraph->getConnectedEdges(editorDestinationPin, editorEdges);
		T_ASSERT(editorEdges.size() == 1);

		m_editorGraph->removeEdge(editorEdges.front());
	}

	m_document->push();

	edge = new Edge(sourcePin, destinationPin);
	m_imageGraph->addEdge(edge);

	editorEdge->setData(L"IMGEDGE", edge);
	m_editorGraph->addEdge(editorEdge);
}

void ImageGraphEditorPage::eventEdgeDisconnect(ui::EdgeDisconnectEvent* event)
{
	ui::Edge* editorEdge = event->getEdge();
	Edge* edge = mandatory_non_null_type_cast< Edge* >(editorEdge->getData(L"IMGEDGE"));

	m_document->push();
	m_imageGraph->removeEdge(edge);
}

	}
}
