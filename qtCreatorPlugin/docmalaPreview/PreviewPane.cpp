#include "PreviewPane.h"

#include <QToolButton>
#include <QVBoxLayout>
#include <QWebEngineView>
//#include <QIcon>
#include <QAction>

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

    toolBarLayout->addWidget(linkButton);
    toolBarLayout->addWidget(highlightButton);
    toolBarLayout->addStretch();
    toolBarLayout->addWidget(closeButton);

    layout->addWidget(toolBar);

}

void PreviewPane::addPreview(QLayout *layout)
{
    _preview = new QWebEngineView(nullptr);
//    preview->setPage (new QWebEnginePage());
//    connect(preview->page(), &QWebEnginePage::loadStarted, [this] {
//    });
//    connect(preview->page(), &QWebEnginePage::loadFinished, [this] {
//        emit previewReady();
//    });

//    preview->setStyleSheet (QStringLiteral ("QWebEngineView {background: #FFFFFF;}"));

    layout->addWidget(_preview);
}


} /*Internal*/
} /*docmala*/
