#pragma once

#include "Core/Guid.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Aabb3.h"
#include "Mesh/MeshResource.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class Shader;

	}

	namespace mesh
	{

class T_DLLCLASS StreamMeshResource : public MeshResource
{
	T_RTTI_CLASS;

public:
	struct T_DLLCLASS Part
	{
		std::wstring shaderTechnique;
		std::wstring meshPart;

		Part();

		void serialize(ISerializer& s);
	};

	virtual Ref< IMesh > createMesh(
		const std::wstring& name,
		IStream* dataStream,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		render::MeshFactory* meshFactory
	) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class StreamMeshConverter;
	typedef std::list< Part > parts_t;

	resource::Id< render::Shader > m_shader;
	AlignedVector< uint32_t > m_frameOffsets;
	Aabb3 m_boundingBox;
	std::map< std::wstring, parts_t > m_parts;
};

	}
}

