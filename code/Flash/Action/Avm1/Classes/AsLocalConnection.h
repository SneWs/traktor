#ifndef traktor_flash_AsLocalConnection_H
#define traktor_flash_AsLocalConnection_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief LocalConnection class.
 * \ingroup Flash
 */
class AsLocalConnection : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsLocalConnection();

	virtual Ref< ActionObject > alloc(ActionContext* context);

	virtual void init(ActionContext* context, ActionObject* self, const ActionValueArray& args);

private:
	void LocalConnection_close(CallArgs& ca);

	void LocalConnection_connect(CallArgs& ca);

	void LocalConnection_domain(CallArgs& ca);

	void LocalConnection_send(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsLocalConnection_H
