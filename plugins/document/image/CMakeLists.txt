add_library(documentPluginImage SHARED
                "imagePlugin.cpp")

target_link_libraries(documentPluginImage docmala)

set_target_properties(documentPluginImage PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")


install(TARGETS documentPluginImage
    LIBRARY DESTINATION bin
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)
