#pragma once

#include "Amalgam/Game/IServer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_GAME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class ISoundPlayer;
class SoundSystem;
class SurroundEnvironment;

	}

	namespace amalgam
	{

/*! \brief Audio server.
 * \ingroup Amalgam
 *
 * "Audio.Type"					- Sound system type.
 * "Audio.MasterVolume"			- Master volume.
 * "Audio.Surround/MaxDistance"	- Surround sound max speaker distance.
 * "Audio.Surround/InnerRadius"	- Surround inner listener radius.
 * "Audio.Volumes/<Category>"   - Per category volume.
 */
class T_DLLCLASS IAudioServer : public IServer
{
	T_RTTI_CLASS;

public:
	virtual sound::SoundSystem* getSoundSystem() = 0;

	virtual sound::ISoundPlayer* getSoundPlayer() = 0;

	virtual sound::SurroundEnvironment* getSurroundEnvironment() = 0;
};

	}
}

