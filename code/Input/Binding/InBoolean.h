#pragma once

#include "Core/RefArray.h"
#include "Input/Binding/IInputNode.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

/*! \brief
 * \ingroup Input
 */
class T_DLLCLASS InBoolean : public IInputNode
{
	T_RTTI_CLASS;

public:
	enum Operator
	{
		OpNot,
		OpAnd,
		OpOr,
		OpXor
	};

	InBoolean();

	InBoolean(
		IInputNode* source1,
		IInputNode* source2,
		Operator op
	);

	virtual Ref< Instance > createInstance() const override final;

	virtual float evaluate(
		Instance* instance,
		const InputValueSet& valueSet,
		float T,
		float dT
	) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class InBooleanTraits;

	RefArray< IInputNode > m_source;
	Operator m_op;
};

	}
}

