#pragma once

#include <string>
#include "Core/Ref.h"
#include "Core/Misc/IHash.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief MD5 checksum.
 * \ingroup Core
 */
class T_DLLCLASS MD5 : public IHash
{
	T_RTTI_CLASS;

public:
	MD5();

	explicit MD5(const void* md5);

	/*! \brief Create object from MD5 checksum string.
	 *
	 * \param md5 32 character hex encoded MD5.
	 * \return True if created successfully.
	 */
	bool create(const std::wstring& md5);

	bool createFromString(const std::wstring& str);

	/*! \brief Begin feeding data for MD5 checksum calculation. */
	virtual void begin() override final;

	/*! \brief Feed data to MD5 checksum calculation.
	 *
	 * \param buffer Pointer to data.
	 * \param bufferSize Amount of data in bytes.
	 */
	virtual void feed(const void* buffer, uint64_t bufferSize) override final;

	/*! \brief End feeding data for MD5 checksum calculation. */
	virtual void end() override final;

	/*! \brief Get pointer to MD5 checksum. */
	const uint32_t* get() const;

	/*! \brief Format MD5 checksum as string. */
	std::wstring format() const;

	bool operator == (const MD5& md5) const;

	bool operator != (const MD5& md5) const;

	bool operator < (const MD5& md5) const;

private:
	uint8_t m_buffer[64];
	uint32_t m_count[2];
	uint32_t m_md5[4];

	void transform(const uint8_t block[64]);
};

}

