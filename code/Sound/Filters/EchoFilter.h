#ifndef traktor_sound_EchoFilter_H
#define traktor_sound_EchoFilter_H

#include "Core/Math/Scalar.h"
#include "Sound/IFilter.h"

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

/*! \brief Echo filter.
 * \ingroup Sound
 */
class T_DLLCLASS EchoFilter : public IFilter
{
	T_RTTI_CLASS;

public:
	EchoFilter();

	EchoFilter(float delay, float decay, float wetMix, float dryMix);

	virtual Ref< IFilterInstance > createInstance() const;

	virtual void apply(IFilterInstance* instance, SoundBlock& outBlock) const;

	virtual void serialize(ISerializer& s);

private:
	float m_delay;
	float m_decay;
	Scalar m_wetMix;
	Scalar m_dryMix;
};

	}
}

#endif	// traktor_sound_EchoFilter_H
