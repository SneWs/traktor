#include "Core/Math/Const.h"
#include "Physics/Body.h"
#include "Physics/BodyDesc.h"
#include "Physics/Editor/RigidEntityEditor.h"
#include "Physics/World/RigidEntity.h"
#include "Physics/World/RigidEntityData.h"
#include "Render/PrimitiveRenderer.h"
#include "Resource/IResourceManager.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Command.h"
#include "World/Entity/Entity.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.RigidEntityEditor", RigidEntityEditor, scene::DefaultEntityEditor)

RigidEntityEditor::RigidEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter)
:	scene::DefaultEntityEditor(context, entityAdapter)
,	m_showHull(false)
{
}

void RigidEntityEditor::entitySelected(bool selected)
{
	if (selected)
		m_showHull = false;
}

void RigidEntityEditor::applyModifier(const ApplyParams& params)
{
	// Apply default modifier.
	scene::DefaultEntityEditor::applyModifier(params);

	// Ensure body is enabled as it might have gone to sleep.
	Ref< RigidEntity > rigidEntity = checked_type_cast< RigidEntity* >(getEntityAdapter()->getEntity());
	if (rigidEntity->getBody())
		rigidEntity->getBody()->setActive(true);
}

bool RigidEntityEditor::handleCommand(const ui::Command& command)
{
	if (command == L"Physics.ToggleMeshTriangles")
	{
		m_showHull = !m_showHull;
		return true;
	}
	else
		return false;
}

void RigidEntityEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer) const
{
	Ref< RigidEntityData > rigidEntityData = checked_type_cast< RigidEntityData* >(getEntityAdapter()->getEntityData());

	const BodyDesc* bodyDesc = rigidEntityData->getBodyDesc();
	if (bodyDesc)
	{
		const ShapeDesc* shapeDesc = bodyDesc->getShape();
		if (shapeDesc)
		{
			Transform body1Transform0 = getEntityAdapter()->getTransform0();
			Transform body1Transform = getEntityAdapter()->getTransform();

			m_physicsRenderer.draw(
				getContext()->getResourceManager(),
				primitiveRenderer,
				body1Transform0,
				body1Transform,
				shapeDesc
			);
		}
	}
}

	}
}
