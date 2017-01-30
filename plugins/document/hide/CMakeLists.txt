
#include_directories("../../ext/extension_system/src")


add_library(hidePlugin SHARED
                "hidePlugin.cpp")
                
target_link_libraries(hidePlugin libDocmala)
