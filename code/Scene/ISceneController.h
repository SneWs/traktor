#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace scene
	{

class Scene;

/*! \brief Scene controller interface.
 * \ingroup Scene
 *
 * Scene controllers are data-driven controllers
 * which can be associated with a scene.
 * Controllers are able to access all entities defined
 * in the scene which makes it possible to create for
 * instance cut-scene controllers etc. without involving
 * too much game logic.
 */
class T_DLLCLASS ISceneController : public Object
{
	T_RTTI_CLASS;

public:
	virtual void update(Scene* scene, float time, float deltaTime) = 0;
};

	}
}

