
include_directories("../../../ext/boost-process/include")

add_library(PlantUMLPlugin SHARED
                "PlantUML.cpp")
                
target_link_libraries(PlantUMLPlugin libDocmala)


add_custom_command(
        TARGET PlantUMLPlugin POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/plantuml.jar
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins/plantuml.jar )

set_target_properties(PlantUMLPlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/plugins")

install(TARGETS PlantUMLPlugin LIBRARY DESTINATION bin/plugins)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/plantuml.jar DESTINATION bin/plugins)
