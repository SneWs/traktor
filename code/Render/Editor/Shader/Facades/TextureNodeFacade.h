#ifndef traktor_render_TextureNodeFacade_H
#define traktor_render_TextureNodeFacade_H

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

class Texture;

class TextureNodeFacade : public NodeFacade
{
	T_RTTI_CLASS;

public:
	TextureNodeFacade(ui::custom::GraphControl* graphControl);

	virtual Ref< Node > createShaderNode(
		const TypeInfo* nodeType,
		editor::IEditor* editor
	);

	virtual Ref< ui::custom::Node > createEditorNode(
		editor::IEditor* editor,
		ui::custom::GraphControl* graphControl,
		Node* shaderNode
	);

	virtual void editShaderNode(
		editor::IEditor* editor,
		ui::custom::GraphControl* graphControl,
		ui::custom::Node* editorNode,
		Node* shaderNode
	);

	virtual void refreshEditorNode(
		editor::IEditor* editor,
		ui::custom::GraphControl* graphControl,
		ui::custom::Node* editorNode,
		Node* shaderNode
	);

	virtual void setValidationIndicator(
		ui::custom::Node* editorNode,
		bool validationSucceeded
	);

private:
	Ref< ui::custom::NodeShape > m_nodeShape;

	void updateThumb(editor::IEditor* editor, ui::custom::Node* editorNode, Texture* texture) const;
};

	}
}

#endif	// traktor_render_TextureNodeFacade_H
