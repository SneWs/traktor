#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Settings/PropertyGroup.h"

namespace traktor
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.PropertyGroup", 0, PropertyGroup, IPropertyValue)

PropertyGroup::PropertyGroup()
{
}

PropertyGroup::value_type_t PropertyGroup::get(const IPropertyValue* value)
{
	return value ? DeepClone(value).create< PropertyGroup >() : 0;
}

void PropertyGroup::setProperty(const std::wstring& propertyName, IPropertyValue* value)
{
	if (value)
		m_value[propertyName] = value;
	else
		m_value.erase(propertyName);
}

Ref< IPropertyValue > PropertyGroup::getProperty(const std::wstring& propertyName)
{
	std::map< std::wstring, Ref< IPropertyValue > >::const_iterator it = m_value.find(propertyName);
	return it != m_value.end() ? it->second.ptr() : 0;
}

Ref< const IPropertyValue > PropertyGroup::getProperty(const std::wstring& propertyName) const
{
	std::map< std::wstring, Ref< IPropertyValue > >::const_iterator it = m_value.find(propertyName);
	return it != m_value.end() ? it->second.ptr() : 0;
}

IPropertyValue* PropertyGroup::merge(IPropertyValue* right, bool join)
{
	if (join)
	{
		if (PropertyGroup* rightGroup = dynamic_type_cast< PropertyGroup* >(right))
		{
			const std::map< std::wstring, Ref< IPropertyValue > >& rightValues = rightGroup->getValues();
			for (std::map< std::wstring, Ref< IPropertyValue > >::const_iterator i = rightValues.begin(); i != rightValues.end(); ++i)
			{
				Ref< IPropertyValue > leftValue = getProperty(i->first);
				if (leftValue)
					setProperty(i->first, leftValue->merge(i->second, join));
				else
					setProperty(i->first, i->second);
			}
			return this;
		}
	}
	return right;
}

bool PropertyGroup::serialize(ISerializer& s)
{
	return s >> MemberStlMap<
		std::wstring,
		Ref< IPropertyValue >,
		MemberStlPair<
			std::wstring,
			Ref< IPropertyValue >,
			Member< std::wstring >,
			MemberRef< IPropertyValue >
		>
	>(L"value", m_value);
}

}
