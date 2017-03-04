#pragma once

#if defined(libDocmala_EXPORTS)
#  define LIBDOCMALA_API __declspec(dllexport)
#else
#  define LIBDOCMALA_API __declspec(dllimport)
#endif

