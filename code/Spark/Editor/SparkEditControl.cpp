#include "Core/Log/Log.h"
#include "Core/Math/Format.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyGroup.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "Spark/CharacterInstance.h"
#include "Spark/SparkRenderer.h"
#include "Spark/Editor/CharacterAdapter.h"
#include "Spark/Editor/Context.h"
#include "Spark/Editor/SparkEditControl.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/Application.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

void drawBound(CharacterInstance* character, render::PrimitiveRenderer* primitiveRenderer)
{
	if (!character)
		return;

	Aabb2 bounds = character->getBounds();

	Matrix33 T = character->getFullTransform();

	Vector2 e[] =
	{
		T * bounds.mn,
		T * Vector2(bounds.mx.x, bounds.mn.y),
		T * bounds.mx,
		T * Vector2(bounds.mn.x, bounds.mx.y)
	};

	float cx = T.e13;
	float cy = T.e23;
	float r = 20.0f;

	primitiveRenderer->drawLine(
		Vector4(cx - r, cy, 1.0f, 1.0f),
		Vector4(cx + r, cy, 1.0f, 1.0f),
		Color4ub(0, 255, 0, 255)
	);
	primitiveRenderer->drawLine(
		Vector4(cx, cy - r, 1.0f, 1.0f),
		Vector4(cx, cy + r, 1.0f, 1.0f),
		Color4ub(0, 255, 0, 255)
	);

	primitiveRenderer->drawLine(
		Vector4(e[0].x, e[0].y, 1.0f, 1.0f),
		Vector4(e[1].x, e[1].y, 1.0f, 1.0f),
		Color4ub(255, 255, 0, 255)
	);
	primitiveRenderer->drawLine(
		Vector4(e[1].x, e[1].y, 1.0f, 1.0f),
		Vector4(e[2].x, e[2].y, 1.0f, 1.0f),
		Color4ub(255, 255, 0, 255)
	);
	primitiveRenderer->drawLine(
		Vector4(e[2].x, e[2].y, 1.0f, 1.0f),
		Vector4(e[3].x, e[3].y, 1.0f, 1.0f),
		Color4ub(255, 255, 0, 255)
	);
	primitiveRenderer->drawLine(
		Vector4(e[3].x, e[3].y, 1.0f, 1.0f),
		Vector4(e[0].x, e[0].y, 1.0f, 1.0f),
		Color4ub(255, 255, 0, 255)
	);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.SparkEditControl", SparkEditControl, ui::Widget)

SparkEditControl::SparkEditControl(editor::IEditor* editor, editor::IEditorPageSite* site, Context* context)
:	m_editor(editor)
,	m_site(site)
,	m_context(context)
,	m_editMode(EmIdle)
,	m_viewWidth(1920)
,	m_viewHeight(1080)
,	m_viewOffset(1920.0f, 1080.0f)
,	m_viewScale(0.3f)
{
}

bool SparkEditControl::create(
	ui::Widget* parent,
	int style,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem
)
{
	if (!Widget::create(parent, style))
		return false;

	render::RenderViewEmbeddedDesc desc;
	desc.depthBits = 16;
	desc.stencilBits = 8;
	desc.multiSample = 4;
	desc.waitVBlank = false;
	desc.nativeWindowHandle = getIWidget()->getSystemHandle();

	m_renderView = renderSystem->createRenderView(desc);
	if (!m_renderView)
		return false;

	m_primitiveRenderer = new render::PrimitiveRenderer();
	if (!m_primitiveRenderer->create(resourceManager, renderSystem))
		return false;

	m_sparkRenderer = new SparkRenderer();
	m_sparkRenderer->create(1);

	addEventHandler< ui::SizeEvent >(this, &SparkEditControl::eventSize);
	addEventHandler< ui::PaintEvent >(this, &SparkEditControl::eventPaint);
	addEventHandler< ui::MouseButtonDownEvent >(this, &SparkEditControl::eventMouseButtonDown);
	addEventHandler< ui::MouseButtonUpEvent >(this, &SparkEditControl::eventMouseButtonUp);
	addEventHandler< ui::MouseMoveEvent >(this, &SparkEditControl::eventMouseMove);
	addEventHandler< ui::MouseWheelEvent >(this, &SparkEditControl::eventMouseWheel);

	m_idleEventHandler = ui::Application::getInstance()->addEventHandler< ui::IdleEvent >(this, &SparkEditControl::eventIdle);
	return true;
}

void SparkEditControl::destroy()
{
	ui::Application::getInstance()->removeEventHandler< ui::IdleEvent >(m_idleEventHandler);
	safeDestroy(m_sparkRenderer);
	safeDestroy(m_primitiveRenderer);
	safeClose(m_renderView);
	Widget::destroy();
}

void SparkEditControl::setViewSize(int32_t width, int32_t height)
{
	if (width != m_viewWidth || height != m_viewHeight)
	{
		m_viewWidth = width;
		m_viewHeight = height;
		update();
	}
}

Vector2 SparkEditControl::clientToView(const ui::Point& point) const
{
	ui::Size sz = getInnerRect().getSize();

	float viewWidth = sz.cx / m_viewScale;
	float viewHeight = sz.cy / m_viewScale;

	float viewX = m_viewOffset.x / viewWidth;
	float viewY = m_viewOffset.y / viewHeight;

	Matrix44 projection(
		2.0f / viewWidth, 0.0f, 0.0f, -viewX,
		0.0f, -2.0f / viewHeight, 0.0f, viewY,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	Vector4 clientPosition(
		2.0f * float(point.x) / sz.cx - 1.0f,
		1.0f - 2.0f * float(point.y) / sz.cy,
		0.0f,
		1.0f
	);

	Vector4 viewPosition = projection.inverse() * clientPosition;
	return Vector2(
		viewPosition.x(),
		viewPosition.y()
	);
}

void SparkEditControl::eventSize(ui::SizeEvent* event)
{
	ui::Size sz = event->getSize();
	if (m_renderView)
	{
		m_renderView->reset(sz.cx, sz.cy);
		m_renderView->setViewport(render::Viewport(0, 0, sz.cx, sz.cy, 0, 1));
	}
	update();
}

void SparkEditControl::eventPaint(ui::PaintEvent* event)
{
	if (!m_renderView)
		return;

	ui::Size sz = getInnerRect().getSize();

	if (m_renderView->begin(render::EtCyclop))
	{
		const PropertyGroup* settings = m_editor->getSettings();
		T_ASSERT (settings);

		Color4ub clearColor = settings->getProperty< PropertyColor >(L"Editor.Colors/Background");

		float c[4];
		clearColor.getRGBA32F(c);
		const Color4f cc(c);

		m_renderView->clear(
			render::CfColor | render::CfDepth | render::CfStencil,
			&cc,
			1.0f,
			0
		);

		float viewWidth = sz.cx / m_viewScale;
		float viewHeight = sz.cy / m_viewScale;

		float viewX = m_viewOffset.x / viewWidth;
		float viewY = m_viewOffset.y / viewHeight;

		Matrix44 projection(
			2.0f / viewWidth, 0.0f, 0.0f, -viewX,
			0.0f, -2.0f / viewHeight, 0.0f, viewY,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);

		if (m_primitiveRenderer->begin(m_renderView, projection))
		{
			m_primitiveRenderer->pushDepthState(false, false, false);

			for (int32_t x = 0; x < m_viewWidth; x += 40)
			{
				m_primitiveRenderer->drawLine(
					Vector4(x, 0.0f, 1.0f, 1.0f),
					Vector4(x, m_viewHeight, 1.0f, 1.0f),
					Color4ub(0, 0, 0, 40)
				);
			}
			for (int32_t y = 0; y < m_viewHeight; y += 40)
			{
				m_primitiveRenderer->drawLine(
					Vector4(0.0f, y, 1.0f, 1.0f),
					Vector4(m_viewWidth, y, 1.0f, 1.0f),
					Color4ub(0, 0, 0, 40)
				);
			}

			m_primitiveRenderer->drawLine(Vector4(0.0f, 0.0f, 1.0f, 1.0f), Vector4(m_viewWidth, 0.0f, 1.0f, 1.0f), Color4ub(0, 0, 0, 255));
			m_primitiveRenderer->drawLine(Vector4(m_viewWidth, 0.0f, 1.0f, 1.0f), Vector4(m_viewWidth, m_viewHeight, 1.0f, 1.0f), Color4ub(0, 0, 0, 255));
			m_primitiveRenderer->drawLine(Vector4(m_viewWidth, m_viewHeight, 1.0f, 1.0f), Vector4(0.0f, m_viewHeight, 1.0f, 1.0f), Color4ub(0, 0, 0, 255));
			m_primitiveRenderer->drawLine(Vector4(0.0f, m_viewHeight, 1.0f, 1.0f), Vector4(0.0f, 0.0f, 1.0f, 1.0f), Color4ub(0, 0, 0, 255));

			m_primitiveRenderer->popDepthState();
			m_primitiveRenderer->end();
		}

		// Draw sprites.
		if (m_sparkRenderer && m_context->getRoot())
		{
			m_sparkRenderer->build(m_context->getRoot()->getCharacterInstance(), 0);
			m_sparkRenderer->render(m_renderView, projection, 0);

			// Draw bounding boxes.
			if (m_primitiveRenderer->begin(m_renderView, projection))
			{
				m_primitiveRenderer->pushDepthState(false, false, false);

				const RefArray< CharacterAdapter >& children = m_context->getRoot()->getChildren();
				for (RefArray< CharacterAdapter >::const_iterator i = children.begin(); i != children.end(); ++i)
					drawBound((*i)->getCharacterInstance(), m_primitiveRenderer);

				m_primitiveRenderer->popDepthState();
				m_primitiveRenderer->end();
			}
		}

		m_renderView->end();
		m_renderView->present();
	}

	event->consume();
}

void SparkEditControl::eventMouseButtonDown(ui::MouseButtonDownEvent* event)
{
	m_lastMousePosition = event->getPosition();
	if ((event->getKeyState() & ui::KsMenu) == 0)
	{
		Vector2 viewPosition = clientToView(event->getPosition());
		if ((m_editCharacter = m_context->hitTest(viewPosition)) != 0)
		{
			if ((event->getKeyState() & ui::KsControl) == 0)
			{
				m_editMode = EmMoveCharacter;
				m_editAxisFactor = Vector2(1.0f, 1.0f);
			}
			else
			{
				m_editMode = EmMoveCharacterAxis;
				m_editAxisFactor = Vector2(0.0f, 0.0f);
			}

			setCapture();
		}
	}
	else
	{
		m_editMode = EmPanView;
		setCapture();
	}
}

void SparkEditControl::eventMouseButtonUp(ui::MouseButtonUpEvent* event)
{
	if (!hasCapture())
		return;

	m_editMode = EmIdle;
	m_editCharacter = 0;

	releaseCapture();
}

void SparkEditControl::eventMouseMove(ui::MouseMoveEvent* event)
{
	ui::Point mousePosition = event->getPosition();
	if (m_editMode == EmPanView)
	{
		Vector2 deltaMove(
			-(mousePosition.x - m_lastMousePosition.x),
			-(mousePosition.y - m_lastMousePosition.y)
		);
		m_viewOffset += 2.0f * deltaMove / m_viewScale;
	}
	else if (m_editMode == EmMoveCharacter || m_editMode == EmMoveCharacterAxis)
	{
		Vector2 from = clientToView(m_lastMousePosition);
		Vector2 to = clientToView(mousePosition);
		Vector2 delta = to - from;

		if (m_editMode == EmMoveCharacterAxis)
		{
			if (abs(delta.x) > abs(delta.y))
				m_editAxisFactor = Vector2(1.0f, 0.0f);
			else
				m_editAxisFactor = Vector2(0.0f, 1.0f);

			m_editMode = EmMoveCharacter;
		}

		delta *= m_editAxisFactor;

		Matrix33 T = m_editCharacter->getTransform();
		m_editCharacter->setTransform(translate(delta.x, delta.y) * T);
	}
	m_lastMousePosition = mousePosition;
	update();
}

void SparkEditControl::eventMouseWheel(ui::MouseWheelEvent* event)
{
	m_viewScale += event->getRotation() * 0.1f;
	m_viewScale = clamp(m_viewScale, 0.1f, 1000.0f);
	update();
}

void SparkEditControl::eventIdle(ui::IdleEvent* event)
{
	if (isVisible(true))
	{
		update();
		event->requestMore();
	}
}

	}
}
