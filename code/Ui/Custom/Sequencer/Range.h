#ifndef traktor_ui_custom_Range_H
#define traktor_ui_custom_Range_H

#include "Ui/Custom/Sequencer/Key.h"

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
		namespace custom
		{

/*! \brief Sequencer range.
 * \ingroup UIC
 */
class T_DLLCLASS Range : public Key
{
	T_RTTI_CLASS;

public:
	Range(int start, int end, bool movable);

	void setStart(int start);

	int getStart() const;

	void setEnd(int end);

	int getEnd() const;

	virtual void move(int offset);

	virtual void getRect(const Sequence* sequence, const Rect& rcClient, Rect& outRect) const;

	virtual void paint(ui::Canvas& canvas, const Sequence* sequence, const Rect& rcClient, int scrollOffset);

private:
	int m_start;
	int m_end;
	bool m_movable;
};

		}
	}
}

#endif	// traktor_ui_custom_Range_H
