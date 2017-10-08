add_library(documentPluginTable SHARED
                "tablePlugin.cpp")

target_link_libraries(documentPluginTable docmala)

set_target_properties(documentPluginTable PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")


install(TARGETS documentPluginTable
    LIBRARY DESTINATION bin
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)
