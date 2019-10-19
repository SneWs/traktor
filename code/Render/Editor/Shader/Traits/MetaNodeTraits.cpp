#include "Render/Editor/Shader/External.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/Traits/MetaNodeTraits.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

int32_t getInputPinIndex(const Node* node, const InputPin* inputPin)
{
	int32_t inputPinCount = node->getInputPinCount();
	for (int32_t i = 0; i < inputPinCount; ++i)
	{
		if (node->getInputPin(i) == inputPin)
			return i;
	}
	T_FATAL_ERROR;
	return -1;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.MetaNodeTraits", 0, MetaNodeTraits, INodeTraits)

TypeInfoSet MetaNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< Branch >();
	typeSet.insert< Comment >();
	typeSet.insert< Connected >();
	typeSet.insert< External >();
	typeSet.insert< InputPort >();
	typeSet.insert< OutputPort >();
	typeSet.insert< Platform >();
	typeSet.insert< Renderer >();
	typeSet.insert< Type >();
	typeSet.insert< Variable >();
	return typeSet;
}

bool MetaNodeTraits::isRoot(const ShaderGraph* shaderGraph, const Node* node) const
{
	if (is_a< InputPort >(node) || is_a< OutputPort >(node))
		return true;
	else if (const External* externalNode = dynamic_type_cast< const External* >(node))
		return externalNode->getOutputPinCount() == 0;
	else if (const Variable* variableNode = dynamic_type_cast< const Variable* >(node))
	{
		return
			bool(shaderGraph->findSourcePin(variableNode->getInputPin(0)) != nullptr) &&
			bool(shaderGraph->getDestinationCount(variableNode->getOutputPin(0)) == 0);
	}
	else
		return false;
}

PinType MetaNodeTraits::getOutputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	PinType outputPinType = PntVoid;
	if (is_a< Branch >(node) || is_a< Connected >(node) || is_a< Platform >(node) || is_a< Renderer >(node))
	{
		uint32_t inputPinCount = node->getInputPinCount();
		for (uint32_t i = 0; i < inputPinCount; ++i)
			outputPinType = std::max< PinType >(
				outputPinType,
				inputPinTypes[i]
			);
	}
	else if (is_a< Type >(node))
	{
		switch (inputPinTypes[0])
		{
		default:
		case PntVoid:
			break;

		case PntScalar1:
			outputPinType = inputPinTypes[1];
			break;

		case PntScalar2:
		case PntScalar3:
		case PntScalar4:
			outputPinType = inputPinTypes[2];
			break;

		case PntMatrix:
			outputPinType = inputPinTypes[3];
			break;

		case PntTexture2D:
		case PntTexture3D:
		case PntTextureCube:
			outputPinType = inputPinTypes[4];
			break;

		case PntStructBuffer:
			break;

		case PntState:
			outputPinType = inputPinTypes[5];
			break;
		}
	}
	return outputPinType;
}

PinType MetaNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes
) const
{
	if (is_a< OutputPort >(node))
		return PntVoid;
	else
		return outputPinTypes[0];
}

int32_t MetaNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin
) const
{
	return getInputPinIndex(node, inputPin);
}

bool MetaNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	return false;
}

bool MetaNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const OutputPin** inputOutputPins,
	const Constant* inputConstants,
	const OutputPin*& foldOutputPin
) const
{
	return false;
}

PinOrderType MetaNodeTraits::evaluateOrder(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const PinOrderType* inputPinOrders,
	bool frequentAsLinear
) const
{
	return pinOrderMax(inputPinOrders, node->getInputPinCount());
}

	}
}
