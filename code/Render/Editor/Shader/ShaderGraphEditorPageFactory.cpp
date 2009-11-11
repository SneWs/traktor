#include "Render/Editor/Shader/ShaderGraphEditorPageFactory.h"
#include "Render/Editor/Shader/ShaderGraphEditorPage.h"
#include "Render/ShaderGraph.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.render.ShaderGraphEditorPageFactory", ShaderGraphEditorPageFactory, editor::IEditorPageFactory)

const TypeSet ShaderGraphEditorPageFactory::getEditableTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< ShaderGraph >());
	return typeSet;
}

Ref< editor::IEditorPage > ShaderGraphEditorPageFactory::createEditorPage(editor::IEditor* editor) const
{
	return gc_new< ShaderGraphEditorPage >(editor);
}

void ShaderGraphEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
	outCommands.push_back(ui::Command(L"ShaderGraph.Editor.AlignLeft"));
	outCommands.push_back(ui::Command(L"ShaderGraph.Editor.AlignRight"));
	outCommands.push_back(ui::Command(L"ShaderGraph.Editor.AlignTop"));
	outCommands.push_back(ui::Command(L"ShaderGraph.Editor.AlignBottom"));
	outCommands.push_back(ui::Command(L"ShaderGraph.Editor.EvenSpaceVertically"));
	outCommands.push_back(ui::Command(L"ShaderGraph.Editor.EventSpaceHorizontally"));
	outCommands.push_back(ui::Command(L"ShaderGraph.Editor.Center"));
}

	}
}
