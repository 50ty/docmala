#pragma once

#if defined(WIN64) || defined(_WIN64) || defined(__WIN64__) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define CURRENT_OS_WINDOWS
#elif defined(__linux__) || defined(__linux)
#define CURRENT_OS_LINUX
#else
#define plantuml plugin does currently not support your OS.
#endif
