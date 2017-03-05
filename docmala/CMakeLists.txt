
include_directories("../ext/extension_system/src")


set( DOCMALA_PUBLIC_HEADERS
                "docmala/docmala_global.h"
                "docmala/Docmala.h"
                "docmala/DocmaPlugin.h"
                "docmala/File.h"
                "docmala/FileLocation.h"
                "docmala/Parameter.h"
                "docmala/DocumentPart.h"
                "docmala/Error.h"
                "docmala/MetaData.h"
                "docmala/Document.h")

add_library(docmala SHARED
                ${DOCMALA_PUBLIC_HEADERS}
                "docmala/Docmala.cpp"
                "docmala/File.cpp" )
                
set_target_properties(docmala PROPERTIES PUBLIC_HEADER "${DOCMALA_PUBLIC_HEADERS}")

target_link_libraries(docmala extension_system)

install(TARGETS docmala
        RUNTIME DESTINATION bin
        PUBLIC_HEADER DESTINATION include/docmala
        LIBRARY DESTINATION bin
        ARCHIVE DESTINATION lib)
