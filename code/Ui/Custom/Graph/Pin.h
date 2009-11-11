#ifndef traktor_ui_custom_Pin_H
#define traktor_ui_custom_Pin_H

#include <string>
#include <map>
#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Ui/Associative.h"
#include "Ui/Point.h"
#include "Ui/Rect.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

class Node;

/*! \brief Graph node pin.
 * \ingroup UIC
 */
class T_DLLCLASS Pin
:	public Object
,	public Associative
{
	T_RTTI_CLASS(Pin)

public:
	enum Direction
	{
		DrInput,
		DrOutput
	};

	Pin(Node* node, const std::wstring& name, Direction direction, bool mandatory);

	Ref< Node > getNode();

	const std::wstring& getName() const;

	Direction getDirection() const;

	bool isMandatory() const;

	Point getPosition() const;

private:
	Ref< Node > m_node;
	std::wstring m_name;
	Direction m_direction;
	bool m_mandatory;
};

		}
	}
}

#endif	// traktor_ui_custom_Pin_H
