
set( HTMLOUTPUTPLUGIN_PUBLIC_HEADERS
                "HtmlOutput.h" )

add_library(htmlOutputPlugin SHARED
                ${HTMLOUTPUTPLUGIN_PUBLIC_HEADERS}
                "HtmlOutput.cpp")

add_custom_command(
        TARGET htmlOutputPlugin POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/htmlOutputPluginCodeHighlight.js
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/htmlOutputPluginCodeHighlight.js )

target_link_libraries(htmlOutputPlugin libDocmala)

set_target_properties(htmlOutputPlugin PROPERTIES
    PUBLIC_HEADER "${HTMLOUTPUTPLUGIN_PUBLIC_HEADERS}"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/plugins")

install(TARGETS htmlOutputPlugin
        RUNTIME DESTINATION bin
        PUBLIC_HEADER DESTINATION include/docmala
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/htmlOutputPluginCodeHighlight.js
              ${CMAKE_CURRENT_SOURCE_DIR}/htmlOutputPluginCodeHighlight.css
    DESTINATION bin/plugins)
