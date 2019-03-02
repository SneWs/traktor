#pragma once

#include "Resource/IResourceFactory.h"

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

class IRenderSystem;
class MeshFactory;

	}

	namespace mesh
	{

class T_DLLCLASS MeshFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS;

public:
	MeshFactory(render::IRenderSystem* renderSystem, render::MeshFactory* meshFactory = 0);

	virtual const TypeInfoSet getResourceTypes() const override final;

	virtual const TypeInfoSet getProductTypes(const TypeInfo& resourceType) const override final;

	virtual bool isCacheable(const TypeInfo& productType) const override final;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const override final;

private:
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< render::MeshFactory > m_meshFactory;
};

	}
}

