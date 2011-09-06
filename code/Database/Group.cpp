#include <algorithm>
#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Database/Provider/IProviderGroup.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.Group", Group, Object)

std::wstring Group::getName() const
{
	T_ASSERT (m_providerGroup);
	return m_name;
}

std::wstring Group::getPath() const
{
	T_ASSERT (m_providerGroup);
	
	if (!m_parent)
		return L"";

	std::wstring path = m_parent->getPath();

	if (!path.empty())
		path += L"/" + m_name;
	else
		path = m_name;

	return path;
}

bool Group::rename(const std::wstring& name)
{
	T_ASSERT (m_providerGroup);
	
	if (!m_providerGroup->rename(name))
		return false;

	m_name = name;
	return true;
}

bool Group::remove()
{
	T_ASSERT (m_providerGroup);

	if (!m_childInstances.empty() || !m_childGroups.empty())
		return false;

	if (!m_providerGroup->remove())
		return false;

	internalDestroy();
	return true;
}

Ref< Group > Group::getGroup(const std::wstring& groupName)
{
	T_ASSERT (m_providerGroup);
	return findChildGroup(this, FindGroupByName(groupName));
}

Ref< Group > Group::createGroup(const std::wstring& groupName)
{
	T_ASSERT (m_providerGroup);

	Ref< Group > group = getGroup(groupName);
	if (group)
		return group; 

	Ref< IProviderGroup > providerGroup = m_providerGroup->createGroup(groupName);
	if (!providerGroup)
		return 0;

	group = new Group(m_eventListener);
	if (!group->internalCreate(providerGroup, this))
		return 0;

	m_childGroups.push_back(group);
	return group;
}

Ref< Instance > Group::getInstance(const std::wstring& instanceName, const TypeInfo* primaryType)
{
	T_ASSERT (m_providerGroup);
	if (!primaryType)
		return findChildInstance(this, FindInstanceByName(instanceName));
	else
	{
		Ref< Instance > instance = findChildInstance(this, FindInstanceByName(instanceName));
		if (!instance || !is_type_of(*primaryType, *instance->getPrimaryType()))
			return 0;
		return instance;
	}
}

Ref< Instance > Group::createInstance(const std::wstring& instanceName, uint32_t flags, const Guid* guid)
{
	T_ASSERT (m_providerGroup);
	Ref< Instance > instance;

	// Create instance guid, use given if available.
	Guid instanceGuid = guid ? *guid : Guid::create();
	if (instanceGuid.isNull() || !instanceGuid.isValid())
	{
		log::error << L"Not allowed to create instance with invalid guid" << Endl;
		return 0;
	}

	// Remove existing instance if we're about to replace it.
	if (flags & CifReplaceExisting)
	{
		instance = getInstance(instanceName);
		if (instance)
		{
			if (!instance->checkout())
				return 0;

			if (!(flags & CifKeepExistingGuid))
			{
				if (!instance->setGuid(instanceGuid))
					return 0;
			}

			if (!instance->removeAllData())
				return 0;

			return instance;
		}
	}

	// Create provider instance.
	Ref< IProviderInstance > providerInstance = m_providerGroup->createInstance(instanceName, instanceGuid);
	if (!providerInstance)
		return 0;

	// Create instance object.
	if (!instance)
		instance = new Instance(this);

	if (!instance->internalCreateNew(providerInstance, this))
		return 0;

	return instance;
}

Ref< Group > Group::getParent()
{
	T_ASSERT (m_providerGroup);
	return m_parent;
}

bool Group::getChildGroups(RefArray< Group >& outChildGroups)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	outChildGroups = m_childGroups;
	return true;
}

bool Group::getChildInstances(RefArray< Instance >& outChildInstances)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	outChildInstances = m_childInstances;
	return true;
}

Group::Group(IInstanceEventListener* eventListener)
:	m_eventListener(eventListener)
,	m_parent(0)
{
}

bool Group::internalCreate(IProviderGroup* providerGroup, Group* parent)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	m_providerGroup = providerGroup;
	m_parent = parent;
	m_name = m_providerGroup->getName();
	
	m_childGroups.resize(0);
	m_childInstances.resize(0);

	RefArray< IProviderGroup > providerChildGroups;
	m_providerGroup->getChildGroups(providerChildGroups);

	m_childGroups.reserve(providerChildGroups.size());
	for (RefArray< IProviderGroup >::iterator i = providerChildGroups.begin(); i != providerChildGroups.end(); ++i)
	{
		Ref< Group > childGroup = new Group(m_eventListener);
		if (!childGroup->internalCreate(*i, this))
			return false;

		m_childGroups.push_back(childGroup);
	}

	RefArray< IProviderInstance > providerChildInstances;
	m_providerGroup->getChildInstances(providerChildInstances);

	m_childInstances.reserve(providerChildInstances.size());
	for (RefArray< IProviderInstance >::iterator i = providerChildInstances.begin(); i != providerChildInstances.end(); ++i)
	{
		Ref< Instance > childInstance = new Instance(this);
		if (!childInstance->internalCreateExisting(*i, this))
			return false;

		m_childInstances.push_back(childInstance);
	}

	return true;
}

void Group::internalDestroy()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	m_eventListener = 0;
	m_providerGroup = 0;
	m_parent = 0;
	m_name = L"";

	for (RefArray< Group >::iterator i = m_childGroups.begin(); i != m_childGroups.end(); ++i)
		(*i)->internalDestroy();

	m_childGroups.resize(0);

	for (RefArray< Instance >::iterator i = m_childInstances.begin(); i != m_childInstances.end(); ++i)
		(*i)->internalDestroy();

	m_childInstances.resize(0);
}

void Group::instanceEventCreated(Instance* instance)
{
	// Add child instance.
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		m_childInstances.push_back(instance);
	}
	m_eventListener->instanceEventCreated(instance);
}

void Group::instanceEventRemoved(Instance* instance)
{
	// Remove child instance from list.
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		RefArray< Instance >::iterator i = std::find(m_childInstances.begin(), m_childInstances.end(), instance);
		T_ASSERT (i != m_childInstances.end());
		m_childInstances.erase(i);
	}
	m_eventListener->instanceEventRemoved(instance);
}

void Group::instanceEventGuidChanged(Instance* instance, const Guid& previousGuid)
{
	m_eventListener->instanceEventGuidChanged(instance, previousGuid);
}

void Group::instanceEventRenamed(Instance* instance, const std::wstring& previousName)
{
	m_eventListener->instanceEventRenamed(instance, previousName);
}

void Group::instanceEventCommitted(Instance* instance)
{
	m_eventListener->instanceEventCommitted(instance);
}

	}
}
