#pragma once

#if defined(__linux__)
#  define LIBDOCMALA_API
#else
#  if defined(libDocmala_EXPORTS)
#    define LIBDOCMALA_API __declspec(dllexport)
#  else
#    define LIBDOCMALA_API __declspec(dllimport)
#  endif
#endif
