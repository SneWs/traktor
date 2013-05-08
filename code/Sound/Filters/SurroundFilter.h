#ifndef traktor_sound_SurroundFilter_H
#define traktor_sound_SurroundFilter_H

#include "Sound/IFilter.h"
#include "Core/Math/Vector4.h"

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

class SurroundEnvironment;

/*! \brief Surround filter.
 * \ingroup Sound
 */
class T_DLLCLASS SurroundFilter : public IFilter
{
	T_RTTI_CLASS;

public:
	static Ref< SurroundFilter > create(SurroundEnvironment* environment, const Vector4& speakerPosition);

	SurroundFilter(SurroundEnvironment* environment, const Vector4& speakerPosition = Vector4::origo(), float maxDistance = 0.0f);

	void setSpeakerPosition(const Vector4& speakerPosition);

	void setMaxDistance(float maxDistance);

	virtual Ref< IFilterInstance > createInstance() const;

	virtual void apply(IFilterInstance* instance, SoundBlock& outBlock) const;

	virtual void serialize(ISerializer& s);

private:
	Ref< SurroundEnvironment > m_environment;
	Vector4 m_speakerPosition;
	Scalar m_maxDistance;

	void applyStereo(IFilterInstance* instance, SoundBlock& outBlock) const;

	void applyFull(IFilterInstance* instance, SoundBlock& outBlock) const;
};

	}
}

#endif	// traktor_sound_SurroundFilter_H
