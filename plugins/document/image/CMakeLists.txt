add_library(imagePlugin SHARED
                "imagePlugin.cpp")
                
target_link_libraries(imagePlugin libDocmala)

set_target_properties(imagePlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/plugins")


install(TARGETS imagePlugin 
        LIBRARY DESTINATION bin/plugins
        ARCHIVE DESTINATION bin/plugins)

