#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class Entity;
class EntityData;
class IEntityComponent;
class IEntityComponentData;
class IEntityEvent;
class IEntityEventData;
class IEntityFactory;

/*! \brief Entity builder interface.
 * \ingroup World
 */
class T_DLLCLASS IEntityBuilder : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Add entity factory to builder.
	 *
	 * \param entityFactory Concrete entity factory to add.
	 */
	virtual void addFactory(const IEntityFactory* entityFactory) = 0;

	/*! \brief Remove entity factory from builder.
	 *
	 * \param entityFactory Concrete entity factory to remove.
	 */
	virtual void removeFactory(const IEntityFactory* entityFactory) = 0;

	/*! \brief Get concrete entity factory which can create entity from given data.
	 *
	 * \param entityData Entity data.
	 * \return Concrete entity factory, null if no factory found.
	 */
	virtual const IEntityFactory* getFactory(const EntityData* entityData) const = 0;

	/*! \brief Get concrete entity factory which can create event from given data.
	 *
	 * \param entityEventData Entity event data.
	 * \return Concrete entity factory, null if no factory found.
	 */
	virtual const IEntityFactory* getFactory(const IEntityEventData* entityEventData) const = 0;

	/*! \brief Get concrete entity factory which can create component from given data.
	 *
	 * \param entityComponentData Entity component data.
	 * \return Concrete entity factory, null if no factory found.
	 */
	virtual const IEntityFactory* getFactory(const IEntityComponentData* entityComponentData) const = 0;

	/*! \brief Create entity from entity data.
	 *
	 * \param entityData Entity data.
	 * \return Concrete entity, null if unable to create entity.
	 */
	virtual Ref< Entity > create(const EntityData* entityData) const = 0;

	/*! \brief Create event from entity event data.
	 *
	 * \param entityEventData Entity event data.
	 * \return Entity event, null if unable to create event.
	 */
	virtual Ref< IEntityEvent > create(const IEntityEventData* entityEventData) const = 0;

	/*! \brief Create component from component data.
	 *
	 * \param entityComponentData Entity component data.
	 * \return Component instance, null if unable to create instance.
	 */
	virtual Ref< IEntityComponent > create(const IEntityComponentData* entityComponentData) const = 0;

	/*! \brief Get composite entity builder.
	 *
	 * \return Pointer to composite entity builder.
	 */
	virtual const IEntityBuilder* getCompositeEntityBuilder() const = 0;

	/*! \brief
	 */
	template < typename EntityType >
	Ref< EntityType > create(const EntityData* entityData) const
	{
		return checked_type_cast< EntityType*, true >(create(entityData));
	}

	/*! \brief
	 */
	template < typename EntityEventType >
	Ref< EntityEventType > create(const IEntityEventData* entityEventData) const
	{
		return checked_type_cast< EntityEventType*, true >(create(entityEventData));
	}

	/*! \brief
	 */
	template < typename EntityComponentType >
	Ref< EntityComponentType > create(const IEntityComponentData* entityComponentData) const
	{
		return checked_type_cast< EntityComponentType*, true >(create(entityComponentData));
	}
};

	}
}

