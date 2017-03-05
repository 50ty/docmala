
#include_directories("../../ext/extension_system/src")


add_library(documentPluginHide SHARED
                "hidePlugin.cpp")
                
target_link_libraries(documentPluginHide docmala)

set_target_properties(documentPluginHide PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/plugins"
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/plugins")


install(TARGETS documentPluginHide
    LIBRARY DESTINATION bin
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)
