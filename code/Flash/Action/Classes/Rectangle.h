#ifndef traktor_flash_Rectangle_H
#define traktor_flash_Rectangle_H

#include "Flash/Action/ActionObject.h"

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

/*! \brief ActionScript rectangle.
 * \ingroup Flash
 */
class T_DLLCLASS Rectangle : public ActionObject
{
	T_RTTI_CLASS;

public:
	avm_number_t left;
	avm_number_t top;
	avm_number_t width;
	avm_number_t height;

	Rectangle();

	Rectangle(avm_number_t left_, avm_number_t top_, avm_number_t right_, avm_number_t bottom_);

	bool contains(avm_number_t x, avm_number_t y) const;

	void inflate(avm_number_t dx, avm_number_t dy);

	bool isEmpty() const;

	void offset(avm_number_t dx, avm_number_t dy);

	virtual ActionValue toString() const;
};

	}
}

#endif	// traktor_flash_Rectangle_H
