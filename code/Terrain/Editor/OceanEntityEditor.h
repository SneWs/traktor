#ifndef traktor_terrain_OceanEntityEditor_H
#define traktor_terrain_OceanEntityEditor_H

#include "Scene/Editor/IEntityEditor.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace terrain
	{

class T_DLLCLASS OceanEntityEditor : public scene::IEntityEditor
{
	T_RTTI_CLASS(OceanEntityEditor)

public:
	virtual TypeSet getEntityTypes() const;

	virtual bool isPickable(
		scene::EntityAdapter* entityAdapter
	) const;

	virtual void entitySelected(
		scene::SceneEditorContext* context,
		scene::EntityAdapter* entityAdapter,
		bool selected
	) const;

	virtual void applyModifier(
		scene::SceneEditorContext* context,
		scene::EntityAdapter* entityAdapter,
		const Matrix44& viewTransform,
		const Vector2& mouseDelta,
		int mouseButton
	) const;

	virtual bool handleCommand(
		scene::SceneEditorContext* context,
		scene::EntityAdapter* entityAdapter,
		const ui::Command& command
	) const;

	virtual void drawGuide(
		scene::SceneEditorContext* context,
		render::PrimitiveRenderer* primitiveRenderer,
		scene::EntityAdapter* entityAdapter
	) const;

	virtual bool getStatusText(
		scene::SceneEditorContext* context,
		scene::EntityAdapter* entityAdapter,
		std::wstring& outStatusText
	) const;
};

	}
}

#endif	// traktor_terrain_OceanEntityEditor_H
