
add_library(includePlugin SHARED
                "includePlugin.cpp")
                
target_link_libraries(includePlugin libDocmala)


install(TARGETS includePlugin LIBRARY DESTINATION bin/plugins)
