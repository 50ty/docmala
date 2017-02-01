
include_directories("../ext/extension_system/src")

add_library(libDocmala SHARED
                "docmala/Docmala.h"
                "docmala/Docmala.cpp"
                "docmala/DocmaPlugin.h"
                "docmala/FileLocation.h"
                "docmala/Parameter.h"
                "docmala/DocumentPart.h"
                "docmala/Error.h")
                
target_link_libraries(libDocmala extension_system)
set_property(TARGET libDocmala PROPERTY CXX_STANDARD 11)
