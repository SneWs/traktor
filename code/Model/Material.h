#ifndef traktor_model_Material_H
#define traktor_model_Material_H

#include <string>
#include <vector>
#include "Core/Config.h"
#include "Core/Math/Color4ub.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MODEL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace model
	{

/*! \brief Material descriptor.
 * \ingroup Model
 */
class T_DLLCLASS Material
{
public:
	enum BlendOperator
	{
		BoDecal,
		BoAdd,
		BoMultiply,
		BoAlpha
	};

	Material();

	Material(const std::wstring& name);

	void setName(const std::wstring& name);
	
	const std::wstring& getName() const;

	void setDiffuseMap(const std::wstring& diffuseMap, BlendOperator diffuseBlendOperator);

	const std::wstring& getDiffuseMap() const;

	BlendOperator getDiffuseBlendOperator() const;

	void setSpecularMap(const std::wstring& specularMap);

	const std::wstring& getSpecularMap() const;

	void setNormalMap(const std::wstring& normalMap);

	const std::wstring& getNormalMap() const;

	void setColor(const Color4ub& color);

	const Color4ub& getColor() const;

	void setDiffuseTerm(float diffuseTerm);

	float getDiffuseTerm() const;

	void setSpecularTerm(float specularTerm);

	float getSpecularTerm() const;

	void setSpecularRoughness(float specularRoughness);

	float getSpecularRoughness() const;

	void setDoubleSided(bool doubleSided);

	bool isDoubleSided() const;
	
private:
	std::wstring m_name;
	std::wstring m_diffuseMap;
	BlendOperator m_diffuseBlendOperator;
	std::wstring m_specularMap;
	std::wstring m_normalMap;
	Color4ub m_color;
	float m_diffuseTerm;
	float m_specularTerm;
	float m_specularRoughness;
	bool m_doubleSided;
};

	}
}

#endif	// traktor_model_Material_H
