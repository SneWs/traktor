#ifndef traktor_render_RenderSettingsPage_H
#define traktor_render_RenderSettingsPage_H

#include "Editor/ISettingsPage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class DropDown;
class Edit;

	}

	namespace render
	{

/*! \brief Editor render settings page.
 * \ingroup Render
 */
class T_DLLCLASS RenderSettingsPage : public editor::ISettingsPage
{
	T_RTTI_CLASS;

public:
	virtual bool create(ui::Container* parent, Settings* settings, const std::list< ui::Command >& shortcutCommands);

	virtual void destroy();

	virtual bool apply(Settings* settings);

private:
	Ref< ui::DropDown > m_dropRenderSystem;
	Ref< ui::DropDown > m_dropCompiler;
	Ref< ui::Edit > m_editMipBias;
	Ref< ui::Edit > m_editMaxAnisotropy;
};

	}
}

#endif	// traktor_render_RenderSettingsPage_H
