#include "Ai/NavMeshEntityData.h"
#include "Ai/Editor/NavMeshEntityEditor.h"
#include "Ai/Editor/NavMeshEntityEditorFactory.h"

namespace traktor
{
	namespace ai
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ai.NavMeshEntityEditorFactory", NavMeshEntityEditorFactory, scene::IEntityEditorFactory)

const TypeInfoSet NavMeshEntityEditorFactory::getEntityDataTypes() const
{
	return makeTypeInfoSet< NavMeshEntityData >();
}

Ref< scene::IEntityEditor > NavMeshEntityEditorFactory::createEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter) const
{
	return new NavMeshEntityEditor(context, entityAdapter);
}

	}
}
