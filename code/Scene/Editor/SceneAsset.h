#pragma once

#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"
#include "World/WorldTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class LayerEntityData;
class WorldRenderSettings;

	}

	namespace scene
	{

class ISceneControllerData;

class T_DLLCLASS SceneAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	SceneAsset();

	void setWorldRenderSettings(world::WorldRenderSettings* worldRenderSettings);

	Ref< world::WorldRenderSettings > getWorldRenderSettings() const;

	void setLayers(const RefArray< world::LayerEntityData >& layers);

	const RefArray< world::LayerEntityData >& getLayers() const;

	void setControllerData(ISceneControllerData* controllerData);

	Ref< ISceneControllerData > getControllerData() const;

	void setOperationData(const RefArray< ISerializable >& operationData);

	const RefArray< ISerializable >& getOperationData() const;

	virtual void serialize(ISerializer& s) override final;

private:
	Ref< world::WorldRenderSettings > m_worldRenderSettings;
	RefArray< world::LayerEntityData > m_layers;
	Ref< ISceneControllerData > m_controllerData;
	RefArray< ISerializable > m_operationData;
};

	}
}

