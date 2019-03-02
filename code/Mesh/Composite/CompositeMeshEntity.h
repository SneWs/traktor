#pragma once

#include <map>
#include "Mesh/MeshEntity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

class T_DLLCLASS CompositeMeshEntity : public MeshEntity
{
	T_RTTI_CLASS;

public:
	CompositeMeshEntity(const Transform& transform);

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual bool supportTechnique(render::handle_t technique) const override final;

	virtual void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass,
		float distance
	) override final;

	virtual void update(const world::UpdateParams& update) override final;

	void addMeshEntity(const std::wstring& name, MeshEntity* meshEntity);

	MeshEntity* getMeshEntity(const std::wstring& name) const;

	inline const std::map< std::wstring, Ref< MeshEntity > >& getMeshEntities() const { return m_meshEntities; }

private:
	std::map< std::wstring, Ref< MeshEntity > > m_meshEntities;
};

	}
}

