#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class Database;

	}

	namespace editor
	{

class IPipelineDepends;

	}

	namespace render
	{

class ShaderGraph;

	}

	namespace model
	{

class Material;

	}

	namespace mesh
	{

class T_DLLCLASS MaterialShaderGenerator : public Object
{
	T_RTTI_CLASS;

public:
	Ref< render::ShaderGraph > generate(
		db::Database* database,
		const model::Material& material,
		const Guid& materialTemplate,
		bool vertexColor
	) const;

	void addDependencies(editor::IPipelineDepends* pipelineDepends);
};

	}
}

