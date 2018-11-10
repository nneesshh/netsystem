#ifndef STRINGS_H_
#define STRINGS_H_

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN  /* We always want minimal includes */
#endif
#include <windows.h>
#include <io.h>              /* because we so often use open/close/etc */
#include <direct.h>          /* for _getcwd */
#include <process.h>         /* for _getpid */
#include <limits.h>          /* for PATH_MAX */
#include <stdarg.h>          /* for va_list */
#include <stdio.h>           /* need this to override stdio's (v)snprintf */
#include <sys/types.h>       /* for _off_t */
#include <assert.h>
#include <stdlib.h>          /* for rand, srand, _strtoxxx */

/*
 * 4018: signed/unsigned mismatch is common (and ok for signed_i < unsigned_i)
 * 4244: otherwise we get problems when subtracting two size_t's to an int
 * 4288: VC++7 gets confused when a var is defined in a loop and then after it
 * 4267: too many false positives for "conversion gives possible data loss"
 * 4290: it's ok windows ignores the "throw" directive
 * 4996: Yes, we're ok using "unsafe" functions like vsnprintf and getenv()
 * 4146: internal_logging.cc intentionally negates an unsigned value
 */
#ifdef _MSC_VER
#pragma warning(disable:4018 4244 4288 4267 4290 4996 4146)
#endif

#ifndef __cplusplus
/* MSVC does not support C99 */
# if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
#  ifdef _MSC_VER
#    define INLINE __inline
#  else
#    define INLINE static
#  endif
# endif
#endif

#ifdef __cplusplus
# define EXTERN_C  extern "C"
#else
# define EXTERN_C  extern
#endif

#ifdef _MSC_VER

/* MSVC doesn't define ffs/ffsl. This dummy strings.h header is provided
* for both */
#include <intrin.h>
#pragma intrinsic(_BitScanForward)
#ifdef _WIN64
#pragma intrinsic(_BitScanForward64)
#endif

#ifdef __cplusplus
extern "C" {
#endif

	int ffsl(long x);
	int ffs(int x);

#ifdef __cplusplus
}
#endif

static __forceinline int ffsl(long x) {
	unsigned long i = 0;

	if (_BitScanForward(&i, (unsigned long)x))
		return (i + 1);
	return (0);
}

static __forceinline int ffs(int x) {
	return (ffsl(x));
}

#endif  /* _MSC_VER */

#undef inline
#undef EXTERN_C

#endif  /* _WIN32 */

#endif
