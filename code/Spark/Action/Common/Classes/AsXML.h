#pragma once

#include "Spark/Action/ActionClass.h"

namespace traktor
{
	namespace spark
	{

struct CallArgs;
class XML;

/*! XML class.
 * \ingroup Spark
 */
class AsXML : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsXML(ActionContext* context);

	virtual void initialize(ActionObject* self) override final;

	virtual void construct(ActionObject* self, const ActionValueArray& args) override final;

	virtual ActionValue xplicit(const ActionValueArray& args) override final;

private:
	void XML_addRequestHeader(CallArgs& ca);

	void XML_createElement(CallArgs& ca);

	void XML_createTextNode(CallArgs& ca);

	void XML_getBytesLoaded(CallArgs& ca);

	void XML_getBytesTotal(CallArgs& ca);

	bool XML_load(XML* self, const std::wstring& url) const;

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

