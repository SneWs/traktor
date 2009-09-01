#ifndef traktor_Format_H
#define traktor_Format_H

#include "Core/Io/OutputStream.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Quaternion.h"

namespace traktor
{

	/*! \brief Format helper of Vector2 instances.
	 * \ingroup Core
	 */
	inline OutputStream& operator << (OutputStream& os, const Vector2& v)
	{
		os << v.x << L", " << v.y;
		return os;
	}

	/*! \brief Format helper of Vector4 instances.
	 * \ingroup Core
	 */
	inline OutputStream& operator << (OutputStream& os, const Vector4& v)
	{
		os << v.x() << L", " << v.y() << L", " << v.z() << L", " << v.w();
		return os;
	}

	/*! \brief Format helper of Quaternion instances.
	 * \ingroup Core
	 */
	inline OutputStream& operator << (OutputStream& os, const Quaternion& q)
	{
		os << q.x << L", " << q.y << L", " << q.z << L", " << q.w;
		return os;
	}

}

#endif	// traktor_Format_H
