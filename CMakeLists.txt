cmake_minimum_required(VERSION 3.7)

project(docmala)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 14)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wnon-virtual-dtor
                        -Woverloaded-virtual
                        $<$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,5.0>:-pedantic>
                        $<$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,4.9>:-fstack-protector-strong>
                        $<$<VERSION_LESS:$<CXX_COMPILER_VERSION>,5.0>:-Wno-missing-field-initializers>
                    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "^.*Clang$")
    add_compile_options(-Wno-c++98-compat -Wno-c++98-compat-pedantic
                    -Wno-global-constructors -Wno-exit-time-destructors
                    -Wno-documentation
                    -Wno-padded
                    -Wno-weak-vtables
                    -Wno-reserved-id-macro
                    -Wno-attributes -Weverything)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DNOMINMAX) # force windows.h not to define min and max

    # disable some annoying compiler warnings
    add_compile_options(/wd4275) # disable warning C4275: non dll-interface x used as base for dll-interface y
    add_compile_options(/wd4251) # disable warning C4251: x needs to have dll-interface to be used by clients of y
    add_compile_options(/wd4068) # disable warning C4068: unknown pragma

    # enable additional compiler warnings
    add_compile_options(/w14062 /w14263 /w14264 /w14289 /w14706)
else()
    message(FATAL_ERROR "Unsupported or unknown compiler ${CMAKE_CXX_COMPILER_ID}")
endif()

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release MinSizeRel RelWithDebInfo)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "^.*Clang$")
    add_compile_options(-foptimize-sibling-calls)
    add_compile_options(-Wall -Wextra -Wno-unknown-pragmas -Wwrite-strings -Wenum-compare
        -Wno-conversion-null -Werror=return-type)
    add_compile_options($<$<CONFIG:Release>:-D_FORTIFY_SOURCE=2>)

    if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -z now -pie")

        set(DOCMALA_SANITIZE "" CACHE STRING "Sanitizer not all options are available in all compiler versions")
        set_property(CACHE DOCMALA_SANITIZE PROPERTY STRINGS "" none address memory thread undefined leak)
        if(NOT "${DOCMALA_SANITIZE}" STREQUAL "")
            add_compile_options(-fsanitize=${DOCMALA_SANITIZE} -fno-omit-frame-pointer)
        endif()
    endif()
endif()

option(DOCMALA_ENABLE_CLANG_TIDY "" OFF)
if(DOCMALA_ENABLE_CLANG_TIDY)
    find_program(
        CLANG_TIDY_EXE
        NAMES "clang-tidy"
        DOC "Path to clang-tidy executable"
        )
    if(CLANG_TIDY_EXE)
        message(STATUS "Enable clang-tidy: ${CLANG_TIDY_EXE}")
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}" "-fix" "-checks=*,-cppcoreguidelines-pro-bounds-*,-cppcoreguidelines-pro-type-vararg,-google-build-using-namespace,-google-runtime-references,-modernize-pass-by-value,-readability-implicit-bool-cast")
    endif()
endif()

find_package(Boost COMPONENTS program_options REQUIRED)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(ext/extension_system)

add_subdirectory(docmala)
add_subdirectory(plugins)

add_executable(docma "main.cpp")
target_link_libraries(docma extension_system docmala Boost::program_options)

install(TARGETS docma
        RUNTIME DESTINATION bin
        PUBLIC_HEADER DESTINATION include/docmala
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)
