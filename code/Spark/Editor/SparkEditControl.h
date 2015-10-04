#ifndef traktor_spark_SparkEditControl_H
#define traktor_spark_SparkEditControl_H

#include "Core/Math/Vector2.h"
#include "Ui/Widget.h"

namespace traktor
{
	namespace editor
	{

class IEditor;
class IEditorPageSite;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace render
	{

class IRenderSystem;
class IRenderView;
class PrimitiveRenderer;
class RenderTargetSet;
class Shader;

	}

	namespace spark
	{

class Context;
class SparkRenderer;

/*! \brief
 * \ingroup Spark
 */
class SparkEditControl : public ui::Widget
{
	T_RTTI_CLASS;

public:
	SparkEditControl(editor::IEditor* editor, editor::IEditorPageSite* site, Context* context);

	bool create(
		ui::Widget* parent,
		int style,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem
	);

	void destroy();

private:
	enum EditMode
	{
		EmIdle,
		EmPanView,
		EmMoveCharacter,
		EmMoveCharacterAxis
	};

	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	Ref< Context > m_context;
	Ref< render::IRenderView > m_renderView;
	Ref< render::PrimitiveRenderer > m_primitiveRenderer;
	Ref< SparkRenderer > m_sparkRenderer;
	Ref< ui::EventSubject::IEventHandler > m_idleEventHandler;
	ui::Point m_lastMousePosition;
	EditMode m_editMode;
	Vector2 m_editAxisFactor;
	Ref< CharacterAdapter > m_editCharacter;
	Vector2 m_viewOffset;
	float m_viewScale;

	Vector2 clientToView(const ui::Point& point) const;

	int32_t hitTest(const ui::Point& point) const;

	void eventSize(ui::SizeEvent* event);

	void eventPaint(ui::PaintEvent* event);

	void eventMouseButtonDown(ui::MouseButtonDownEvent* event);

	void eventMouseButtonUp(ui::MouseButtonUpEvent* event);

	void eventMouseMove(ui::MouseMoveEvent* event);

	void eventMouseWheel(ui::MouseWheelEvent* event);

	void eventIdle(ui::IdleEvent* event);
};

	}
}

#endif	// traktor_spark_SparkEditControl_H
