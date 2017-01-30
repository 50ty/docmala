
include_directories("../../../ext/boost-process/include")

add_library(PlantUMLPlugin SHARED
                "PlantUML.cpp")
                
target_link_libraries(PlantUMLPlugin libDocmala)
