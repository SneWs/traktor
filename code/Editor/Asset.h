#ifndef traktor_editor_Asset_H
#define traktor_editor_Asset_H

#include "Core/Io/Path.h"
#include "Editor/ITypedAsset.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

/*! \brief Base class for assets.
 * \ingroup Editor
 *
 * Derive from this class for assets which reference
 * source assets such as image files etc.
 */
class T_DLLCLASS Asset : public ITypedAsset
{
	T_RTTI_CLASS;

public:
	Asset();

	Asset(const Path& fileName);

	void setFileName(const Path& fileName);

	const Path& getFileName() const;

	virtual bool serialize(ISerializer& s);

private:
	Path m_fileName;
};

	}
}

#endif	// traktor_editor_Asset_H
