
include_directories("../../../ext/boost-process/include")

add_library(documentPluginPlantUML SHARED
                "PlantUML_linux.cpp"
                "PlantUML_windows.cpp"
                "DetectOS.h")
                
target_link_libraries(documentPluginPlantUML docmala)


add_custom_command(
        TARGET documentPluginPlantUML POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/plantuml.jar
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plantuml.jar )

add_custom_command(
        TARGET documentPluginPlantUML POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/PlantUMLHost.jar
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/PlantUMLHost.jar )

set_target_properties(documentPluginPlantUML PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

install(TARGETS documentPluginPlantUML
    LIBRARY DESTINATION bin
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/plantuml.jar DESTINATION bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/PlantUMLHost.jar DESTINATION bin)
