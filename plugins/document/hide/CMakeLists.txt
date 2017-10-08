add_library(documentPluginHide SHARED
                "hidePlugin.cpp")

target_link_libraries(documentPluginHide docmala)

set_target_properties(documentPluginHide PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")


install(TARGETS documentPluginHide
    LIBRARY DESTINATION bin
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)
