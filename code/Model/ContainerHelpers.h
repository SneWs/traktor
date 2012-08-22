#ifndef traktor_model_ContainerHelpers_H
#define traktor_model_ContainerHelpers_H

#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"
#include "Model/Material.h"

namespace traktor
{
	namespace model
	{

//@{
// \ingroup Model

template < typename ContainerType, typename ItemType >
int addId(ContainerType& v, ItemType item)
{
	v.push_back(item);
	return int(v.size() - 1);
}

template < typename ContainerType, typename ItemType, typename ItemPredicate >
int addUniqueId(ContainerType& v, ItemType item, const ItemPredicate& predicate = ItemPredicate())
{
	for (int i = 0; i < int(v.size()); ++i)
	{
		if (predicate(v[i], item))
			return i;
	}
	return addId< ContainerType, ItemType >(v, item);
}

template < typename ItemType >
struct DefaultPredicate
{
	inline bool operator () (const ItemType& a, const ItemType& b) const
	{
		return a == b;
	}
};

struct MaterialNamePredicate
{
	inline bool operator () (const Material& a, const Material& b) const
	{
		return a.getName() == b.getName();
	}
};

struct MaterialPredicate
{
	inline bool operator () (const Material& a, const Material& b) const
	{
		return
			a.getDiffuseMap() == b.getDiffuseMap() &&
			a.getSpecularMap() == b.getSpecularMap() &&
			a.getEmissiveMap() == b.getEmissiveMap() &&
			a.getReflectiveMap() == b.getReflectiveMap() &&
			a.getNormalMap() == b.getNormalMap() &&
			a.getColor() == b.getColor() &&
			a.getDiffuseTerm() == b.getDiffuseTerm() &&
			a.getSpecularTerm() == b.getSpecularTerm() &&
			a.getSpecularRoughness() == b.getSpecularRoughness() &&
			a.getEmissive() == b.getEmissive() &&
			a.getReflective() == b.getReflective() &&
			a.getRimLightIntensity() == b.getRimLightIntensity() &&
			a.getBlendOperator() == b.getBlendOperator() &&
			a.isDoubleSided() == b.isDoubleSided();
	}
};

//@}

	}
}

#endif	// traktor_model_ContainerHelpers_H
