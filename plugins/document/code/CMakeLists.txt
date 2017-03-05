
#include_directories("../../ext/extension_system/src")


add_library(documentPluginCode SHARED
                "codePlugin.cpp")
                
target_link_libraries(documentPluginCode docmala)

set_target_properties(documentPluginCode PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/plugins"
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/plugins")


install(TARGETS documentPluginCode
        LIBRARY DESTINATION bin
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin)
