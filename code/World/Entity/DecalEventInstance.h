#pragma once

#include "World/IEntityEventInstance.h"

namespace traktor
{
	namespace world
	{

class ComponentEntity;
class DecalEvent;

/*! \brief
 * \ingroup World
 */
class DecalEventInstance : public IEntityEventInstance
{
	T_RTTI_CLASS;

public:
	DecalEventInstance(const DecalEvent* event, const Transform& Toffset);

	virtual bool update(const UpdateParams& update) override final;

	virtual void build(IWorldRenderer* worldRenderer) override final;

	virtual void cancel(CancelType when) override final;

private:
	Ref< ComponentEntity > m_entity;
};

	}
}

