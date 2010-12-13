#ifndef traktor_ui_custom_RichEdit_H
#define traktor_ui_custom_RichEdit_H

#include <list>
#include "Ui/Widget.h"

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

class ScrollBar;

		namespace custom
		{

/*! \brief Rich, multi-line, text editor.
 * \ingroup UIC
 */
class T_DLLCLASS RichEdit : public Widget
{
	T_RTTI_CLASS;

public:
	RichEdit();

	bool create(Widget* parent, const std::wstring& text);

	int addAttribute(const Color4ub& textColor, const Color4ub& backColor, bool bold, bool italic, bool underline);

	void setAttribute(int start, int length, int attribute);

	void clear(bool attributes, bool content);

	void insert(const std::wstring& text);

	int getCaretOffset() const;

	int getLineFromOffset(int offset) const;

	int getLineCount() const;

	int getLineOffset(int line) const;

	int getLineLength(int line) const;

	std::wstring getLine(int line) const;

	bool redo();

	bool undo();

	void addChangeEventHandler(EventHandler* eventHandler);

private:
	struct Attribute
	{
		Color4ub textColor;
		Color4ub backColor;
		bool bold;
		bool italic;
		bool underline;
	};

	struct Format
	{
		uint32_t start;
		uint32_t stop;
		uint32_t attribute;
	};

	struct Line
	{
		uint32_t start;
		uint32_t stop;
	};

	Ref< ScrollBar > m_scrollBar;
	std::vector< Attribute > m_attributes;
	std::vector< Format > m_formats;
	std::vector< Line > m_lines;
	std::vector< wchar_t > m_text;
	uint32_t m_caret;

	void updateScrollBars();

	void eventKeyDown(Event* event);

	void eventKey(Event* event);

	void eventPaint(Event* event);

	void eventSize(Event* event);

	void eventScroll(Event* event);
};

		}
	}
}

#endif	// traktor_ui_custom_RichEdit_H
