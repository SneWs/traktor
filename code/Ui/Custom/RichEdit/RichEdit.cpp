#include <cwctype>
#include "Core/Log/Log.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Clipboard.h"
#include "Ui/ScrollBar.h"
#include "Ui/Custom/RichEdit/RichEdit.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

const int32_t c_lineMarginMin = 40;
const int32_t c_iconSize = 16;
const int32_t c_scrollHSteps = 10;

#if defined(__APPLE__)
const int32_t c_fontHeightMargin = 4;
#elif defined(__LINUX__)
const int32_t c_fontHeightMargin = 1;
#else
const int32_t c_fontHeightMargin = 1;
#endif

bool isWordSeparator(wchar_t ch)
{
	return !std::iswalnum(ch);
}

			}

#if defined(_DEBUG)
#	define CHECK checkConsistency()
#else
#	define CHECK
#endif

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.RichEdit", RichEdit, Widget)

RichEdit::RichEdit()
:	m_imageWidth(0)
,	m_imageHeight(0)
,	m_imageCount(0)
,	m_charWidth(0)
,	m_caret(0)
,	m_selectionStart(-1)
,	m_selectionStop(-1)
,	m_lineMargin(c_lineMarginMin)
,	m_lineOffsetH(0)
{
}

bool RichEdit::create(Widget* parent, const std::wstring& text, int32_t style)
{
	if (!Widget::create(parent, style | WsWantAllInput))
		return false;

	addEventHandler< KeyDownEvent >(this, &RichEdit::eventKeyDown);
	addEventHandler< KeyEvent >(this, &RichEdit::eventKey);
	addEventHandler< MouseButtonDownEvent >(this, &RichEdit::eventButtonDown);
	addEventHandler< MouseWheelEvent >(this, &RichEdit::eventMouseWheel);
	addEventHandler< PaintEvent >(this, &RichEdit::eventPaint);
	addEventHandler< SizeEvent >(this, &RichEdit::eventSize);

	// Create scrollbars.
	m_scrollBarV = new ScrollBar();
	m_scrollBarH = new ScrollBar();
	if (!m_scrollBarV->create(this, ScrollBar::WsVertical))
		return false;
	if (!m_scrollBarH->create(this, ScrollBar::WsHorizontal))
		return false;

	m_scrollBarV->addEventHandler< ScrollEvent >(this, &RichEdit::eventScroll);
	m_scrollBarH->addEventHandler< ScrollEvent >(this, &RichEdit::eventScroll);

	Attribute attrib;
	attrib.textColor = Color4ub(0, 0, 0);
	attrib.backColor = Color4ub(255, 255, 255);
	attrib.bold = false;
	attrib.italic = false;
	attrib.underline = false;
	m_attributes.push_back(attrib);

	setText(text);

	CHECK;

	return true;
}

void RichEdit::setText(const std::wstring& text)
{
	m_text.clear();
	m_lines.clear();

	if (!text.empty())
	{
		size_t i = 0;
		while (i < text.length())
		{
			size_t j = text.find(L'\n', i);

			std::wstring ln = (j != text.npos) ? text.substr(i, j - i) : text.substr(i);

			Line line;
			line.start = m_text.size();
			line.stop = line.start + ln.length();
			m_lines.push_back(line);

			m_text.insert(m_text.end(), ln.begin(), ln.end());
			m_text.push_back(L'\n');

			if (j != text.npos)
			{
				i = j + 1;
				if (i < text.length() && text[i] == L'\r')
					++i;
			}
			else
				break;
		}
	}
	else
	{
		Line line;
		line.start = 0;
		line.stop = 0;
		m_lines.push_back(line);
		m_text.push_back(L'\n');
	}

	m_meta.clear();
	m_meta.resize(m_text.size(), 0);

	m_selectionStart =
	m_selectionStop = -1;

	int32_t lastOffset = int32_t(m_text.size());
	if (m_caret >= lastOffset)
		m_caret = lastOffset;

	updateScrollBars();
}

std::wstring RichEdit::getText() const
{
	if (!m_text.empty())
	{
		const wchar_t* text = &m_text[0];
		return std::wstring(text, text + m_text.size());
	}
	else
		return L"";
}

int32_t RichEdit::addAttribute(const Color4ub& textColor, const Color4ub& backColor, bool bold, bool italic, bool underline)
{
	Attribute attr;
	attr.textColor = textColor;
	attr.backColor = backColor;
	attr.bold = bold;
	attr.italic = italic;
	attr.underline = underline;
	m_attributes.push_back(attr);
	return int32_t(m_attributes.size() - 1);
}

void RichEdit::setAttribute(int32_t start, int32_t length, int32_t attribute)
{
	if (attribute < 0 || attribute >= int32_t(m_attributes.size()))
		attribute = 0;

	if (start < 0)
		start = 0;

	if (start + length >= int32_t(m_text.size()))
	{
		length = int32_t(m_text.size()) - start;
		if (length < 0)
			return;
	}

	for (int32_t i = start; i < start + length; ++i)
		m_meta[i] = uint16_t(attribute);

	CHECK;

	update();
}

int32_t RichEdit::addImage(Bitmap* image, uint32_t imageCount)
{
	T_ASSERT (image);
	T_ASSERT (imageCount > 0);

	if (m_image)
	{
		Ref< Bitmap > source = image;

		uint32_t width = m_image->getSize().cx + source->getSize().cx;
		uint32_t height = std::max(m_image->getSize().cy, source->getSize().cy);

		Ref< ui::Bitmap > newImage = new ui::Bitmap(width, height);
		newImage->copyImage(m_image->getImage());
		newImage->copySubImage(image->getImage(), Rect(Point(0, 0), source->getSize()), Point(m_image->getSize().cx, 0));
		m_image = newImage;
	}
	else
	{
		m_image = image;
		m_imageWidth = std::max< uint32_t >(m_imageWidth, m_image->getSize().cx / imageCount);
		m_imageHeight = std::max< uint32_t >(m_imageHeight, m_image->getSize().cy);
	}

	uint32_t imageBase = m_imageCount;
	m_imageCount += imageCount;

	return imageBase;
}

void RichEdit::setImage(int32_t line, int32_t image)
{
	if (line >= 0 && line < int32_t(m_lines.size()))
		m_lines[line].image = image;
}

void RichEdit::clear(bool attributes, bool images, bool content)
{
	if (attributes)
	{
		m_attributes.clear();

		m_meta.clear();
		m_meta.resize(m_text.size(), 0);
	}

	if (content)
	{
		m_lines.clear();
		m_text.clear();
	}
	else if (images)
	{
		for (std::vector< Line >::iterator i = m_lines.begin(); i != m_lines.end(); ++i)
			i->image = -1;
	}

	CHECK;

	updateScrollBars();
	update();
}

void RichEdit::insert(const std::wstring& text)
{
	std::wstring tmp = replaceAll< std::wstring >(text, L"\r\n", L"\n");
	for (std::wstring::const_iterator i = tmp.begin(); i != tmp.end(); ++i)
		insertCharacter(*i);
}

int32_t RichEdit::getCaretOffset() const
{
	return m_caret;
}

int32_t RichEdit::getLineFromOffset(int32_t offset) const
{
	for (int32_t i = 0; i < int32_t(m_lines.size()) - 1; ++i)
	{
		if (offset >= m_lines[i].start && offset <= m_lines[i].stop)
			return i;
	}
	return int32_t(m_lines.size()) - 1;
}

int32_t RichEdit::getLineCount() const
{
	return int32_t(m_lines.size());
}

int32_t RichEdit::getLineOffset(int32_t line) const
{
	return line < int32_t(m_lines.size()) ? m_lines[line].start : 0;
}

int32_t RichEdit::getLineLength(int32_t line) const
{
	return line < int32_t(m_lines.size()) ? (m_lines[line].stop - m_lines[line].start) : 0;
}

void RichEdit::setLine(int32_t line, const std::wstring& text)
{
	if (line >= int32_t(m_lines.size()))
		return;

	Line& ln = m_lines[line];

	m_text.erase(m_text.begin() + ln.start, m_text.begin() + ln.stop);
	m_meta.erase(m_meta.begin() + ln.start, m_meta.begin() + ln.stop);

	for (uint32_t i = 0; i < text.size(); ++i)
	{
		m_text.insert(m_text.begin() + ln.start + i, text[i]);
		m_meta.insert(m_meta.begin() + ln.start + i, 0);
	}

	int32_t adjust = text.size() - (ln.stop - ln.start);
	for (uint32_t i = line + 1; i < m_lines.size(); ++i)
	{
		m_lines[i].start += adjust;
		m_lines[i].stop += adjust;
	}

	ln.stop = ln.start + text.size();
}

std::wstring RichEdit::getLine(int32_t line) const
{
	if (line < int32_t(m_lines.size()))
	{
		const wchar_t* text = &m_text[0];
		return std::wstring(
			text + m_lines[line].start,
			text + m_lines[line].stop + 1
		);
	}
	else
		return L"";
}

int32_t RichEdit::getSelectionStartOffset() const
{
	return m_selectionStart;
}

int32_t RichEdit::getSelectionStopOffset() const
{
	return m_selectionStop;
}

std::wstring RichEdit::getSelectedText() const
{
	if (m_selectionStart < 0 || m_text.empty())
		return L"";

	const wchar_t* text = &m_text[0];
	return std::wstring(text + m_selectionStart, text + m_selectionStop);
}

bool RichEdit::scrollToLine(int32_t line)
{
	if (m_scrollBarV->isVisible(false))
	{
		m_scrollBarV->setPosition(line);
		m_scrollBarV->update();
		update();
	}
	return true;
}

int32_t RichEdit::getScrollLine() const
{
	if (m_scrollBarV->isVisible(false))
		return m_scrollBarV->getPosition();
	else
		return 0;
}

bool RichEdit::showLine(int32_t line)
{
	if (m_scrollBarV->isVisible(false))
	{
		Font font = getFont();
		Rect rc = getEditRect();

		int32_t lineHeight = font.getSize() + c_fontHeightMargin;
		int32_t pageLines = (rc.getHeight() + lineHeight - 1) / lineHeight;

		int32_t top = m_scrollBarV->getPosition();

		if (line >= top && line < top + pageLines)
			return true;

		m_scrollBarV->setPosition(std::max< int32_t >(line - pageLines / 3, 0));
		m_scrollBarV->update();

		update();
	}
	return true;
}

void RichEdit::placeCaret(int32_t offset)
{
	m_caret = offset;
	update();
}

bool RichEdit::copy()
{
	Clipboard* clipboard = Application::getInstance()->getClipboard();
	if (!clipboard)
		return false;

	std::wstring selectedText = getSelectedText();
	return clipboard->setText(selectedText);
}

bool RichEdit::paste()
{
	Clipboard* clipboard = Application::getInstance()->getClipboard();
	if (!clipboard)
		return false;

	if (m_selectionStart >= 0)
		deleteCharacters();

	std::wstring pasteText = clipboard->getText();
	insert(pasteText);

	return true;
}

void RichEdit::updateScrollBars()
{
	Font font = getFont();
	Rect rc = getEditRect();

	uint32_t lineCount = m_lines.size();
	uint32_t lineOffset = 0;
	uint32_t lineHeight = font.getSize() + c_fontHeightMargin;
	uint32_t pageLines = (rc.getHeight() + lineHeight - 1) / lineHeight;

	uint32_t lineWidth = 0;
	if (m_charWidth > 0)
	{
		for (std::vector< Line >::const_iterator i = m_lines.begin(); i != m_lines.end(); ++i)
		{
			uint32_t x = 0;
			for (int32_t j = i->start; j < i->stop; ++j)
			{
				if (m_text[j] != '\t')
					x += m_charWidth;
				else
					x = alignUp(x + 4 * m_charWidth, 4 * m_charWidth);
			}
			lineWidth = std::max< uint32_t >(lineWidth, x);
		}
	}

	m_scrollBarV->setRange(lineCount + pageLines);
	m_scrollBarV->setPage(pageLines);
	m_scrollBarV->setVisible(lineCount > pageLines);
	m_scrollBarV->update();

	if (lineWidth >= rc.getWidth() - m_lineMargin)
	{
		m_scrollBarH->setRange(lineWidth / c_scrollHSteps);
		m_scrollBarH->setPage(c_scrollHSteps);
		m_scrollBarH->setVisible(true);
	}
	else
		m_scrollBarH->setVisible(false);

	m_scrollBarH->update();
}

void RichEdit::deleteCharacters()
{
	int32_t start = m_caret;
	int32_t stop = m_caret;

	if (m_selectionStart >= 0)
	{
		start = m_selectionStart;
		stop = m_selectionStop - 1;
	}

	stop = std::min(stop, int32_t(m_text.size() - 2));
	if (stop < start)
		return;

	m_text.erase(m_text.begin() + start, m_text.begin() + stop + 1);
	m_meta.erase(m_meta.begin() + start, m_meta.begin() + stop + 1);

	for (std::vector< Line >::iterator i = m_lines.begin(); i != m_lines.end(); )
	{
		if (
			start == i->stop ||
			(start <= i->start && stop >= i->stop)
		)
		{
			int32_t tmp = i->start; i = m_lines.erase(i);
			if (i != m_lines.end())
				i->start = tmp;
		}
		else
			++i;
	}

	for (std::vector< Line >::iterator i = m_lines.begin(); i != m_lines.end(); ++i)
	{
		bool startIn = (start >= i->start && start <= i->stop);
		bool stopIn = (stop >= i->start && stop <= i->stop);

		if (startIn && stopIn)
			i->stop -= (stop - start) + 1;
		else if (startIn)
			i->stop = start;
		else if (stopIn)
			i->start = stop;
		else if (stop <= i->start)
		{
			i->start -= (stop - start) + 1;
			i->stop -= (stop - start) + 1;
		}
	}

	m_selectionStart = -1;
	m_selectionStop = -1;
	m_caret = start;

	CHECK;

	ContentChangeEvent contentChangeEvent(this);
	raiseEvent(&contentChangeEvent);
}

void RichEdit::insertCharacter(wchar_t ch)
{
	if (ch == L'\n' || ch == L'\r')
	{
		if (m_selectionStart >= 0)
			deleteCharacters();

		insertAt(m_caret++, L'\n');

		ContentChangeEvent contentChangeEvent(this);
		raiseEvent(&contentChangeEvent);
	}
	else if (ch == '\t')
	{
		if (m_selectionStart >= 0)
		{
			int32_t indentFromLine = getLineFromOffset(m_selectionStart);
			int32_t indentToLine = getLineFromOffset(m_selectionStop - 1);

			if (indentFromLine <= indentToLine)
			{
				for (int32_t i = indentFromLine; i <= indentToLine; ++i)
				{
					int32_t offset = getLineOffset(i);
					insertAt(offset, L'\t');
				}

				m_selectionStart = getLineOffset(indentFromLine);
				m_selectionStop = getLineOffset(indentToLine) + getLineLength(indentToLine);

				m_caret++;

				ContentChangeEvent contentChangeEvent(this);
				raiseEvent(&contentChangeEvent);

				return;
			}
			else
				deleteCharacters();
		}

		insertAt(m_caret++, L'\t');

		ContentChangeEvent contentChangeEvent(this);
		raiseEvent(&contentChangeEvent);
	}
	else if (ch >= 32)
	{
		if (m_selectionStart >= 0)
			deleteCharacters();

		insertAt(m_caret++, ch);

		ContentChangeEvent contentChangeEvent(this);
		raiseEvent(&contentChangeEvent);
	}
}

void RichEdit::insertAt(int32_t offset, wchar_t ch)
{
	m_text.insert(m_text.begin() + offset, ch);
	m_meta.insert(m_meta.begin() + offset, 0);

	if (ch == L'\n' || ch == L'\r')
	{
		for (std::vector< Line >::iterator i = m_lines.begin(); i != m_lines.end(); ++i)
		{
			if (offset >= i->start && offset <= i->stop)
			{
				Line line;
				line.start = i->start;
				line.stop = offset;
				i->start = offset + 1;
				i->stop++;
				i = m_lines.insert(i, line) + 1;
			}
			else if (i->start > offset)
			{
				i->start++;
				i->stop++;
			}
		}
	}
	else
	{
		for (std::vector< Line >::iterator i = m_lines.begin(); i != m_lines.end(); ++i)
		{
			if (offset >= i->start && offset <= i->stop)
				i->stop++;
			else if (offset < i->start)
			{
				i->start++;
				i->stop++;
			}
		}
	}
}

void RichEdit::scrollToCaret()
{
	if (m_scrollBarV->isVisible(false))
	{
		int32_t caretLine = getLineFromOffset(m_caret);

		Font font = getFont();
		Rect rc = getEditRect();

		int32_t lineHeight = font.getSize() + c_fontHeightMargin;
		int32_t pageLines = rc.getHeight() / lineHeight;

		int32_t top = m_scrollBarV->getPosition();

		if (caretLine < top)
		{
			m_scrollBarV->setPosition(caretLine);
			m_scrollBarV->update();
		}
		else if (caretLine >= top + pageLines)
		{
			m_scrollBarV->setPosition(caretLine - pageLines + 1);
			m_scrollBarV->update();
		}

		update();
	}
}

int32_t RichEdit::getCharacterStops(const std::wstring& text, std::vector< int32_t >& outStops) const
{
	int32_t x = 0;

	outStops.resize(0);
	outStops.reserve(text.length());

	std::wstring::const_iterator i0 = text.begin();
	for (std::wstring::const_iterator i = text.begin(); i != text.end(); ++i)
	{
		outStops.push_back(x);
		if (*i != '\t')
			x += m_charWidth;
		else
			x = alignUp(x + 4 * m_charWidth, 4 * m_charWidth);
	}

	return x;
}

Rect RichEdit::getEditRect() const
{
	Rect rc = getInnerRect();
	if (m_scrollBarV->isVisible(false))
		rc.right -= m_scrollBarV->getPreferedSize().cx;
	if (m_scrollBarH->isVisible(false))
		rc.bottom -= m_scrollBarH->getPreferedSize().cy;
	return rc;
}

void RichEdit::eventKeyDown(KeyDownEvent* event)
{
	int32_t caret = m_caret;
	bool caretMovement = false;

	switch (event->getVirtualKey())
	{
	case VkUp:
		// Move caret up.
		for (uint32_t i = 1; i < m_lines.size(); ++i)
		{
			if (m_caret >= m_lines[i].start && m_caret <= m_lines[i].stop)
			{
				int32_t offset = m_caret - m_lines[i].start;
				offset = std::min(offset, m_lines[i - 1].stop - m_lines[i - 1].start);
				m_caret = m_lines[i - 1].start + offset;
				break;
			}
		}
		caretMovement = true;
		break;

	case VkDown:
		// Move caret down.
		for (uint32_t i = 0; i < m_lines.size() - 1; ++i)
		{
			if (m_caret >= m_lines[i].start && m_caret <= m_lines[i].stop)
			{
				int32_t offset = m_caret - m_lines[i].start;
				offset = std::min(offset, m_lines[i + 1].stop - m_lines[i + 1].start);
				m_caret = m_lines[i + 1].start + offset;
				break;
			}
		}
		caretMovement = true;
		break;

	case VkLeft:
		// Move caret left.
		if ((event->getKeyState() & KsControl) == 0)
		{
			if (m_caret > 0)
				--m_caret;
		}
		else
		{
			if (m_caret > 0)
				--m_caret;

			while (m_caret > 0)
			{
				if (isWordSeparator(m_text[m_caret - 1]))
					break;
				--m_caret;
			}
		}
		caretMovement = true;
		break;

	case VkRight:
		// Move caret right.
		if ((event->getKeyState() & KsControl) == 0)
		{
			if (m_caret < int32_t(m_text.size()) - 1)
				++m_caret;
		}
		else
		{
			while (m_caret < int32_t(m_text.size()) - 1)
			{
				++m_caret;
				if (isWordSeparator(m_text[m_caret]))
					break;
			}
		}
		caretMovement = true;
		break;

	case VkHome:
		{
			if ((event->getKeyState() & KsControl) == 0)
			{
				for (std::vector< Line >::iterator i = m_lines.begin(); i != m_lines.end(); ++i)
				{
					if (m_caret > i->start && m_caret <= i->stop)
					{
						m_caret = i->start;
						break;
					}
				}
			}
			else
				m_caret = 0;

			caretMovement = true;
		}
		break;

	case VkEnd:
		{
			if ((event->getKeyState() & KsControl) == 0)
			{
				for (std::vector< Line >::iterator i = m_lines.begin(); i != m_lines.end(); ++i)
				{
					if (m_caret >= i->start && m_caret < i->stop)
					{
						m_caret = i->stop;
						break;
					}
				}
			}
			else
				m_caret = m_lines.back().stop;

			caretMovement = true;
		}
		break;

	case VkPageUp:
		// Move caret one page up.
		{
			Font font = getFont();
			Rect rc = getEditRect();

			int32_t lineHeight = font.getSize() + c_fontHeightMargin;
			int32_t pageLines = (rc.getHeight() + lineHeight - 1) / lineHeight;

			for (int32_t i = 1; i < int32_t(m_lines.size()); ++i)
			{
				if (m_caret >= m_lines[i].start && m_caret <= m_lines[i].stop)
				{
					int32_t offset = m_caret - m_lines[i].start;
					int32_t di = std::min(pageLines, i);
					offset = std::min(offset, m_lines[i - di].stop - m_lines[i - di].start);
					m_caret = m_lines[i - di].start + offset;
					break;
				}
			}
		}
		caretMovement = true;
		break;

	case VkPageDown:
		// Move caret one page down.
		{
			Font font = getFont();
			Rect rc = getEditRect();

			int32_t lineHeight = font.getSize() + c_fontHeightMargin;
			int32_t pageLines = (rc.getHeight() + lineHeight - 1) / lineHeight;

			for (int32_t i = 0; i < int32_t(m_lines.size()) - 1; ++i)
			{
				if (m_caret >= m_lines[i].start && m_caret <= m_lines[i].stop)
				{
					int32_t offset = m_caret - m_lines[i].start;
					int32_t di = std::min< uint32_t >(pageLines, m_lines.size() - 1 - i);
					offset = std::min(offset, m_lines[i + di].stop - m_lines[i + di].start);
					m_caret = m_lines[i + di].start + offset;
					break;
				}
			}
		}
		caretMovement = true;
		break;

	case VkBackSpace:
		{
			if (m_selectionStart < 0 && m_caret > 0)
			{
				m_caret--;
				deleteCharacters();
			}
			else if (m_selectionStart >= 0)
				deleteCharacters();
		}
		break;

	case VkDelete:
		deleteCharacters();
		break;

	case VkShift:
		return;

	default:
		break;
	}

	// If caret moved while holding "shift" key then expand selection range.
	if (caretMovement && (event->getKeyState() & KsShift) != 0)
	{
		if (m_selectionStart < 0)
		{
			m_selectionStart = std::min(caret, m_caret);
			m_selectionStop = std::max(caret, m_caret);
		}
		else
		{
			bool caretAtSelectionHead = bool(caret == m_selectionStart);
			bool caretAtSelectionTail = bool(caret == m_selectionStop);

			if (caretAtSelectionHead && !caretAtSelectionTail)
				m_selectionStart = m_caret;
			else if (!caretAtSelectionHead && caretAtSelectionTail)
				m_selectionStop = m_caret;
			else
			{
				m_selectionStart = std::min(m_selectionStart, m_caret);
				m_selectionStop = std::max(m_selectionStop, m_caret);
			}

			if (m_selectionStart > m_selectionStop)
				std::swap(m_selectionStart, m_selectionStop);
		}
	}
	// If caret moved but no "shift" key held, discard selection.
	else if (caretMovement)
	{
		m_selectionStart =
		m_selectionStop = -1;
	}

	updateScrollBars();
	scrollToCaret();
	update();
}

void RichEdit::eventKey(KeyEvent* event)
{
	wchar_t ch = event->getCharacter();
	if (ch == 3)
		copy();
	else if (ch == 22)
		paste();
	else if (ch == 24)
	{
		copy();
		deleteCharacters();
	}
	else if (ch != 8)
		insertCharacter(ch);

	CHECK;

	updateScrollBars();
	scrollToCaret();
	update();
}

void RichEdit::eventButtonDown(MouseButtonDownEvent* event)
{
	Point mousePosition = event->getPosition();

	Font font = getFont();
	Rect rc = getEditRect();

	uint32_t lineCount = m_lines.size();
	uint32_t lineOffset = m_scrollBarV->getPosition();
	uint32_t lineHeight = font.getSize() + c_fontHeightMargin;
	uint32_t pageLines = (rc.getHeight() + lineHeight - 1) / lineHeight;

	uint32_t line = lineOffset + mousePosition.y / lineHeight;
	if (line >= lineCount)
		return;

	const Line& ln = m_lines[line];
	std::wstring text(&m_text[ln.start], &m_text[ln.stop]);

	std::vector< int32_t > stops;
	int32_t lineWidth = getCharacterStops(text, stops);

	int32_t linePosition = mousePosition.x - m_lineMargin + m_lineOffsetH;
	if (linePosition < lineWidth)
	{
		for (int32_t i = stops.size() - 1; i >= 0; --i)
		{
			if (linePosition >= stops[i])
			{
				m_caret = ln.start + i;
				break;
			}
		}
	}
	else
		m_caret = ln.stop;

	// Remove selection.
	m_selectionStart = -1;
	m_selectionStop = -1;

	update();
}

void RichEdit::eventMouseWheel(MouseWheelEvent* event)
{
	if (m_scrollBarV->isVisible(false))
	{
		int32_t position = m_scrollBarV->getPosition();
		position -= event->getRotation() * 4;
		m_scrollBarV->setPosition(position);
		m_scrollBarV->update();
		update();
	}
}

void RichEdit::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();

	Font font = getFont();
	Rect rc = getInnerRect();

	// Cache font character width.
	int32_t charWidth = canvas.getTextExtent(L" ").cx;
	if (charWidth != m_charWidth)
	{
		m_charWidth = charWidth;
		updateScrollBars();
	}

	// Clear entire background.
	canvas.setBackground(Color4ub(255, 255, 255));
	canvas.fillRect(rc);

	// Adjust client area if scrollbars are visible.
	if (m_scrollBarV->isVisible(false))
		rc.right -= m_scrollBarV->getPreferedSize().cx;
	if (m_scrollBarH->isVisible(false))
		rc.bottom -= m_scrollBarH->getPreferedSize().cy;

	uint32_t lineCount = m_lines.size();
	uint32_t lineOffset = m_scrollBarV->getPosition();
	uint32_t lineHeight = abs(font.getSize()) + c_fontHeightMargin;
	uint32_t pageLines = (rc.getHeight() + lineHeight - 1) / lineHeight;

	// Calculate margin width from highest visible line number.
	m_lineMargin = c_iconSize + canvas.getTextExtent(toString(lineOffset + pageLines)).cx + 2;
	m_lineOffsetH = m_scrollBarH->getPosition() * c_scrollHSteps;

	// Background
	{
		Rect marginRc(rc.left, rc.top, rc.left + m_lineMargin, rc.bottom);
		Rect iconsRc(rc.left, rc.top, rc.left + c_iconSize, rc.top + lineHeight);
		Rect lineRc(rc.left + c_iconSize, rc.top, rc.left + m_lineMargin, rc.top + lineHeight);

		canvas.setForeground(Color4ub(180, 180, 180));
		canvas.setBackground(Color4ub(200, 200, 200));
		canvas.fillGradientRect(marginRc, false);

		canvas.setForeground(Color4ub(0, 0, 0));
		for (uint32_t i = lineOffset; i < lineOffset + pageLines && i < lineCount; ++i)
		{
			canvas.drawText(lineRc, toString(i + 1), AnLeft, AnCenter);

			if (m_lines[i].image >= 0)
			{
				canvas.drawBitmap(
					Point(iconsRc.left, iconsRc.top + (iconsRc.getHeight() - m_imageHeight) / 2),
					Point(m_lines[i].image * m_imageWidth, 0),
					Size(m_imageWidth, m_imageHeight),
					m_image,
					BmAlpha
				);
			}

			iconsRc = iconsRc.offset(0, lineHeight);
			lineRc = lineRc.offset(0, lineHeight);
		}
	}

	// Formatted text.
	{
		canvas.setClipRect(Rect(
			rc.left + m_lineMargin,
			rc.top,
			rc.right,
			rc.bottom
		));

		Rect lineRc(rc.left, rc.top, rc.right, rc.top + lineHeight);
		uint32_t lineOffsetEnd = std::min(lineOffset + pageLines, lineCount);
		for (uint32_t i = lineOffset; i < lineOffsetEnd; ++i)
		{
			const Line& line = m_lines[i];

			Rect textRc = lineRc;
			uint32_t x = 0;

			// Non-empty line; format print.
			for (int32_t j = line.start; j < line.stop; ++j)
			{
				const Attribute& attrib = m_attributes[m_meta[j]];

				if (m_caret == j)
				{
					textRc.left = m_lineMargin + 2 + x - 1 - m_lineOffsetH;
					textRc.right = textRc.left + 1;

					canvas.setBackground(Color4ub(0, 0, 0));
					canvas.fillRect(textRc);
				}

				bool solidBackground = false;
				if (j >= m_selectionStart && j < m_selectionStop)
				{
					canvas.setBackground(Color4ub(51, 153, 255));
					canvas.setForeground(Color4ub(255, 255, 255));
					solidBackground = true;
				}
				else
				{
					canvas.setBackground(attrib.backColor);
					canvas.setForeground(attrib.textColor);
					solidBackground = bool(attrib.backColor != Color4ub(255, 255, 255));
				}

				if (m_text[j] != '\t')
				{
					textRc.left = m_lineMargin + 2 + x - m_lineOffsetH;
					textRc.right = textRc.left + m_charWidth;

					if (solidBackground)
						canvas.fillRect(textRc);

					std::wstring ch(&m_text[j], &m_text[j + 1]);
					canvas.drawText(textRc, ch, AnLeft, AnCenter);

					x += m_charWidth;
				}
				else
				{
					// Adjust offset to nearest tab-stop.
					int32_t nx = alignUp(x + 4 * m_charWidth, 4 * m_charWidth);

					textRc.left = m_lineMargin + 2 + x - m_lineOffsetH;
					textRc.right = m_lineMargin + 2 + nx - m_lineOffsetH;

					if (solidBackground)
						canvas.fillRect(textRc);

					x = nx;
				}
			}

			// Special condition; caret at the very end of a line.
			if (m_caret == line.stop)
			{
				textRc.left = m_lineMargin + 2 + x - 1;
				textRc.right = textRc.left + 1;

				canvas.setBackground(Color4ub(0, 0, 0));
				canvas.fillRect(textRc);
			}

			lineRc = lineRc.offset(0, lineHeight);
		}
	}

	event->consume();
}

void RichEdit::eventSize(SizeEvent* event)
{
	int32_t width = m_scrollBarV->getPreferedSize().cx;
	int32_t height = m_scrollBarH->getPreferedSize().cy;

	Rect inner = getInnerRect();

	Rect rcV(Point(inner.getWidth() - width, 0), Size(width, inner.getHeight() - height));
	m_scrollBarV->setRect(rcV);

	Rect rcH(Point(0, inner.getHeight() - height), Size(inner.getWidth() - width, height));
	m_scrollBarH->setRect(rcH);

	updateScrollBars();
}

void RichEdit::eventScroll(ScrollEvent* event)
{
	update();
}

#if defined(_DEBUG)
void RichEdit::checkConsistency()
{
	if (m_lines.empty())
	{
		T_ASSERT (m_text.empty());
		T_ASSERT (m_meta.empty());
		return;
	}

	T_ASSERT (m_text.size() == m_meta.size());

	T_ASSERT (m_lines[0].start == 0);
	for (uint32_t i = 0; i < m_lines.size() - 1; ++i)
	{
		T_ASSERT (m_lines[i].stop + 1 == m_lines[i + 1].start);

		std::wstring text(&m_text[m_lines[i].start], &m_text[m_lines[i].stop]);
		T_ASSERT (text.find('\n') == text.npos);
		T_ASSERT (text.find('\r') == text.npos);
	}

	T_ASSERT (m_lines.back().stop == m_text.size() - 1);

	for (std::vector< uint16_t >::const_iterator i = m_meta.begin(); i != m_meta.end(); ++i)
		T_ASSERT (*i < m_attributes.size());

	// Save "good" lines so we can spot differences if
	// next fail.
	m_linesLastGood = m_lines;
}
#endif

		}
	}
}
