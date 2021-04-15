#include <algorithm>
#include <cmath>
#include "Drawing/Image.h"
#include "Ui/Application.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Graph/GraphCanvas.h"
#include "Ui/Graph/IpolNodeShape.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/Pin.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

const int c_pinHitWidth = 14;	/*< Width of pin hit area from visual edge. */

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.IpolNodeShape", IpolNodeShape, INodeShape)

IpolNodeShape::IpolNodeShape()
{
	m_imageNode[0] = new ui::StyleBitmap(L"UI.Graph.Ipol");
	m_imageNode[1] = new ui::StyleBitmap(L"UI.Graph.IpolSelected");
	m_imageNode[2] = new ui::StyleBitmap(L"UI.Graph.IpolError");
	m_imageNode[3] = new ui::StyleBitmap(L"UI.Graph.IpolErrorSelected");

	m_imagePin = new ui::StyleBitmap(L"UI.Graph.Pin");
	m_imagePinHot = new ui::StyleBitmap(L"UI.Graph.PinHot");
}

Point IpolNodeShape::getPinPosition(GraphControl* graph, const Node* node, const Pin* pin) const
{
	Rect rc = node->calculateRect();

	int32_t f = dpi96(0);
	int32_t x = pin->getDirection() == Pin::DrInput ? -f : rc.getWidth() + f;
	int32_t y = rc.getHeight() / 2;

	return Point(rc.left + x, rc.top + y);
}

Pin* IpolNodeShape::getPinAt(GraphControl* graph, const Node* node, const Point& pt) const
{
	Rect rc = node->calculateRect();

	int32_t f = dpi96(4);
	int32_t x = pt.x - rc.left;
	int32_t y = pt.y - rc.top;

	if (y >= rc.getHeight() / 2 - f && y <= rc.getHeight() / 2 + f)
	{
		if (x >= -f && x <= dpi96(c_pinHitWidth))
			return node->getInputPins()[0];
		if (x >= rc.getWidth() - dpi96(c_pinHitWidth) && x <= rc.getWidth() + f)
			return node->getOutputPins()[0];
	}

	return nullptr;
}

void IpolNodeShape::paint(GraphControl* graph, const Node* node, GraphCanvas* canvas, const Pin* hotPin, const Size& offset) const
{
	Rect rc = node->calculateRect().offset(offset);

	int32_t imageIndex = (node->isSelected() ? 1 : 0) + (node->getState() ? 2 : 0);
	Size sz = m_imageNode[imageIndex]->getSize();

	canvas->drawBitmap(
		rc.getTopLeft(),
		sz,
		Point(0, 0),
		sz,
		m_imageNode[imageIndex],
		ui::BmAlpha
	);

	int32_t f = dpi96(0);
	Size pinSize = m_imagePin->getSize();

	canvas->drawBitmap(
		Point(rc.left - f - pinSize.cx / 2, rc.getCenter().y - pinSize.cy / 2),
		pinSize,
		Point(0, 0),
		pinSize,
		hotPin == node->getInputPins()[0] ? m_imagePinHot : m_imagePin,
		ui::BmAlpha
	);

	canvas->drawBitmap(
		Point(rc.right + f - pinSize.cx / 2, rc.getCenter().y - pinSize.cy / 2),
		pinSize,
		Point(0, 0),
		pinSize,
		hotPin == node->getOutputPins()[0] ? m_imagePinHot : m_imagePin,
		ui::BmAlpha
	);
}

Size IpolNodeShape::calculateSize(GraphControl* graph, const Node* node) const
{
	int32_t imageIndex = (node->isSelected() ? 1 : 0) + (node->getState() ? 2 : 0);
	return m_imageNode[imageIndex]->getSize();
}

	}
}
