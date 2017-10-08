add_library(documentPluginCode SHARED
                "codePlugin.cpp")

target_link_libraries(documentPluginCode docmala)

set_target_properties(documentPluginCode PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

install(TARGETS documentPluginCode
        LIBRARY DESTINATION bin
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin)
