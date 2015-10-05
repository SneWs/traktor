#ifndef traktor_sound_TriggerGrain_H
#define traktor_sound_TriggerGrain_H

#include "Core/Ref.h"
#include "Core/Math/Random.h"
#include "Sound/Resound/IGrain.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

/*! \brief
 * \ingroup Sound
 */
class T_DLLCLASS TriggerGrain : public IGrain
{
	T_RTTI_CLASS;

public:
	TriggerGrain(
		handle_t id,
		float position,
		float rate,
		IGrain* grain
	);

	virtual Ref< ISoundBufferCursor > createCursor() const T_OVERRIDE T_FINAL;

	virtual void updateCursor(ISoundBufferCursor* cursor) const T_OVERRIDE T_FINAL;

	virtual const IGrain* getCurrentGrain(const ISoundBufferCursor* cursor) const T_OVERRIDE T_FINAL;

	virtual void getActiveGrains(const ISoundBufferCursor* cursor, RefArray< const IGrain >& outActiveGrains) const T_OVERRIDE T_FINAL;

	virtual bool getBlock(ISoundBufferCursor* cursor, const ISoundMixer* mixer, SoundBlock& outBlock) const T_OVERRIDE T_FINAL;

private:
	handle_t m_id;
	float m_position;
	float m_rate;
	Ref< IGrain > m_grain;
};

	}
}

#endif	// traktor_sound_TriggerGrain_H
