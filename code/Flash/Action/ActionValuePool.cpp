#include "Flash/Action/ActionValue.h"
#include "Flash/Action/ActionValuePool.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

const uint32_t c_poolValueCount = 4096;

		}

ActionValuePool::ActionValuePool()
:	m_next(0)
,	m_peek(0)
{
	m_top.reset(new ActionValue [c_poolValueCount]);
	T_FATAL_ASSERT_M (m_top.ptr(), L"Out of memory");
}

ActionValue* ActionValuePool::alloc(uint32_t count)
{
	ActionValue* value = &m_top[m_next];
	m_next += count;
	m_peek = std::max(m_peek, m_next);
	return value;
}

uint32_t ActionValuePool::offset() const
{
	return m_next;
}

void ActionValuePool::rewind(uint32_t offset)
{
	T_ASSERT (offset <= m_next);
	m_next = offset;
}

void ActionValuePool::flush()
{
	for (uint32_t i = 0; i < m_peek; ++i)
		m_top[i] = ActionValue();
	m_next = 0;
	m_peek = 0;
}

	}
}
