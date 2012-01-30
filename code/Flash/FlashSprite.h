#ifndef traktor_flash_FlashSprite_H
#define traktor_flash_FlashSprite_H

#include <map>
#include "Core/RefArray.h"
#include "Flash/FlashCharacter.h"
#include "Flash/SwfTypes.h"

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

class ActionContext;
class FlashFrame;
class IActionVM;
class IActionVMImage;

/*! \brief Flash sprite.
 * \ingroup Flash
 */
class T_DLLCLASS FlashSprite : public FlashCharacter
{
	T_RTTI_CLASS;

public:
	FlashSprite();

	FlashSprite(uint16_t id, uint16_t frameRate);

	uint16_t getFrameRate() const;

	void addFrame(FlashFrame* frame);

	uint32_t getFrameCount() const;

	FlashFrame* getFrame(uint32_t frameId) const;

	int findFrame(const std::string& frameLabel) const;

	void addInitActionScript(const IActionVMImage* initActionScript);

	const RefArray< const IActionVMImage >& getInitActionScripts() const;

	virtual Ref< FlashCharacterInstance > createInstance(ActionContext* context, FlashCharacterInstance* parent, const std::string& name, const ActionObject* initObject) const;

	virtual bool serialize(ISerializer& s);

private:
	uint16_t m_frameRate;
	RefArray< FlashFrame > m_frames;
	RefArray< const IActionVMImage > m_initActionScripts;
};

	}
}

#endif	// traktor_flash_FlashSprite_H
