cmake_minimum_required(VERSION 3.7)

project(docmala)

find_package(Boost COMPONENTS program_options REQUIRED)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(ext/extension_system)
include_directories(ext/extension_system/src)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(docmala)


add_subdirectory(docmala)
add_subdirectory(plugins)


add_executable(docma "main.cpp")
target_link_libraries(docma extension_system docmala ${Boost_LIBRARIES})

install(TARGETS docma
        RUNTIME DESTINATION bin
        PUBLIC_HEADER DESTINATION include/docmala
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)
