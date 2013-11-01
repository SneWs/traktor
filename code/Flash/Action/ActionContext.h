#ifndef traktor_flash_ActionContext_H
#define traktor_flash_ActionContext_H

#include "Flash/Collectable.h"
#include "Flash/Action/ActionStrings.h"
#include "Flash/Action/ActionValue.h"
#include "Flash/Action/ActionValuePool.h"

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

class FlashDictionary;
class FlashMovie;
class FlashSpriteInstance;
class IActionVM;
class IFlashMovieLoader;

/*! \brief ActionScript execution context.
 * \ingroup Flash
 */
class T_DLLCLASS ActionContext : public Collectable
{
	T_RTTI_CLASS;

public:
	// Predefined strings.
	enum StringIds
	{
		IdEmpty = 0,
		Id__proto__ = 1,
		IdPrototype = 2,
		Id__ctor__ = 3,
		IdConstructor = 4,
		IdThis = 5,
		IdSuper = 6,
		IdGlobal = 7,
		IdArguments = 8,
		IdRoot = 9,
		IdParent = 10,
		IdApply = 11,
		IdCall = 12,
		IdToString = 13,
		IdTarget = 14,
		IdTargetProperty = 15,
		IdFunction = 16,
		IdOnLoad = 17,
		IdOnEnterFrame = 18,
		IdOnKeyDown = 19,
		IdOnKeyUp = 20,
		IdOnMouseDown = 21,
		IdOnPress = 22,
		IdOnMouseUp = 23,
		IdOnRelease = 24,
		IdOnMouseMove = 25,
		IdOnRollOver = 26,
		IdOnRollOut = 27,
		IdOnFrame = 28,
		IdOnSetFocus = 29,
		IdOnKillFocus = 30,
		IdOnMouseWheel = 31,
		IdOnChanged = 32,
		IdOnScroller = 33
	};

	ActionContext(const IActionVM* vm, const FlashMovie* movie, const IFlashMovieLoader* movieLoader, FlashDictionary* dictionary);

	void setGlobal(ActionObject* global);

	void setMovieClip(FlashSpriteInstance* movieClip);

	void addFrameListener(ActionObject* frameListener);

	void removeFrameListener(ActionObject* frameListener);

	void notifyFrameListeners(avm_number_t time);

	ActionObject* lookupClass(const std::string& className);

	uint32_t getString(const std::string& str);

	std::string getString(uint32_t id);

	const IActionVM* getVM() const { return m_vm; }

	const FlashMovie* getMovie() const { return m_movie; }

	const IFlashMovieLoader* getMovieLoader() const { return m_movieLoader; }

	FlashDictionary* getDictionary() const { return m_dictionary; }

	ActionObject* getGlobal() const { return m_global; }

	FlashSpriteInstance* getMovieClip() const { return m_movieClip; }

	ActionValuePool& getPool() { return m_pool; }

protected:
	virtual void trace(const IVisitor& visitor) const;

	virtual void dereference();

private:
	struct FrameListener
	{
		Ref< ActionObject > listenerTarget;
		Ref< ActionFunction > listenerFunction;
	};

	const IActionVM* m_vm;
	const FlashMovie* m_movie;
	Ref< const IFlashMovieLoader > m_movieLoader;
	Ref< FlashDictionary > m_dictionary;
	Ref< ActionObject > m_global;
	Ref< FlashSpriteInstance > m_movieClip;
	ActionStrings m_strings;
	ActionValuePool m_pool;
	std::vector< FrameListener > m_frameListeners;
};

	}
}

#endif	// traktor_flash_ActionContext_H
