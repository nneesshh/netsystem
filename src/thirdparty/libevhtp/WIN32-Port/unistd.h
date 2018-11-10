/** Dummy file to satisfy source file dependencies on Windows platform **/
#ifndef ___PLATFORM_UNISTD_H__
#define ___PLATFORM_UNISTD_H__

#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN 1
# include <winsock2.h>
# include <sys/timeb.h>
# include <sys/queue.h>
# include <sys/tree.h>
# include <io.h>
# include <process.h>
#endif

#include <getopt.h>

#ifdef _MSC_VER /* msvc */
# pragma warning(disable : 4786)
# ifndef INLINE
# define INLINE __inline
# endif
# define NOINLINE __declspec (noinline)
#else  /* gcc */
# ifndef INLINE
# define INLINE inline
# endif
# define NOINLINE __attribute__ ((noinline))
#endif

#ifdef _WIN32
typedef int       socket_len_t;
typedef int       sigset_t;
typedef int       mode_t;
//typedef SSIZE_T   ssize_t;
#else
typedef socklen_t socket_len_t;
#endif

#endif /* ___PLATFORM_UNISTD_H__ */