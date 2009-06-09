#ifndef traktor_physics_Conversion_H
#define traktor_physics_Conversion_H

#include "Core/Math/Vector4.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Matrix44.h"

namespace traktor
{
	namespace physics
	{

/*! \ingroup PhysX */
//@{

/*! \brief Convert from PhysX vector. */
T_FORCE_INLINE Vector4 fromNxVec3(const NxVec3& v, float w = 0.0f)
{
	return Vector4(v[0], v[1], v[2], w);
}

/*! \brief Convert to PhysX vector. */
T_FORCE_INLINE NxVec3 toNxVec3(const Vector4& v)
{
	return NxVec3(v.x(), v.y(), v.z());
}

/*! \brief Convert from PhysX matrix. */
T_FORCE_INLINE Matrix44 fromNxMat34(const NxMat34& m)
{
	return Matrix44(
		m.M(0, 0), m.M(1, 0), m.M(2, 0), 0.0f,
		m.M(0, 1), m.M(1, 1), m.M(2, 1), 0.0f,
		m.M(0, 2), m.M(1, 2), m.M(2, 2), 0.0f,
		m.t[0], m.t[1], m.t[2], 1.0f
	);
}

/*! \brief Convert to PhysX matrix. */
T_FORCE_INLINE NxMat34 toNxMat34(const Matrix44& m)
{
	NxMat34 r;
	r.M(0, 0) = m(0, 0); r.M(1, 0) = m(0, 1); r.M(2, 0) = m(0, 2);
	r.M(0, 1) = m(1, 0); r.M(1, 1) = m(1, 1); r.M(2, 1) = m(1, 2);
	r.M(0, 2) = m(2, 0); r.M(1, 2) = m(2, 1); r.M(2, 2) = m(2, 2);
	r.t[0] = m(3, 0); r.t[1] = m(3, 1); r.t[2] = m(3, 2);
	return r;
}

//@}

	}
}

#endif	// traktor_physics_Conversion_H
