#pragma once

#if defined(__linux__)
#define DOCMALA_API
#else
#if defined(docmala_EXPORTS)
#define DOCMALA_API __declspec(dllexport)
#else
#define DOCMALA_API __declspec(dllimport)
#endif
#endif
