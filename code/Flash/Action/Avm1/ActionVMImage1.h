#ifndef traktor_flash_ActionVMImage1_H
#define traktor_flash_ActionVMImage1_H

#include "Core/Containers/AlignedVector.h"
#include "Flash/Action/ActionValue.h"
#include "Flash/Action/IActionVMImage.h"

namespace traktor
{

class Timer;

	namespace flash
	{

class ActionFrame;
class ActionVMTrace1;

class ActionVMImage1 : public IActionVMImage
{
	T_RTTI_CLASS;

public:
	ActionVMImage1();

	ActionVMImage1(const uint8_t* byteCode, uint32_t length);

	void execute(ActionFrame* frame, const Timer& timer, ActionVMTrace1* trace) const;

	void prepare();

	uint16_t addConstData(const ActionValue& cd);

	const ActionValue& getConstData(uint16_t index) const { return m_constData[index]; }

private:
	friend class ActionVM1;

	AlignedVector< uint8_t > m_byteCode;
	AlignedVector< ActionValue > m_constData;
};

	}
}

#endif	// traktor_flash_ActionVMImage1_H
