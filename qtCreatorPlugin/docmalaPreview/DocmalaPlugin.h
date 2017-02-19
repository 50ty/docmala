#pragma once

#include "Docmala_global.h"
#include "PreviewPage.h"
#include "PreviewPane.h"
#include "Renderer.h"
#include "Settings.h"

#include <extensionsystem/iplugin.h>
#include <coreplugin/idocument.h>

namespace docmala {
class Docmala;

namespace Internal {

class DocmalaPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "Docmala.json")

public:
    DocmalaPlugin();
    ~DocmalaPlugin();

    bool initialize(const QStringList &arguments, QString *errorString);
    void extensionsInitialized();
    ShutdownFlag aboutToShutdown();

    void showPreview(bool show);

private slots:
    void renderingFinished();

private:
    void triggerAction();
    void createOptionsPage();
    void settingsChanged();

    void currentEditorChanged();
    void documentChanged();
    void updatePreview();
    void updateHighlight();

    QWebEngineView* _preview = nullptr;
    PreviewPane* _previewPane = nullptr;
    PreviewPage* _previewPage = nullptr;
    Renderer* _renderer = nullptr;

    Settings _settings;
    bool _paneWasVisible = false;
    Core::IDocument* _document = nullptr;
    QMetaObject::Connection _documentChangedConnection;
};

} // namespace Internal
} // namespace Docmala
