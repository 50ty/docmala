
#include_directories("../../ext/extension_system/src")


add_library(tablePlugin SHARED
                "tablePlugin.cpp")
                
target_link_libraries(tablePlugin libDocmala)

set_target_properties(tablePlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/plugins")


install(TARGETS tablePlugin LIBRARY DESTINATION bin/plugins)
