#pragma once

#include "Core/RefArray.h"
#include "Ui/Auto/AutoWidget.h"

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

class ListBoxItem;

/*! \brief List box control.
 * \ingroup UI
 */
class T_DLLCLASS ListBox : public AutoWidget
{
	T_RTTI_CLASS;

public:
	enum Styles
	{
		WsSingle = 0,
		WsMultiple = WsUser,
		WsExtended = (WsUser << 1),
		WsSort = (WsUser << 2),
		WsDefault = WsSingle
	};

	ListBox();

	bool create(Widget* parent, int32_t style = WsDefault);

	virtual Size getPreferedSize() const override;

	int32_t add(const std::wstring& item, Object* data = 0);

	bool remove(int32_t index);

	void removeAll();

	int32_t count() const;

	void setItem(int32_t index, const std::wstring& item);

	void setData(int32_t index, Object* data);

	std::wstring getItem(int32_t index) const;

	Ref< Object > getData(int32_t index) const;

	void select(int32_t index);

	bool selected(int32_t index) const;

	int32_t getSelected(std::vector< int32_t >& selected) const;

	int32_t getSelected() const;

	std::wstring getSelectedItem() const;

	Ref< Object > getSelectedData() const;

	int32_t getItemHeight();

	Rect getItemRect(int32_t index) const;

	template < typename T >
	Ref< T > getData(int index) const
	{
		return dynamic_type_cast< T* >(getData(index));
	}

	template < typename T >
	Ref< T > getSelectedData() const
	{
		return dynamic_type_cast< T* >(getSelectedData());
	}

private:
	friend class ListBoxItem;

	int32_t m_style;
	int32_t m_lastHitIndex;
	RefArray< ListBoxItem > m_items;

	void eventButtonDown(MouseButtonDownEvent* event);

	virtual void layoutCells(const Rect& rc) override;
};

	}
}

