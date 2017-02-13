
add_library(includePlugin SHARED
                "includePlugin.cpp")
                
target_link_libraries(includePlugin libDocmala)

set_target_properties(includePlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/plugins")

install(TARGETS includePlugin LIBRARY DESTINATION bin/plugins)
