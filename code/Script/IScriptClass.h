#ifndef traktor_script_IScriptClass_H
#define traktor_script_IScriptClass_H

#include "Core/Object.h"
#include "Script/Any.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

/*! \brief Script class definition.
 * \ingroup Script
 *
 * This class is used to describe native
 * classes which are intended to be used
 * from scripts.
 */
class T_DLLCLASS IScriptClass : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Get exported native type. */
	virtual const TypeInfo& getExportType() const = 0;


	/*! \brief Have constructor. */
	virtual bool haveConstructor() const = 0;

	/*! \brief Construct new object. */
	virtual Ref< Object > construct(uint32_t argc, const Any* argv) const = 0;


	/*! \brief Get exported method count. */
	virtual uint32_t getMethodCount() const = 0;

	/*! \brief Get name of exported method. */
	virtual std::wstring getMethodName(uint32_t methodId) const = 0;

	/*! \brief Invoke exported method. */
	virtual Any invoke(Object* object, uint32_t methodId, uint32_t argc, const Any* argv) const = 0;

	/*! \brief Invoke unknown method. */
	virtual Any invokeUnknown(Object* object, const std::wstring& methodName, uint32_t argc, const Any* argv) const = 0;


	/*! \brief Get exported properties count. */
	virtual uint32_t getPropertyCount() const = 0;

	/*! \brief Get name of exported property. */
	virtual std::wstring getPropertyName(uint32_t propertyId) const = 0;

	/*! \brief Get property value. */
	virtual Any getPropertyValue(const Object* object, uint32_t propertyId) const = 0;

	/*! \brief Set property value. */
	virtual void setPropertyValue(Object* object, uint32_t propertyId, const Any& value) const = 0;
};

	}
}

#endif	// traktor_script_IScriptClass_H
