#pragma once
#include <QWebEnginePage>
#include <QWebEngineView>
#include <QWidget>

#include "Settings.h"

namespace docmala {
namespace Internal {

class PreviewPane : public QWidget
{
    Q_OBJECT
public:
    explicit PreviewPane(Settings *settings, QWidget *parent = 0);

    void setPage( QWebEnginePage *page );

signals:
    void hide();
    void followCursorChanged(bool followCursor);
    void highlightCurrentLineChanged(bool highlightCurrentLine);

private:
    void addToolBar(QLayout *layout);
    void addPreview(QLayout *layout);

    Settings* _settings;
    QWebEngineView* _preview;
};

} /*Internal*/
} /*docmala*/
