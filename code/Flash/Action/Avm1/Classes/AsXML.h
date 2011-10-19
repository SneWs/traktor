#ifndef traktor_flash_AsXML_H
#define traktor_flash_AsXML_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief XML class.
 * \ingroup Flash
 */
class AsXML : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsXML();

	virtual Ref< ActionObject > alloc(ActionContext* context);

	virtual void init(ActionContext* context, ActionObject* self, const ActionValueArray& args);

private:
	void XML_addRequestHeader(CallArgs& ca);

	void XML_createElement(CallArgs& ca);

	void XML_createTextNode(CallArgs& ca);

	void XML_getBytesLoaded(CallArgs& ca);

	void XML_getBytesTotal(CallArgs& ca);

	void XML_load(CallArgs& ca);

	void XML_parseXML(CallArgs& ca);

	void XML_send(CallArgs& ca);

	void XML_sendAndLoad(CallArgs& ca);

	void XML_get_contentType(CallArgs& ca);

	void XML_set_contentType(CallArgs& ca);

	void XML_get_docTypeDecl(CallArgs& ca);

	void XML_set_docTypeDecl(CallArgs& ca);

	void XML_get_idMap(CallArgs& ca);

	void XML_set_idMap(CallArgs& ca);

	void XML_get_ignoreWhite(CallArgs& ca);

	void XML_set_ignoreWhite(CallArgs& ca);

	void XML_get_loaded(CallArgs& ca);

	void XML_set_loaded(CallArgs& ca);

	void XML_get_status(CallArgs& ca);

	void XML_set_status(CallArgs& ca);

	void XML_get_xmlDecl(CallArgs& ca);

	void XML_set_xmlDecl(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsXML_H
