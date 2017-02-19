#pragma once
#include <QWebEnginePage>
#include <QWebSocketServer>
#include <QWidget>

#include <docmala/HtmlOutput.h>

#include "Settings.h"

namespace docmala {
namespace Internal {

class PreviewPage : public QWebEnginePage
{
public:
    PreviewPage();
    void updateContent( const HtmlOutput::HtmlDocument& html);
    void resetContent();
    void highlightLine(int line);
    void scrollToLine(int line);
private:
    void update();

    bool _skeletonPresent = false;
    HtmlOutput::HtmlDocument _currentContent;
    QWebSocketServer _webSocketServer;
    QWebSocket* _clientConnection = nullptr;
};

} /*Internal*/
} /*docmala*/
