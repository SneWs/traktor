#include "Spark/Sprite.h"
#include "Spark/Editor/SparkEditorPage.h"
#include "Spark/Editor/SparkEditorPageFactory.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.SparkEditorPageFactory", 0, SparkEditorPageFactory, editor::IEditorPageFactory)

const TypeInfoSet SparkEditorPageFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Sprite >());
	return typeSet;
}

bool SparkEditorPageFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	outDependencies.insert(Guid(L"{5B786C6B-8818-A24A-BD1C-EE113B79BCE2}"));	// System/Primitive/Shaders/Primitive
	return true;
}

Ref< editor::IEditorPage > SparkEditorPageFactory::createEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document) const
{
	return new SparkEditorPage(editor, site, document);
}

void SparkEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

	}
}
