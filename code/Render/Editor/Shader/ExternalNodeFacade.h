#ifndef traktor_render_ExternalNodeFacade_H
#define traktor_render_ExternalNodeFacade_H

#include "Core/Heap/Ref.h"
#include "Render/Editor/Shader/NodeFacade.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

class GraphControl;
class NodeShape;

		}
	}

	namespace render
	{

class ExternalNodeFacade : public NodeFacade
{
	T_RTTI_CLASS(ExternalNodeFacade)

public:
	ExternalNodeFacade(ui::custom::GraphControl* graphControl);

	virtual Node* createShaderNode(
		const Type* nodeType,
		editor::IEditor* editor
	);

	virtual ui::custom::Node* createEditorNode(
		editor::IEditor* editor,
		ui::custom::GraphControl* graphControl,
		Node* shaderNode
	);

	virtual void editShaderNode(
		editor::IEditor* editor,
		ui::custom::GraphControl* graphControl,
		Node* shaderNode
	);

	virtual void setValidationIndicator(
		ui::custom::Node* editorNode,
		bool validationSucceeded
	);

private:
	Ref< ui::custom::NodeShape > m_nodeShape;
};

	}
}

#endif	// traktor_render_ExternalNodeFacade_H
