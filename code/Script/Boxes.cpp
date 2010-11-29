#include "Script/AutoScriptClass.h"
#include "Script/Boxes.h"
#include "Script/IScriptManager.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.Vector4", BoxedVector4, Object)

BoxedVector4::BoxedVector4()
:	m_value(Vector4::zero())
{
}

BoxedVector4::BoxedVector4(const Vector4& value)
:	m_value(value)
{
}

BoxedVector4::BoxedVector4(float x, float y, float z)
:	m_value(x, y, z, 0.0f)
{
}

BoxedVector4::BoxedVector4(float x, float y, float z, float w)
:	m_value(x, y, z, w)
{
}

void BoxedVector4::set(float x, float y, float z, float w)
{
	m_value.set(x, y, z, w);
}

float BoxedVector4::x() const
{
	return m_value.x();
}

float BoxedVector4::y() const
{
	return m_value.y();
}

float BoxedVector4::z() const
{
	return m_value.z();
}

float BoxedVector4::w() const
{
	return m_value.w();
}

float BoxedVector4::length() const
{
	return m_value.length();
}

Vector4 BoxedVector4::normalized() const
{
	return m_value.normalized();
}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Quaternion", BoxedQuaternion, Object)

BoxedQuaternion::BoxedQuaternion()
:	m_value(Quaternion::identity())
{
}

BoxedQuaternion::BoxedQuaternion(const Quaternion& value)
:	m_value(value)
{
}

BoxedQuaternion::BoxedQuaternion(float x, float y, float z, float w)
:	m_value(x, y, z, w)
{
}

BoxedQuaternion::BoxedQuaternion(const Vector4& axis, float angle)
:	m_value(axis, angle)
{
}

BoxedQuaternion::BoxedQuaternion(float head, float pitch, float bank)
:	m_value(head, pitch, bank)
{
}

BoxedQuaternion::BoxedQuaternion(const Vector4& from, const Vector4& to)
:	m_value(from, to)
{
}

float BoxedQuaternion::x() const
{
	return m_value.e.x();
}

float BoxedQuaternion::y() const
{
	return m_value.e.y();
}

float BoxedQuaternion::z() const
{
	return m_value.e.z();
}

float BoxedQuaternion::w() const
{
	return m_value.e.w();
}

Quaternion BoxedQuaternion::normalized() const
{
	return m_value.normalized();
}

Quaternion BoxedQuaternion::inverse() const
{
	return m_value.inverse();
}

const Quaternion& BoxedQuaternion::unbox() const
{
	return m_value;
}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Transform", BoxedTransform, Object)

BoxedTransform::BoxedTransform()
{
}

BoxedTransform::BoxedTransform(const Transform& value)
:	m_value(value)
{
}

BoxedTransform::BoxedTransform(const Vector4& translation, const Quaternion& rotation)
:	m_value(translation, rotation)
{
}

const Vector4& BoxedTransform::translation() const
{
	return m_value.translation();
}

const Quaternion& BoxedTransform::rotation() const
{
	return m_value.rotation();
}

const Transform& BoxedTransform::unbox() const
{
	return m_value;
}

void registerBoxClasses(IScriptManager* scriptManager)
{
	Ref< AutoScriptClass< BoxedVector4 > > classBoxedVector4 = new AutoScriptClass< BoxedVector4 >();
	classBoxedVector4->addConstructor();
	classBoxedVector4->addConstructor< float, float, float >();
	classBoxedVector4->addConstructor< float, float, float, float >();
	classBoxedVector4->addMethod(L"set", &BoxedVector4::set);
	classBoxedVector4->addMethod(L"x", &BoxedVector4::x);
	classBoxedVector4->addMethod(L"y", &BoxedVector4::y);
	classBoxedVector4->addMethod(L"z", &BoxedVector4::z);
	classBoxedVector4->addMethod(L"w", &BoxedVector4::w);
	classBoxedVector4->addMethod(L"length", &BoxedVector4::length);
	classBoxedVector4->addMethod(L"normalized", &BoxedVector4::normalized);
	scriptManager->registerClass(classBoxedVector4);

	Ref< AutoScriptClass< BoxedQuaternion > > classBoxedQuaternion = new AutoScriptClass< BoxedQuaternion >();
	classBoxedQuaternion->addConstructor();
	classBoxedQuaternion->addConstructor< float, float, float, float >();
	classBoxedQuaternion->addConstructor< const Vector4&, float >();
	classBoxedQuaternion->addConstructor< float, float, float >();
	classBoxedQuaternion->addConstructor< const Vector4&, const Vector4& >();
	classBoxedQuaternion->addMethod(L"x", &BoxedQuaternion::x);
	classBoxedQuaternion->addMethod(L"y", &BoxedQuaternion::y);
	classBoxedQuaternion->addMethod(L"z", &BoxedQuaternion::z);
	classBoxedQuaternion->addMethod(L"w", &BoxedQuaternion::w);
	classBoxedQuaternion->addMethod(L"normalized", &BoxedQuaternion::normalized);
	classBoxedQuaternion->addMethod(L"inverse", &BoxedQuaternion::inverse);
	scriptManager->registerClass(classBoxedQuaternion);
	
	Ref< AutoScriptClass< BoxedTransform > > classBoxedTransform = new AutoScriptClass< BoxedTransform >();
	classBoxedTransform->addConstructor();
	classBoxedTransform->addConstructor< const Vector4&, const Quaternion& >();
	classBoxedTransform->addMethod(L"translation", &BoxedTransform::translation);
	classBoxedTransform->addMethod(L"rotation", &BoxedTransform::rotation);
	scriptManager->registerClass(classBoxedTransform);
}
	
	}
}
