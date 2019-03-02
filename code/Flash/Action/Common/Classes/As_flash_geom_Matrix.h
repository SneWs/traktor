#pragma once

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief Matrix class.
 * \ingroup Flash
 */
class As_flash_geom_Matrix : public ActionClass
{
	T_RTTI_CLASS;

public:
	As_flash_geom_Matrix(ActionContext* context);

	virtual void initialize(ActionObject* self) override final;

	virtual void construct(ActionObject* self, const ActionValueArray& args) override final;

	virtual ActionValue xplicit(const ActionValueArray& args) override final;

private:
	void Matrix_distance(CallArgs& ca);

	void Matrix_interpolate(CallArgs& ca);

	void Matrix_polar(CallArgs& ca);
};

	}
}

