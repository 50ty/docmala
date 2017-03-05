#pragma once

#if defined(__linux__)
#  define HTMLOUTPUT_API
#else
#  if defined(outputPluginHtml_EXPORTS)
#    define HTMLOUTPUT_API __declspec(dllexport)
#  else
#    define HTMLOUTPUT_API __declspec(dllimport)
#  endif
#endif
