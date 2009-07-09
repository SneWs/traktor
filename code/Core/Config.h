#ifndef traktor_Config_H
#define traktor_Config_H

/*! \ingroup Core */
//@{

#if defined (_MSC_VER)
#	if !defined (T_STATIC) && !defined (__CLR_VER)
#		define T_DLLIMPORT __declspec(dllimport)
#		define T_DLLEXPORT __declspec(dllexport)
#	endif
#	define T_NOVTABLE __declspec(novtable)
#	define T_RESTRICT __restrict
#	define T_FORCE_INLINE __forceinline
#	define T_ALIGN16 __declspec(align(16))
#	if defined(WINCE)
#		define T_UNALIGNED __unaligned
#	endif
#elif defined(__GNUC__)
#	if !defined(T_STATIC)
#		define T_DLLIMPORT
#		define T_DLLEXPORT
#		define T_DLLLOCAL __attribute__((visibility("hidden")))
#	endif
#endif

#if !defined(T_DLLIMPORT)
#	define T_DLLIMPORT
#endif
#if !defined(T_DLLEXPORT)
#	define T_DLLEXPORT
#endif
#if !defined(T_DLLLOCAL)
#	define T_DLLLOCAL
#endif
#if !defined(T_NOVTABLE)
#	define T_NOVTABLE
#endif
#if !defined(T_RESTRICT)
#	define T_RESTRICT
#endif
#if !defined(T_FORCE_INLINE)
#	define T_FORCE_INLINE
#endif
#if !defined(T_ALIGN16)
#	define T_ALIGN16
#endif
#if !defined(T_UNALIGNED)
#	define T_UNALIGNED
#endif

// Detect endian.
#if defined(__APPLE__)

// Mac has a special header defining target endian.
#	include <TargetConditionals.h>
#	if TARGET_RT_LITTLE_ENDIAN
#		define T_LITTLE_ENDIAN
#	elif TARGET_RT_BIG_ENDIAN
#		define T_BIG_ENDIAN
#	endif

#elif defined(_XBOX) || defined(_PS3)

// Both Xenon and PS3 use a PowerPC derivate.
#	define T_BIG_ENDIAN

#elif defined(_WIN32)

// Assume little endian on Win32 as it's probably most common.
#	define T_LITTLE_ENDIAN

#elif defined(__GNUC__)

// Assume little endian on unknown platform using GCC, assuming Linux/x86 for now.
#	define T_LITTLE_ENDIAN

#endif

// Disable various compiler warnings.
#if defined(_MSC_VER)
#	pragma warning( disable : 4275 )	// non-dll class foo used as base for dll-interface class bar
#	pragma warning( disable : 4251 )	// <identifier> : class foo needs to have dll-interface to be used by clients of class bar
#	pragma warning( disable : 4190 )	// 'foo' has C-linkage specified, but returns UDT 'bar' which is incompatible with C
#	pragma warning( disable : 4345 )	// behavior change: an object of POD type constructed with an initializer of the form () will be default-initialized
#	pragma deprecated("null")
#endif

// Cross platform type definitions.
#if defined(_MSC_VER)
#	if !defined(T_HAVE_TYPES)

typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;

typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;

typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;

typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;

#		define T_HAVE_TYPES
#	endif
#else

#include <sys/types.h>

#	if !defined(__int8_t_defined)

typedef signed char int8_t;
typedef unsigned char uint8_t;

typedef short int16_t;
typedef unsigned short uint16_t;

typedef int int32_t;
typedef unsigned int uint32_t;

#		if !defined(__APPLE__)

typedef long int int64_t;
typedef long unsigned int uint64_t;

#		endif

#	endif

#endif

// Helper macros
#if !defined(sizeof_array)
#	define sizeof_array(arr) (sizeof(arr) / sizeof(arr[0]))
#endif

// Use these macros in c/d tors only.
#if !defined(T_EXCEPTION_GUARD_BEGIN)
#	define T_EXCEPTION_GUARD_BEGIN try {
#endif
#if !defined(T_EXCEPTION_GUARD_END)
#	define T_EXCEPTION_GUARD_END } catch (...) { T_BREAKPOINT; }
#endif

template < typename T >
size_t alignOf()
{
#if !defined(alignof)
#	if defined(_MSC_VER)
	return __alignof(T);
#	elif defined(__GNUC__)
	return __alignof__(T);
#	else
	struct __align_struct__ { char dummy1; T dummy2; };
	return offsetof(__align_struct, dummy2);
#	endif
#endif
}

// Debug settings.
#if defined(_DEBUG)
//#	define T_DEBUG_REFERENCES				//< Add some additional information to reference containers about object they are referencing.
//#	define T_DEBUG_OBJECT_NOT_REFERENCED	//< Check object isn't referenced when destroyed.
#	if defined(_MSC_VER) && !defined(_XBOX) && !defined(_WIN64) && !defined(WINCE)
#		define T_DEBUG_REFERENCE_ADDRESS		//< Reference instantiation address; must be x86 32bit arch currently.
#endif
#endif

// Include assert helper.
#include "Core/Assert.h"

//@}

#endif	// traktor_Config_H
