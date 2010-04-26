#ifndef traktor_flash_AsPoint_H
#define traktor_flash_AsPoint_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief Point class.
 * \ingroup Flash
 */
class AsPoint : public ActionClass
{
	T_RTTI_CLASS;

public:
	static Ref< AsPoint > getInstance();

private:
	AsPoint();

	void createPrototype();

	virtual ActionValue construct(ActionContext* context, const ActionValueArray& args);

	void Point_add(CallArgs& ca);

	void Point_clone(CallArgs& ca);

	void Point_distance(CallArgs& ca);

	void Point_equals(CallArgs& ca);

	void Point_interpolate(CallArgs& ca);

	void Point_normalize(CallArgs& ca);

	void Point_offset(CallArgs& ca);

	void Point_polar(CallArgs& ca);

	void Point_subtract(CallArgs& ca);

	void Point_toString(CallArgs& ca);

	void Point_get_length(CallArgs& ca);

	void Point_set_length(CallArgs& ca);

	void Point_get_x(CallArgs& ca);

	void Point_set_x(CallArgs& ca);

	void Point_get_y(CallArgs& ca);

	void Point_set_y(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsPoint_H
