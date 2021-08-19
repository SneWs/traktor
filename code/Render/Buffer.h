#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IBufferView;

/*! GPU buffer.
 * \ingroup Render
 */
class T_DLLCLASS Buffer : public Object
{
	T_RTTI_CLASS;

public:
	/*! Get buffer size in bytes.
	 *
	 * \return Buffer size.
	 */
	uint32_t getBufferSize() const;

	/*! Destroy resources allocated by this buffer. */
	virtual void destroy() = 0;

	/*! Lock access to entire buffer data.
	 *
	 * \return Pointer to buffer beginning.
	 */
	virtual void* lock() = 0;

	/*! Unlock access. */
	virtual void unlock() = 0;

	/*! */
	virtual const IBufferView* getBufferView() const = 0;

protected:
	explicit Buffer(uint32_t bufferSize);

private:
	uint32_t m_bufferSize;
};

	}
}

