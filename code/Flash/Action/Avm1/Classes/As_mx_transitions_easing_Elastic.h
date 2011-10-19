#ifndef traktor_flash_As_mx_transitions_easing_Elastic_H
#define traktor_flash_As_mx_transitions_easing_Elastic_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class As_mx_transitions_easing_Elastic : public ActionClass
{
	T_RTTI_CLASS;

public:
	As_mx_transitions_easing_Elastic();

	virtual Ref< ActionObject > alloc(ActionContext* context);

	virtual void init(ActionContext* context, ActionObject* self, const ActionValueArray& args);

private:
	void Elastic_easeIn(CallArgs& ca);

	void Elastic_easeInOut(CallArgs& ca);

	void Elastic_easeOut(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_As_mx_transitions_easing_Elastic_H
