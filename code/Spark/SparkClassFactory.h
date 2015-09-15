#ifndef traktor_spark_SparkClassFactory_H
#define traktor_spark_SparkClassFactory_H

#include "Core/Class/IRuntimeClassFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

/*! \brief Spark runtime class factory.
 * \ingroup Spark
 */
class T_DLLCLASS SparkClassFactory : public IRuntimeClassFactory
{
	T_RTTI_CLASS;

public:
	virtual void createClasses(IRuntimeClassRegistrar* registrar) const;
};

	}
}

#endif	// traktor_spark_SparkClassFactory_H
