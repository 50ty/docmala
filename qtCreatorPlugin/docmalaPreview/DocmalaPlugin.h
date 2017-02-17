#pragma once

#include <QtWebEngineWidgets/QWebEngineView>

#include "Docmala_global.h"
#include "Settings.h"
#include <docmala/Error.h>

#include <extensionsystem/iplugin.h>
#include <texteditor/texteditor.h>
#include <coreplugin/idocument.h>

#include <QThread>
#include <QTimer>
#include <QMutex>

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

    void documentChanged();
    void updatePreview();

    void render();


    QWebEngineView* _preview = nullptr;
    QWidget* _previewPane = nullptr;
    bool _showPreviewPane = false;
    Settings _settings;
    Core::IDocument* _document;
    QScopedPointer<Docmala> _docmala;
    QMetaObject::Connection _documentChangedConnection;
    QThread _renderThread;
    QTimer _renderTimer;
    QMutex _renderDataMutex;
    QByteArray _renderContent;
    QString _renderFileName;
    std::string _renderLastFileName;
    QString _renderRenderedHTML;
    std::vector<docmala::Error> _renderOccuredErrors;
    bool _previewFollowCursor = true;
    bool _previewHighlightLine = true;
};

} // namespace Internal
} // namespace Docmala
