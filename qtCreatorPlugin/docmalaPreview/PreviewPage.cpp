#include "PreviewPage.h"
#include <QWebSocket>

namespace docmala {
namespace Internal {

PreviewPage::PreviewPage()
    : _webSocketServer("docmalaPreviewServer", QWebSocketServer::NonSecureMode)
{
    _webSocketServer.listen();

    connect( &_webSocketServer, &QWebSocketServer::newConnection, [this] {
        _clientConnection = _webSocketServer.nextPendingConnection();
        update();
    });

}

void PreviewPage::updateContent(const HtmlOutput::HtmlDocument &html)
{
    _currentContent = html;
    if( !_skeletonPresent ) {
        QString document = "<!doctype html><html><head>";
        document += QString::fromStdString(html.head) + "</head>";

        document += "<body>" "\n"
                    "  <div id=\"placeholder\"></div>" "\n"
                    "  <script>" "\n"
                    "    'use strict';" "\n"
                    "    var placeholder = document.getElementById('placeholder');" "\n"
                    "    var lastStyle;" "\n"
                    "    var lastElement = null;" "\n"
                    "    var lastLine = -1;" "\n"
                    "    function highlightLine(line) { " "\n"
                    "      lastLine = line;" "\n"
                    "      if( lastElement ) lastElement.style = lastStyle;" "\n"
                    "      var myElement = document.querySelector(\"#line_\"+line.toString());" "\n"
                    "      if( myElement ) {" "\n"
                    "        lastStyle = myElement.style;" "\n"
                    "        myElement.style.border = \"2px solid grey\";" "\n"
                    "        myElement.style.borderRadius = \"6px\";" "\n"
                    "        myElement.style.background = \"lightgrey\";" "\n"
                    "      }" "\n"
                    "      lastElement = myElement;" "\n"
                    "    }" "\n"
                    "    function scrollToLine(line) { " "\n"
                    "      var myElement = document.querySelector(\"#line_\"+line.toString());" "\n"
                    "      if( myElement ) {" "\n"
                    "        const elementRect = myElement.getBoundingClientRect();" "\n"
                    "        const absoluteElementTop = elementRect.top + window.pageYOffset;" "\n"
                    "        const middle = absoluteElementTop - (window.innerHeight / 2);" "\n"
                    "        window.scrollTo(0, middle);" "\n"
                    "      }" "\n"
                    "    }" "\n"
                    "    var updateText = function(message) {" "\n"
                    "      placeholder.innerHTML = message.data;" "\n"
                    "      var blocks = placeholder.querySelectorAll('pre code');" "\n"
                    "      var ArrayProto = [];" "\n"
                    "      ArrayProto.forEach.call(blocks, hljs.highlightBlock);" "\n"
                    "      highlightLine(lastLine);" "\n"
                    "    }" "\n"
                    "    var webSocket = new WebSocket(\"ws://localhost:"+ QString::number(_webSocketServer.serverPort())+"\");" "\n"
                    "    webSocket.onmessage = updateText;"
                    "  </script>" "\n"
                    "</body>" "\n";

        setHtml( document, QUrl() );
        _skeletonPresent = true;
    }
    update();
}

void PreviewPage::resetContent()
{
    _skeletonPresent = false;
    _clientConnection->deleteLater();
    _clientConnection = nullptr;
}

void PreviewPage::highlightLine(int line)
{
    runJavaScript(QString("highlightLine(") + QString::number(line) + ");");
}

void PreviewPage::scrollToLine(int line)
{
    runJavaScript(QString("scrollToLine(") + QString::number(line) + ");");
}

void PreviewPage::update()
{
    if( _clientConnection ) {
        _clientConnection->sendTextMessage(QString::fromStdString(_currentContent.body));
    }
}

} /*Internal*/
} /*docmala*/
