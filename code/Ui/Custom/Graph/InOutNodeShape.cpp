/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include <cmath>
#include "Drawing/Image.h"
#include "Ui/Application.h"
#include "Ui/StyleBitmap.h"
#include "Ui/Custom/Graph/InOutNodeShape.h"
#include "Ui/Custom/Graph/GraphControl.h"
#include "Ui/Custom/Graph/PaintSettings.h"
#include "Ui/Custom/Graph/Node.h"
#include "Ui/Custom/Graph/Pin.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

const int32_t c_marginWidth = 3;	/*< Distance from image edge to "visual" edge. */
const int32_t c_textMargin = 8;
const int32_t c_textHeight = 16;
const int32_t c_minExtent = 30;
const int32_t c_pinHitWidth = 14;	/*< Width of pin hit area from visual edge. */

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.InOutNodeShape", InOutNodeShape, NodeShape)

InOutNodeShape::InOutNodeShape(GraphControl* graphControl)
:	m_graphControl(graphControl)
{
	m_imageNode[0] = new ui::StyleBitmap(L"UI.Graph.InOut");
	m_imageNode[1] = new ui::StyleBitmap(L"UI.Graph.InOutSelected");
	m_imageNode[2] = new ui::StyleBitmap(L"UI.Graph.InOutError");
	m_imageNode[3] = new ui::StyleBitmap(L"UI.Graph.InOutErrorSelected");

	m_imagePin = new ui::StyleBitmap(L"UI.Graph.Pin");
}

Point InOutNodeShape::getPinPosition(const Node* node, const Pin* pin)
{
	Rect rc = node->calculateRect();
	Point pt;

	if (pin->getDirection() == Pin::DrInput)
		pt = Point(rc.left, rc.getCenter().y);
	else // DrOutput
		pt = Point(rc.right, rc.getCenter().y);

	return pt;
}

Pin* InOutNodeShape::getPinAt(const Node* node, const Point& pt)
{
	Rect rc = node->calculateRect();

	int32_t x = pt.x - rc.left;
	int32_t y = pt.y - rc.top;
	int32_t f = ui::scaleBySystemDPI(4);

	if (x >= 0 && x <= ui::scaleBySystemDPI(c_pinHitWidth) && y >= rc.getHeight() / 2 - f && y <= rc.getHeight() + f)
		return node->getInputPins()[0];

	if (x >= rc.getWidth() - ui::scaleBySystemDPI(c_pinHitWidth) && x <= rc.getWidth() && y >= rc.getHeight() / 2 - f && y <= rc.getHeight() + f)
		return node->getOutputPins()[0];

	return 0;
}

void InOutNodeShape::paint(const Node* node, const PaintSettings* settings, Canvas* canvas, const Size& offset)
{
	Rect rc = node->calculateRect().offset(offset);

	// Draw node shape.
	{
		int32_t imageIndex = (node->isSelected() ? 1 : 0) + (node->getState() ? 2 : 0);
		Size sz = m_imageNode[imageIndex]->getSize();

		int32_t tw = sz.cx / 3;
		int32_t th = sz.cy / 3;

		int32_t sx[] = { 0, tw, sz.cx - tw, sz.cx };
		int32_t dx[] = { 0, tw, rc.getWidth() - tw, rc.getWidth() };

		for (int32_t ix = 0; ix < 3; ++ix)
		{
			canvas->drawBitmap(
				rc.getTopLeft() + Size(dx[ix], 0),
				Size(dx[ix + 1] - dx[ix], sz.cy),
				Point(sx[ix], 0),
				Size(sx[ix + 1] - sx[ix], sz.cy),
				m_imageNode[imageIndex],
				ui::BmAlpha
			);
		}
	}

	Size pinSize = m_imagePin->getSize();

	canvas->setBackground(Color4ub(255, 255, 255));

	Point inputPinPos(
		rc.left - pinSize.cx / 2 + ui::scaleBySystemDPI(c_marginWidth),
		rc.getCenter().y - pinSize.cy / 2
	);

	canvas->drawBitmap(
		inputPinPos,
		Point(0, 0),
		pinSize,
		m_imagePin,
		ui::BmAlpha
	);

	Point outputPinPos(
		rc.right - pinSize.cx / 2 - ui::scaleBySystemDPI(c_marginWidth),
		rc.getCenter().y - pinSize.cy / 2
	);

	canvas->drawBitmap(
		outputPinPos,
		Point(0, 0),
		pinSize,
		m_imagePin,
		ui::BmAlpha
	);

	std::wstring info = node->getInfo();
	if (!info.empty())
	{
		Size ext = canvas->getTextExtent(info);

		canvas->setForeground(settings->getNodeTextInfo());
		canvas->drawText(
			Point(
				rc.left + (rc.getWidth() - ext.cx) / 2,
				rc.top + (rc.getHeight() - ext.cy) / 2
			),
			info
		);
	}

	const std::wstring& comment = node->getComment();
	if (!comment.empty())
	{
		canvas->setForeground(settings->getNodeShadow());
		canvas->drawText(Rect(rc.left, rc.top - ui::scaleBySystemDPI(c_textHeight), rc.right, rc.top), comment, AnCenter, AnCenter);
	}
}

Size InOutNodeShape::calculateSize(const Node* node)
{
	Font currentFont = m_graphControl->getFont();
	
	int32_t imageIndex = (node->isSelected() ? 1 : 0) + (node->getState() ? 2 : 0);
	Size sz = m_imageNode[imageIndex]->getSize();

	int32_t width = ui::scaleBySystemDPI(c_marginWidth) * 2 + ui::scaleBySystemDPI(c_textMargin) * 2;

	if (!node->getInfo().empty())
	{
		m_graphControl->setFont(m_graphControl->getPaintSettings()->getFont());
		int32_t extent = m_graphControl->getTextExtent(node->getInfo()).cx;
		width += std::max(extent, ui::scaleBySystemDPI(c_minExtent));
	}

	m_graphControl->setFont(currentFont);

	return Size(width, sz.cy);
}

		}
	}
}
