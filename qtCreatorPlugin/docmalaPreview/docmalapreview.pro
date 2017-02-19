DEFINES += Docmala_LIBRARY

# Docmala files

QT += webenginewidgets websockets

SOURCES += \
    DocmalaPlugin.cpp \
    OptionsPage.cpp \
    Settings.cpp \
    OptionsDialog.cpp \
    PreviewPane.cpp \
    PreviewPage.cpp \
    Renderer.cpp

HEADERS += \
    DocmalaPlugin.h \
    DocmalaConstants.h \
    Docmala_global.h \
    OptionsPage.h \
    Settings.h \
    OptionsDialog.h \
    PreviewPane.h \
    PreviewPage.h \
    Renderer.h

LIBS += -L"$$_PRO_FILE_PWD_/ext/lib/" -llibDocmala -lhtmlOutputPlugin
INCLUDEPATH += "$$_PRO_FILE_PWD_/ext/include/"

# Qt Creator linking

## Either set the IDE_SOURCE_TREE when running qmake,
## or set the QTC_SOURCE environment variable, to override the default setting
isEmpty(IDE_SOURCE_TREE): IDE_SOURCE_TREE = $$(QTC_SOURCE)
isEmpty(IDE_SOURCE_TREE): IDE_SOURCE_TREE = "$$_PRO_FILE_PWD_/../../../qt-creator"

## Either set the IDE_BUILD_TREE when running qmake,
## or set the QTC_BUILD environment variable, to override the default setting
isEmpty(IDE_BUILD_TREE): IDE_BUILD_TREE = $$(QTC_BUILD)
isEmpty(IDE_BUILD_TREE): IDE_BUILD_TREE = "$$_PRO_FILE_PWD_/../../../qt-creator-build"

## uncomment to build plugin into user config directory
## <localappdata>/plugins/<ideversion>
##    where <localappdata> is e.g.
##    "%LOCALAPPDATA%\QtProject\qtcreator" on Windows Vista and later
##    "$XDG_DATA_HOME/data/QtProject/qtcreator" or "~/.local/share/data/QtProject/qtcreator" on Linux
##    "~/Library/Application Support/QtProject/Qt Creator" on OS X
# USE_USER_DESTDIR = yes

###### If the plugin can be depended upon by other plugins, this code needs to be outsourced to
###### <dirname>_dependencies.pri, where <dirname> is the name of the directory containing the
###### plugin's sources.

QTC_PLUGIN_NAME = Docmala
QTC_LIB_DEPENDS += \
    # nothing here at this time

QTC_PLUGIN_DEPENDS += \
    coreplugin \
    texteditor \
    projectexplorer

QTC_PLUGIN_RECOMMENDS += \
    # optional plugin dependencies. nothing here at this time

###### End _dependencies.pri contents ######

include($$IDE_SOURCE_TREE/src/qtcreatorplugin.pri)

FORMS += \
    OptionsDialog.ui
