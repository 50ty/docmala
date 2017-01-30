
add_library(htmlOutputPlugin SHARED
                "HtmlOutput.cpp")
                
target_link_libraries(htmlOutputPlugin libDocmala)
