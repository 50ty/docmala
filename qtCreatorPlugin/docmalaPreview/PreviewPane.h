#pragma once
#include <QWebEnginePage>
#include <QWebEngineView>
#include <QWidget>
#include <QComboBox>

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

protected:
    void wheelEvent(QWheelEvent *event) override;

private:
    void addToolBar(QLayout *layout);
    void addPreview(QLayout *layout);
    void setZoomFactor(double zoom);

    Settings* _settings;
    QWebEngineView* _preview;
    QComboBox* _zoomComboBox;
};

} /*Internal*/
} /*docmala*/
