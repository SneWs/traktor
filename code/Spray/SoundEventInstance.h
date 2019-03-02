#pragma once

#include "Core/Math/Transform.h"
#include "Resource/Proxy.h"
#include "World/IEntityEventInstance.h"

namespace traktor
{
	namespace sound
	{

class ISoundHandle;
class ISoundPlayer;
class Sound;

	}

	namespace world
	{

class Entity;

	}

	namespace spray
	{

class SoundEventInstance : public world::IEntityEventInstance
{
	T_RTTI_CLASS;

public:
	SoundEventInstance(
		world::Entity* sender,
		const Transform& Toffset,
		sound::ISoundPlayer* soundPlayer,
		const resource::Proxy< sound::Sound >& sound,
		bool positional,
		bool follow,
		bool autoStopFar
	);

	virtual bool update(const world::UpdateParams& update) override final;

	virtual void build(world::IWorldRenderer* worldRenderer) override final;

	virtual void cancel(world::CancelType when) override final;

private:
	Ref< world::Entity > m_sender;
	Transform m_Toffset;
	sound::ISoundPlayer* m_soundPlayer;
	resource::Proxy< sound::Sound > m_sound;
	bool m_positional;
	bool m_follow;
	bool m_autoStopFar;
	Ref< sound::ISoundHandle > m_handle;
};

	}
}

