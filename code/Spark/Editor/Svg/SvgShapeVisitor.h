#pragma once

#include "Core/Object.h"

namespace traktor
{
	namespace spark
	{

class SvgShape;

/*! \brief
 * \ingroup Spark
 */
class SvgShapeVisitor : public Object
{
	T_RTTI_CLASS;

public:
	virtual void enter(SvgShape* shape) = 0;

	virtual void leave(SvgShape* shape) = 0;
};

	}
}

