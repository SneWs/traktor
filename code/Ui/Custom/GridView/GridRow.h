#ifndef traktor_ui_GridRow_H
#define traktor_ui_GridRow_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Ui/Custom/GridView/GridCell.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Bitmap;
class Font;

		namespace custom
		{

class GridCell;

/*! \brief Grid row.
 * \ingroup UIC
 */
class T_DLLCLASS GridRow : public GridCell
{
	T_RTTI_CLASS;

public:
	enum RowStates
	{
		RsExpanded = 1,
		RsSelected = 2
	};

	GridRow(uint32_t initialState = RsExpanded);

	void setState(uint32_t state);

	uint32_t getState() const { return m_state; }

	void setFont(Font* font);

	Ref< Font > getFont() const { return m_font; }

	void add(GridCell* item);

	Ref< GridCell > get(uint32_t index) const;

	const RefArray< GridCell >& get() const { return m_items; }

	void addChild(GridRow* row);

	void insertChildBefore(GridRow* insertBefore, GridRow* row);

	void insertChildAfter(GridRow* insertAfter, GridRow* row);

	void removeChild(GridRow* row);

	void removeAllChildren();

	GridRow* getParent() { return m_parent; }

	const RefArray< GridRow >& getChildren() const { return m_children; }

	void placeCells(AutoWidget* widget, const Rect& rect, const RefArray< GridColumn >& columns);

	virtual void mouseDown(AutoWidget* widget, const Point& position);

	virtual void paint(AutoWidget* widget, Canvas& canvas, const Rect& rect);

	virtual int32_t getHeight() const;

private:
	uint32_t m_state;
	Ref< Font > m_font;
	Ref< Bitmap > m_expand[2];
	RefArray< GridCell > m_items;
	GridRow* m_parent;
	RefArray< GridRow > m_children;

	int32_t getDepth() const;
};

		}
	}
}

#endif	// traktor_ui_GridRow_H
