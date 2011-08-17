#ifndef traktor_script_Script_H
#define traktor_script_Script_H

#include "Core/Guid.h"
#include "Editor/ITypedAsset.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

/*! \brief Persistent script.
 * \ingroup Script
 */
class T_DLLCLASS Script : public editor::ITypedAsset
{
	T_RTTI_CLASS;

public:
	Script();

	Script(const std::wstring& text);

	void addDependency(const Guid& dependency);

	std::vector< Guid >& getDependencies();

	const std::vector< Guid >& getDependencies() const;

	void setText(const std::wstring& text);

	const std::wstring& getText() const;

	virtual const TypeInfo* getOutputType() const;

	virtual bool serialize(ISerializer& s);

private:
	std::vector< Guid > m_dependencies;
	std::wstring m_text;
};

	}
}

#endif	// traktor_script_Script_H
