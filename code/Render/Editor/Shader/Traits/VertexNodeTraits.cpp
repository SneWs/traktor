#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/VertexNodeTraits.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.VertexNodeTraits", 0, VertexNodeTraits, INodeTraits)

TypeInfoSet VertexNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< VertexInput >());
	typeSet.insert(&type_of< VertexOutput >());
	return typeSet;
}

PinType VertexNodeTraits::getOutputPinType(
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	if (const VertexInput* vertexInputNode = dynamic_type_cast< const VertexInput* >(node))
	{
		DataType vertexDataType = vertexInputNode->getDataType();
		switch (vertexDataType)
		{
		case DtFloat1:
			return PntScalar1;

		case DtFloat2:
		case DtShort2:
		case DtShort2N:
		case DtHalf2:
			return PntScalar2;

		case DtFloat3:
			return PntScalar3;

		case DtFloat4:
		case DtByte4:
		case DtByte4N:
		case DtShort4:
		case DtShort4N:
		case DtHalf4:
			return PntScalar4;
		}
	}
	return PntVoid;
}

PinType VertexNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* outputPinTypes
) const
{
	if (is_a< VertexOutput >(node))
		return PntScalar4;
	else
		return PntVoid;
}

bool VertexNodeTraits::evaluate(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	return false;
}

	}
}
