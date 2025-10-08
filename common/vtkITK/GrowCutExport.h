
#ifndef GrowCut_EXPORT_H
#define GrowCut_EXPORT_H


#define GrowCut_STATIC

#if defined(_WIN32) && !defined(GrowCut_STATIC)
# if defined(GrowCut_EXPORTS)
#  define GrowCut_EXPORT __declspec(dllexport)
# else
#  define GrowCut_EXPORT __declspec(dllimport)
# endif
#else
# define GrowCut_EXPORT
#endif


#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef GROWCUT_NO_DEPRECATED
#    define GROWCUT_NO_DEPRECATED
#  endif
#endif
//#include "itkNamespace.h"
#endif /* GrowCut_EXPORT_H */
