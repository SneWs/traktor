#pragma once

#include "Ui/Layout.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

/*! \brief Aspect layout.
 * \ingroup UI
 */
class T_DLLCLASS AspectLayout : public Layout
{
	T_RTTI_CLASS;

public:
	AspectLayout(float ratio = -1.0f);

	virtual bool fit(Widget* widget, const Size& bounds, Size& result) override;

	virtual void update(Widget* widget) override;

private:
	float m_ratio;
};

	}
}

