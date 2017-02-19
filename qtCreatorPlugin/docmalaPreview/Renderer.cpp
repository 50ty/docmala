#include "Renderer.h"

#include <docmala/HtmlOutput.h>

namespace docmala {
namespace Internal {

Renderer::Renderer(Settings *settings)
    : _settings(settings)
{
    _renderThread.setObjectName("docmalaRenderer");
    _renderThread.start();

    _renderTimer.moveToThread(&_renderThread);
    _renderTimer.setSingleShot(true);
    _renderTimer.setInterval(100);
    connect( &_renderTimer, &QTimer::timeout, this, &Renderer::render, Qt::DirectConnection);
}

Renderer::~Renderer()
{
    _renderThread.quit();
    _renderThread.wait(200);
    if( _renderThread.isRunning() ) {
        _renderThread.terminate();
    }
}

void Renderer::update(const Renderer::RenderInput &input)
{
    {
        QMutexLocker locker(&_mutex);
        _lastInput = input;
        _pluginDir = _settings->docmalaInstallDir().path() + "/plugins";
    }
    QMetaObject::invokeMethod(&_renderTimer, "start", Qt::QueuedConnection);
}

Renderer::RenderResult Renderer::result() const
{
    QMutexLocker locker(&_mutex);
    RenderResult result = _renderResult;
    return result;
}

void Renderer::internalRenderingFinished()
{
    emit renderingFinished();
}

void Renderer::render()
{
    RenderInput input;
    QString pluginDir;
    {
        QMutexLocker locker(&_mutex);
        input = _lastInput;
        pluginDir = _pluginDir;
    }

    if( _lastRenderedInput.fileName != input.fileName ) {
        _docmala.reset(new docmala::Docmala(pluginDir.toStdString()));
    }

    _docmala->parseData(input.document.toStdString(), input.fileName.toStdString());

    HtmlOutput htmlOutput;
    ParameterList parameters;
    parameters.insert(std::make_pair("embedImages", Parameter{"embedImages", "", FileLocation() } ));
    parameters.insert(std::make_pair("pluginDir", Parameter{"pluginDir", _docmala->pluginDir(), FileLocation()} ) );

    auto html = htmlOutput.produceHtml(parameters, _docmala->document(), "" );//scripts));

    _lastRenderedInput = input;

    {
        QMutexLocker locker(&_mutex);
        _renderResult.document = html;
        _renderResult.errors = _docmala->errors();
    }

    QMetaObject::invokeMethod(this, "renderingFinished", Qt::QueuedConnection);
}

} /*Internal*/
} /*docmala*/
