#pragma once

#include "Ui/Graph/NodeShape.h"

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

/*! \brief Comment node shape.
 * \ingroup UI
 */
class T_DLLCLASS CommentNodeShape : public NodeShape
{
	T_RTTI_CLASS;

public:
	CommentNodeShape(GraphControl* graphControl);

	virtual Point getPinPosition(const Node* node, const Pin* pin) const override final;

	virtual Pin* getPinAt(const Node* node, const Point& pt) const override final;

	virtual void paint(const Node* node, GraphCanvas* canvas, const Size& offset) const override final;

	virtual Size calculateSize(const Node* node) const override final;

private:
	GraphControl* m_graphControl;
	Ref< IBitmap > m_imageNode;
};

	}
}

