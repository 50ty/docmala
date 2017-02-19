#pragma once

#include "Settings.h"

#include <QMutex>
#include <QObject>
#include <QThread>
#include <QTimer>

#include <docmala/Docmala.h>
#include <docmala/HtmlOutput.h>

namespace docmala {
namespace Internal {

class Renderer : public QObject
{
    Q_OBJECT
public:
    struct RenderInput {
        QString fileName;
        QByteArray document;
    };

    struct RenderResult {
        HtmlOutput::HtmlDocument document;
        std::vector<docmala::Error> errors;
    };

    Renderer(Settings *settings);
    ~Renderer() override;

    void update(const RenderInput& input);

    RenderResult result() const;
private slots:
    void internalRenderingFinished();
signals:
    void renderingFinished();

private:
    void render();

    Settings* _settings;
    mutable QMutex _mutex;
    QThread _renderThread;
    QTimer _renderTimer;
    RenderInput _lastInput;
    RenderInput _lastRenderedInput;
    QScopedPointer<Docmala> _docmala;
    QString _pluginDir;
    RenderResult _renderResult;
};

} /*Internal*/
} /*docmala*/
