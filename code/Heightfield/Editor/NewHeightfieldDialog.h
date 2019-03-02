#pragma once

#include "Core/Math/Plane.h"
#include "Ui/ConfigDialog.h"

namespace traktor
{
	namespace ui
	{

class Command;
class Edit;

	}

	namespace hf
	{

class NewHeightfieldDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	int32_t getGridSize();

	Vector4 getWorldExtent();

	Plane getWorldPlane();

private:
	Ref< ui::Edit > m_editGridSize;
	Ref< ui::Edit > m_editExtent[3];
	Ref< ui::Edit > m_editPlane[3];
};

	}
}

