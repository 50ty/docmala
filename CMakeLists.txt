cmake_minimum_required(VERSION 3.7)

project(docmala)

find_package(Boost COMPONENTS program_options iostreams REQUIRED)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(ext/extension_system)
include_directories(ext/extension_system/src)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(libDocmala)


add_subdirectory(libDocmala)
add_subdirectory(plugins)


add_executable(docmala "main.cpp")
target_link_libraries(docmala extension_system libDocmala ${Boost_LIBRARIES})
