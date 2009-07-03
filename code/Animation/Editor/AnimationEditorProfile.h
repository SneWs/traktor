#ifndef traktor_animation_AnimationEditorProfile_H
#define traktor_animation_AnimationEditorProfile_H

#include "Scene/Editor/SceneEditorProfile.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

class T_DLLCLASS AnimationEditorProfile : public scene::SceneEditorProfile
{
	T_RTTI_CLASS(AnimationEditorProfile)

public:
	virtual void getCommands(
		std::list< ui::Command >& outCommands
	) const;

	virtual void createResourceFactories(
		scene::SceneEditorContext* context,
		RefArray< resource::IResourceFactory >& outResourceFactories
	) const;

	virtual void createEntityFactories(
		scene::SceneEditorContext* context,
		RefArray< world::IEntityFactory >& outEntityFactories
	) const;

	virtual void createEntityRenderers(
		scene::SceneEditorContext* context,
		render::RenderView* renderView,
		render::PrimitiveRenderer* primitiveRenderer,
		RefArray< world::IEntityRenderer >& outEntityRenderers
	) const;

	virtual void createEntityEditors(
		scene::SceneEditorContext* context,
		RefArray< scene::IEntityEditor >& outEntityEditors
	) const;
};

	}
}

#endif	// traktor_animation_AnimationEditorProfile_H
