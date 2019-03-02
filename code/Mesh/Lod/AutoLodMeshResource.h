#pragma once

#include "Core/RefArray.h"
#include "Core/Math/Aabb3.h"
#include "Mesh/IMeshResource.h"

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

class StaticMeshResource;

class T_DLLCLASS AutoLodMeshResource : public IMeshResource
{
	T_RTTI_CLASS;

public:
	AutoLodMeshResource();

	virtual Ref< IMesh > createMesh(
		const std::wstring& name,
		IStream* dataStream,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		render::MeshFactory* meshFactory
	) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class AutoLodMeshConverter;

	float m_maxDistance;
	float m_cullDistance;
	Aabb3 m_boundingBox;
	RefArray< StaticMeshResource > m_lods;
};

	}
}

