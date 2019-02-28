#ifndef __NETSYSTEM_EXTERN_H__
#define __NETSYSTEM_EXTERN_H__

/* Export functions from the DLL */
#ifndef MY_NETSYSTEM_EXTERN
# if defined(WIN32) || defined(_WIN32)
  /* Windows - set up dll import/export decorators. */
#  if defined(MY_NETSYSTEM_BUILDING_SHARED)
   /* Building shared library. */
#   define MY_NETSYSTEM_EXTERN __declspec(dllexport)
#  elif defined(MY_NETSYSTEM_USING_SHARED)
    /* Using shared library. */
#   define MY_NETSYSTEM_EXTERN __declspec(dllimport)
#  else
    /* Building static library. */
#    define MY_NETSYSTEM_EXTERN /* nothing */
#  endif
# elif __GNUC__ >= 4
#  define MY_NETSYSTEM_EXTERN __attribute__((visibility("default")))
# else
#  define MY_NETSYSTEM_EXTERN /* nothing */
# endif
#endif

#endif /* __NETSYSTEM_EXTERN_H__ */
