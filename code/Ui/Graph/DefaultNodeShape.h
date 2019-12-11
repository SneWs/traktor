#pragma once

#include "Ui/Graph/INodeShape.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class GraphControl;
class IBitmap;

/*! Default node shape.
 * \ingroup UI
 */
class T_DLLCLASS DefaultNodeShape : public INodeShape
{
	T_RTTI_CLASS;

public:
	enum Style
	{
		StDefault = 0,
		StExternal = 1,
		StScript = 2
	};

	DefaultNodeShape(GraphControl* graphControl, Style style);

	virtual Point getPinPosition(const Node* node, const Pin* pin) const override final;

	virtual Pin* getPinAt(const Node* node, const Point& pt) const override final;

	virtual void paint(const Node* node, const Pin* hotPin, GraphCanvas* canvas, const Size& offset) const override final;

	virtual Size calculateSize(const Node* node) const override final;

private:
	GraphControl* m_graphControl;
	Ref< IBitmap > m_imageNode[4];
	Ref< IBitmap > m_imagePin;
	Ref< IBitmap > m_imagePinHot;
};

	}
}

