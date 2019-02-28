#pragma once

#ifdef __cplusplus 
extern "C" {
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

#pragma warning(disable:4819)

#include <stdint.h>

#ifdef __APPLE__
#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED
#include <malloc/malloc.h>
#else
#include <sys/malloc.h>
#endif
#else
#include <malloc.h>
#endif

#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN 1
# include <winsock2.h>
#else
	typedef char*           LPSTR;
	typedef const char*		LPCTSTR;
	typedef unsigned char	BYTE;
	typedef unsigned short	WORD;
	typedef unsigned long	DWORD;
	typedef unsigned int    LONG;
	typedef uint64_t		HANDLE;
	typedef uint64_t		HWND;
	typedef uint64_t		HDC;
#ifndef OBJC_BOOL_DEFINED
	typedef unsigned int	BOOL;
#endif
	typedef struct	tagPOINT { LONG x, y; }	POINT;
	typedef struct	tagRECT	{ LONG left; LONG top; LONG right; LONG bottom; }	RECT;
	typedef struct	tagSIZE { LONG cx; LONG cy; }	SIZE, *PSIZE;

	typedef uint32_t		COLORREF;
	typedef uint32_t		UINT;
	typedef BOOL*			LPBOOL;
	typedef wchar_t*		LPWSTR;
	typedef const wchar_t*	LPCWSTR;
	typedef const char*		LPCSTR;

#define     stricmp          strcasecmp
#define     RGB(r,g,b)		( (r) | ( (g) << 8 ) | ( (b) << 16 ) )
#define	GetRValue(rgb)	( (rgb) & 0xff )
#define	GetGValue(rgb)	( ( (rgb) >> 8 ) & 0xff )
#define	GetBValue(rgb)	( ( (rgb) >> 16 ) )

#define TRUE 1
#define FALSE 0

#define ZeroMemory(buf,Len) memset(buf,0,Len)

#define MAKEWORD(a, b)      ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
#define LOWORD(l)           ((WORD)(l))
#define HIWORD(l)           ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
#define LOBYTE(w)           ((BYTE)(w))
#define HIBYTE(w)           ((BYTE)(((WORD)(w) >> 8) & 0xFF))
#define MAX_PATH            255

#endif

#ifndef	TRACE
#define	TRACE	printf
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) {	if ( (p) != NULL) { delete (p); (p) = NULL; } }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p); (p)=NULL; } }
#endif

#ifndef SAFE_DELETE_ARRAY2
#define SAFE_DELETE_ARRAY2(p,size) { for (register int isda = 0; (p) && isda < (size); isda++) { SAFE_DELETE((p)[isda]); (p)[isda] = NULL; } }
#endif

#ifndef SAFE_DELETE_ARRAY3
#define SAFE_DELETE_ARRAY3(p) { for (register int isda = 0; (p) && isda < (p)->size(); isda++) { SAFE_DELETE((*(p))[isda]); } (p)->clear(); }
#endif


#ifdef	_DEBUG
#include "assert.h"
#else
#ifndef ASSERT
#define	ASSERT(exp)
#endif
#endif

#define WEEK_SECONDS			604800	// 24 hours * 7
#define DAY_SECONDS				86400	// 24 hours
#define CHINA_TIME_ZONE_LAG		28800	// 8 hours
#define HOUR_SECONDS			3600	// 1 hours
#define HALF_HOUR_SECONDS		1800	// 0.5 hours

#ifdef __cplusplus 
}
#endif 

/*EOF*/