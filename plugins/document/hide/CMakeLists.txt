
#include_directories("../../ext/extension_system/src")


add_library(hidePlugin SHARED
                "hidePlugin.cpp")
                
target_link_libraries(hidePlugin libDocmala)

set_target_properties(hidePlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/plugins"
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/plugins")


install(TARGETS hidePlugin 
        LIBRARY DESTINATION bin/plugins
        ARCHIVE DESTINATION bin/plugins
        RUNTIME DESTINATION bin/plugins)
