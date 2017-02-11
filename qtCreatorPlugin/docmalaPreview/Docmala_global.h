#pragma once

#include <QtGlobal>

#if defined(Docmala_LIBRARY)
#  define DocmalaSHARED_EXPORT Q_DECL_EXPORT
#else
#  define DocmalaSHARED_EXPORT Q_DECL_IMPORT
#endif
