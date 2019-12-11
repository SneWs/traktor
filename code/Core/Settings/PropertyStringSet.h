#pragma once

#include <set>
#include "Core/Settings/IPropertyValue.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! String set property value.
 * \ingroup Core
 */
class T_DLLCLASS PropertyStringSet : public IPropertyValue
{
	T_RTTI_CLASS;

public:
	typedef std::set< std::wstring > value_type_t;

	PropertyStringSet(const value_type_t& value = value_type_t());

	static value_type_t get(const IPropertyValue* value);

	virtual void serialize(ISerializer& s) override final;

protected:
	virtual Ref< IPropertyValue > join(const IPropertyValue* right) const override final;

	virtual Ref< IPropertyValue > clone() const override final;

private:
	value_type_t m_value;
};

/*!
 * \ingroup Core
 */
template< >
struct PropertyTrait< std::set< std::wstring > >
{
	typedef PropertyStringSet property_type_t;
	typedef const std::set< std::wstring >& default_value_type_t;
	typedef std::set< std::wstring > return_type_t;
};

}

