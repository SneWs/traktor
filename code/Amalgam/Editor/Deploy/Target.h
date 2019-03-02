#pragma once

#include <string>
#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_DEPLOY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

class TargetConfiguration;

/*! \brief
 * \ingroup Amalgam
 */
class T_DLLCLASS Target : public ISerializable
{
	T_RTTI_CLASS;

public:
	void setIdentifier(const std::wstring& identifier);

	const std::wstring& getIdentifier() const;

	void addConfiguration(TargetConfiguration* configuration);

	void removeConfiguration(TargetConfiguration* configuration);

	void removeAllConfigurations();

	const RefArray< TargetConfiguration >& getConfigurations() const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_identifier;
	RefArray< TargetConfiguration > m_configurations;
};

	}
}

