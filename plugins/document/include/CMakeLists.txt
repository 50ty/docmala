
add_library(documentPluginInclude SHARED
                "includePlugin.cpp")
                
target_link_libraries(documentPluginInclude docmala)

set_target_properties(documentPluginInclude PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/plugins"
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/plugins")

install(TARGETS documentPluginInclude
    LIBRARY DESTINATION bin
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)
