#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Settings/PropertyColor.h"

namespace traktor
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.PropertyColor", 0, PropertyColor, IPropertyValue)

PropertyColor::PropertyColor(const value_type_t& value)
:	m_value(value)
{
}

PropertyColor::value_type_t PropertyColor::get(const IPropertyValue* value)
{
	return value ? checked_type_cast< const PropertyColor* >(value)->m_value : value_type_t();
}

IPropertyValue* PropertyColor::merge(IPropertyValue* right, bool join)
{
	return right;
}

bool PropertyColor::serialize(ISerializer& s)
{
	return s >> Member< value_type_t >(L"value", m_value);
}

}
