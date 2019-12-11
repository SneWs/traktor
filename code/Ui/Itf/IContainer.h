#pragma once

#include "Ui/Itf/IWidget.h"

namespace traktor
{
	namespace ui
	{

/*! Container interface.
 * \ingroup UI
 */
class IContainer : public IWidget
{
public:
	virtual bool create(IWidget* parent, int style) = 0;
};

	}
}

