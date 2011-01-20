#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Settings/PropertyInteger.h"

namespace traktor
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.PropertyInteger", 0, PropertyInteger, IPropertyValue)

PropertyInteger::PropertyInteger(value_type_t value)
:	m_value(value)
{
}

PropertyInteger::value_type_t PropertyInteger::get(const IPropertyValue* value)
{
	return value ? checked_type_cast< const PropertyInteger* >(value)->m_value : false;
}

IPropertyValue* PropertyInteger::merge(IPropertyValue* right, bool join)
{
	return right;
}

bool PropertyInteger::serialize(ISerializer& s)
{
	return s >> Member< value_type_t >(L"value", m_value);
}

}
