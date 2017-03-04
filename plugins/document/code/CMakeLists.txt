
#include_directories("../../ext/extension_system/src")


add_library(codePlugin SHARED
                "codePlugin.cpp")
                
target_link_libraries(codePlugin libDocmala)

set_target_properties(codePlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/plugins")


install(TARGETS codePlugin 
        LIBRARY DESTINATION bin/plugins
        ARCHIVE DESTINATION bin/plugins)
