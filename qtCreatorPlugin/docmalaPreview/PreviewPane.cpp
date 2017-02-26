#include "PreviewPane.h"

#include <QToolButton>
#include <QVBoxLayout>
#include <QWebEngineView>
#include <QtMath>
#include <QAction>
#include <QWheelEvent>
#include <QComboBox>
#include <QDoubleSpinBox>

#include <utils/styledbar.h>
#include <utils/utilsicons.h>


namespace docmala {
namespace Internal {

PreviewPane::PreviewPane(Settings *settings, QWidget *parent)
    : QWidget(parent)
    , _settings(settings)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

    addToolBar(layout);
    addPreview(layout);
}

void PreviewPane::setPage(QWebEnginePage *page)
{
    _preview->setPage(page);
    _preview->setZoomFactor(_settings->zoom());
    //page->setZoomFactor(_settings->zoom());
}

void PreviewPane::wheelEvent(QWheelEvent *event)
{
    if( event->modifiers() & Qt::CTRL ) {
        double factor = event->angleDelta().ry() / 360.0;
        factor = qPow(2, factor);

        setZoomFactor( factor * _preview->zoomFactor() );
        _zoomComboBox->setCurrentText( QString::number(static_cast<int>(_preview->zoomFactor() * 100)) + " %");
    }
}

void PreviewPane::addToolBar(QLayout *layout)
{
    auto toolBar = new Utils::StyledBar();

    auto closeAction = new QAction(Utils::Icons::CLOSE_TOOLBAR.icon(), QString(), toolBar);
    auto linkAction = new QAction(Utils::Icons::LINK.icon(), QString(), toolBar);
    auto highlightAction = new QAction(Utils::Icons::EYE_OPEN_TOOLBAR.icon(), QString(), toolBar);

    connect(closeAction, &QAction::triggered, [this]() {
        emit hide();
    });

    connect(linkAction, &QAction::toggled, [this](bool toggled) {
        _settings->setFollowCursor(toggled);
        emit followCursorChanged(toggled);
    });

    connect(highlightAction, &QAction::toggled, [this](bool toggled) {
        _settings->setHighlightCurrentLine(toggled);
        emit highlightCurrentLineChanged(toggled);
    });

    linkAction->setCheckable(true);
    linkAction->setChecked(_settings->followCursor());

    highlightAction->setCheckable(true);
    highlightAction->setChecked(_settings->highlightCurrentLine());

    auto toolBarLayout = new QHBoxLayout(toolBar);
    toolBarLayout->setMargin(0);
    toolBarLayout->setSpacing(0);

    auto closeButton = new QToolButton();
    auto linkButton = new QToolButton();
    auto highlightButton = new QToolButton();

    closeButton->setDefaultAction(closeAction);
    linkButton->setDefaultAction(linkAction);
    highlightButton->setDefaultAction(highlightAction);

    _zoomComboBox = new QComboBox();
    _zoomComboBox->addItem("50 %");
    _zoomComboBox->addItem("75 %");
    _zoomComboBox->addItem("100 %");
    _zoomComboBox->addItem("150 %");
    _zoomComboBox->addItem("200 %");
    _zoomComboBox->addItem("300 %");
    _zoomComboBox->addItem("400 %");
    _zoomComboBox->addItem("500 %");
    _zoomComboBox->setEditable(true);

    connect(_zoomComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [this](int index) {
        switch (index) {
        case 0: setZoomFactor(0.5);
            break;
        case 1: setZoomFactor(0.75);
            break;
        case 2: setZoomFactor(1.0);
            break;
        case 3: setZoomFactor(1.5);
            break;
        case 4: setZoomFactor(2.0);
            break;
        case 5: setZoomFactor(3.0);
            break;
        case 6: setZoomFactor(4.0);
            break;
        case 7: setZoomFactor(5.0);
            break;
        }
    });

    connect(_zoomComboBox, &QComboBox::editTextChanged, [this](QString text){
        bool ok = false;
        int zoom = text.toInt(&ok);
        if( ok ) {
            setZoomFactor(zoom / 100.0);
        }
    });
    toolBarLayout->addWidget(linkButton);
    toolBarLayout->addWidget(highlightButton);
    toolBarLayout->addSpacing(10);
    toolBarLayout->addWidget(_zoomComboBox);
    toolBarLayout->addStretch();
    toolBarLayout->addWidget(closeButton);

    layout->addWidget(toolBar);

}

void PreviewPane::addPreview(QLayout *layout)
{
    _preview = new QWebEngineView(nullptr);
    layout->addWidget(_preview);
    _preview->setZoomFactor(_settings->zoom());
    _zoomComboBox->setCurrentText( QString::number(static_cast<int>(_preview->zoomFactor() * 100)) + " %");
}

void PreviewPane::setZoomFactor(double zoom)
{
    _preview->setZoomFactor(zoom);
    _settings->setZoom(zoom);
}


} /*Internal*/
} /*docmala*/
