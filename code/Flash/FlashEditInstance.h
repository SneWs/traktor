#ifndef traktor_flash_FlashEditInstance_H
#define traktor_flash_FlashEditInstance_H

#include <list>
#include "Core/Thread/Semaphore.h"
#include "Flash/FlashCharacterInstance.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class FlashEdit;

/*! \brief Dynamic text character instance.
 * \ingroup Flash
 */
class T_DLLCLASS FlashEditInstance : public FlashCharacterInstance
{
	T_RTTI_CLASS;

public:
	typedef std::list< std::wstring > text_t;

	FlashEditInstance(ActionContext* context, FlashCharacterInstance* parent, const FlashEdit* edit, const std::wstring& html);

	const FlashEdit* getEdit() const;

	bool parseText(const std::wstring& text);

	bool parseHtml(const std::wstring& html);

	const SwfColor& getTextColor() const;

	void setTextColor(const SwfColor& textColor);

	text_t getText() const;

	std::wstring getConcatedText() const;

	bool getTextExtents(float& outWidth, float& outHeight) const;

	virtual SwfRect getBounds() const;

private:
	mutable Semaphore m_lock;
	Ref< const FlashEdit > m_edit;
	SwfColor m_textColor;
	text_t m_text;
};

	}
}

#endif	// traktor_flash_FlashEditInstance_H
