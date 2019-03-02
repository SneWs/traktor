#include "Core/Misc/String.h"
#include "I18N/Text.h"
#include "Render/Editor/Shader/InputPin.h"
#include "Render/Editor/Shader/OutputPin.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/Facades/DefaultNodeFacade.h"
#include "Ui/Application.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/DefaultNodeShape.h"
#include "Ui/Graph/InputNodeShape.h"
#include "Ui/Graph/OutputNodeShape.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.DefaultNodeFacade", DefaultNodeFacade, INodeFacade)

DefaultNodeFacade::DefaultNodeFacade(ui::GraphControl* graphControl)
{
	m_nodeShapes[0] = new ui::DefaultNodeShape(graphControl);
	m_nodeShapes[1] = new ui::InputNodeShape(graphControl);
	m_nodeShapes[2] = new ui::OutputNodeShape(graphControl);
}

Ref< Node > DefaultNodeFacade::createShaderNode(
	const TypeInfo* nodeType,
	editor::IEditor* editor
)
{
	return checked_type_cast< Node* >(nodeType->createInstance());
}

Ref< ui::Node > DefaultNodeFacade::createEditorNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	std::wstring title = type_name(shaderNode);
	size_t p = title.find_last_of(L'.');
	if (p > 0)
		title = i18n::Text(L"SHADERGRAPH_NODE_" + toUpper(title.substr(p + 1)));

	Ref< ui::NodeShape > shape;
	if (shaderNode->getInputPinCount() == 1 && shaderNode->getOutputPinCount() == 0)
		shape = m_nodeShapes[2];
	else if (shaderNode->getInputPinCount() == 0 && shaderNode->getOutputPinCount() == 1)
		shape = m_nodeShapes[1];
	else
		shape = m_nodeShapes[0];

	Ref< ui::Node > editorNode = new ui::Node(
		title,
		shaderNode->getInformation(),
		ui::Point(
			ui::dpi96(shaderNode->getPosition().first),
			ui::dpi96(shaderNode->getPosition().second)
		),
		shape
	);

	for (int j = 0; j < shaderNode->getInputPinCount(); ++j)
	{
		const InputPin* inputPin = shaderNode->getInputPin(j);
		editorNode->createInputPin(
			inputPin->getName(),
			!inputPin->isOptional()
		);
	}

	for (int j = 0; j < shaderNode->getOutputPinCount(); ++j)
	{
		const OutputPin* outputPin = shaderNode->getOutputPin(j);
		editorNode->createOutputPin(
			outputPin->getName()
		);
	}

	editorNode->setComment(shaderNode->getComment());
	return editorNode;
}

void DefaultNodeFacade::editShaderNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ui::Node* editorNode,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
}

void DefaultNodeFacade::refreshEditorNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ui::Node* editorNode,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	editorNode->setComment(shaderNode->getComment());
	editorNode->setInfo(shaderNode->getInformation());
}

void DefaultNodeFacade::setValidationIndicator(
	ui::Node* editorNode,
	bool validationSucceeded
)
{
	editorNode->setState(validationSucceeded ? 0 : 1);
}

	}
}
