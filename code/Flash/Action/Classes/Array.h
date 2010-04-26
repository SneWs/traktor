#ifndef traktor_flash_Array_H
#define traktor_flash_Array_H

#include "Flash/Action/ActionObject.h"
#include "Flash/Action/ActionValueArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

/*! \brief Array of ActionValue objects.
 * \ingroup Flash
 */
class T_DLLCLASS Array : public ActionObject
{
	T_RTTI_CLASS;

public:
	Array();

	Ref< Array > concat() const;

	Ref< Array > concat(const ActionValueArray& values) const;

	std::wstring join(const std::wstring& delimiter) const;

	void push(const ActionValue& value);

	ActionValue pop();

	uint32_t length() const;

	virtual void setMember(const std::wstring& memberName, const ActionValue& memberValue);

	virtual bool getMember(const std::wstring& memberName, ActionValue& outMemberValue) const;

	virtual std::wstring toString() const;

private:
	std::vector< ActionValue > m_values;
};

	}
}

#endif	// traktor_flash_Array_H
