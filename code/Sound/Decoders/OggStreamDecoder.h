#pragma once

#include "Sound/IStreamDecoder.h"

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

class OggStreamDecoderImpl;

/*! \brief OGG stream decoder.
 * \ingroup Sound
 */
class T_DLLCLASS OggStreamDecoder : public IStreamDecoder
{
	T_RTTI_CLASS;

public:
	virtual bool create(IStream* stream) override final;

	virtual void destroy() override final;

	virtual double getDuration() const override final;

	virtual bool getBlock(SoundBlock& outSoundBlock) override final;

	virtual void rewind() override final;

private:
	Ref< IStream > m_stream;
	Ref< OggStreamDecoderImpl > m_decoderImpl;
};

	}
}

