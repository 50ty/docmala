
include_directories("../ext/extension_system/src")


set( LIBDOCMALA_PUBLIC_HEADERS
                "docmala/Docmala.h"
                "docmala/DocmaPlugin.h"
                "docmala/File.h"
                "docmala/FileLocation.h"
                "docmala/Parameter.h"
                "docmala/DocumentPart.h"
                "docmala/Error.h"
                "docmala/Document.h")

add_library(libDocmala SHARED
                ${LIBDOCMALA_PUBLIC_HEADERS}
                "docmala/Docmala.cpp"
                "docmala/File.cpp" )
                
set_target_properties(libDocmala PROPERTIES PUBLIC_HEADER "${LIBDOCMALA_PUBLIC_HEADERS}")

target_link_libraries(libDocmala extension_system)
set_property(TARGET libDocmala PROPERTY CXX_STANDARD 11)

install(TARGETS libDocmala
        RUNTIME DESTINATION bin
        PUBLIC_HEADER DESTINATION include/docmala
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)
