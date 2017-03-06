
set( HTMLOUTPUTPLUGIN_PUBLIC_HEADERS
                "HtmlOutput.h"
                "HtmlOutput_global.h")

add_library(outputPluginHtml SHARED
                ${HTMLOUTPUTPLUGIN_PUBLIC_HEADERS}
                "HtmlOutput.cpp")

add_custom_command(
    TARGET outputPluginHtml POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/outputPluginHtmlCodeHighlight.js
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/outputPluginHtmlCodeHighlight.js )
add_custom_command(
    TARGET outputPluginHtml POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/outputPluginHtmlCodeHighlight.css
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/outputPluginHtmlCodeHighlight.css )

add_custom_command(
    TARGET outputPluginHtml POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/outputPluginHtmlDefaultStyle.css
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/outputPluginHtmlDefaultStyle.css )

target_link_libraries(outputPluginHtml docmala)

set_target_properties(outputPluginHtml PROPERTIES
    PUBLIC_HEADER "${HTMLOUTPUTPLUGIN_PUBLIC_HEADERS}"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
    )

install(TARGETS outputPluginHtml
        PUBLIC_HEADER DESTINATION include/docmala
        LIBRARY DESTINATION bin
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/outputPluginHtmlCodeHighlight.js
                ${CMAKE_CURRENT_SOURCE_DIR}/outputPluginHtmlCodeHighlight.css
                ${CMAKE_CURRENT_SOURCE_DIR}/outputPluginHtmlDefaultStyle.css
                DESTINATION bin)
