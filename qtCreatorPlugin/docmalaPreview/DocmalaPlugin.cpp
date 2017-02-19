#include "DocmalaPlugin.h"
#include "DocmalaConstants.h"
#include "PreviewPane.h"

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/rightpane.h>

#include <coreplugin/editormanager/editormanager.h>

#include <texteditor/texteditor.h>


#include <QAction>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenu>
#include <QCoreApplication>
#include <QVBoxLayout>
#include <QMutexLocker>
#include <QToolButton>

#include <texteditor/texteditor.h>
#include <texteditor/textdocument.h>
#include <texteditor/normalindenter.h>
#include <texteditor/texteditorconstants.h>
#include <projectexplorer/taskhub.h>

#include <utils/styledbar.h>
#include <utils/utilsicons.h>

#include "OptionsPage.h"

#include <docmala/Docmala.h>
#include <docmala/HtmlOutput.h>

namespace docmala {
namespace Internal {

DocmalaPlugin::DocmalaPlugin()
{
    // Create your members
}

DocmalaPlugin::~DocmalaPlugin()
{
}

bool DocmalaPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    // Register objects in the plugin manager's object pool
    // Load settings
    // Add actions to menus
    // Connect to other plugins' signals
    // In the initialize function, a plugin can be sure that the plugins it
    // depends on have initialized their members.
    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

    ProjectExplorer::TaskHub::addCategory(Constants::DOCMALA_TASK_ID, "Documentation errors", true);

    _settings.load();
    
    createOptionsPage();

    QAction *action = new QAction(tr("Preview"), this);
    Core::Command *cmd = Core::ActionManager::registerAction(action, Constants::Docmala_ID,
                                                             Core::Context(Core::Constants::C_GLOBAL));
    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Meta+X")));

    connect(action, &QAction::triggered, [this]() {
        showPreview(!Core::RightPaneWidget::instance()->isShown());
    });

    Core::ActionContainer *menu = Core::ActionManager::createMenu(Constants::Docmala_MENU_ID);
    menu->menu()->setTitle(tr("Docmala"));
    menu->addAction(cmd);
    Core::ActionManager::actionContainer(Core::Constants::M_TOOLS)->addMenu(menu);

    connect(Core::ICore::instance(),
            &Core::ICore::saveSettingsRequested,
            [this] { _settings.save(); });

    connect(Core::EditorManager::EditorManager::instance(),
            &Core::EditorManager::EditorManager::currentEditorChanged,
            this,
            &DocmalaPlugin::currentEditorChanged );

    _previewPane = new PreviewPane(&_settings);
    connect( _previewPane, &PreviewPane::hide, [this] {
        showPreview(false);
    });

    connect( _previewPane, &PreviewPane::highlightCurrentLineChanged, [this] {
        updateHighlight();
    });

    connect( _previewPane, &PreviewPane::followCursorChanged, [this] {
        updateHighlight();
    });

    _previewPage = new PreviewPage();
    _previewPane->setPage(_previewPage);
    _renderer = new Renderer(&_settings);

    connect( _renderer, &Renderer::renderingFinished, this, &DocmalaPlugin::renderingFinished);

    Core::RightPaneWidget::instance()->readSettings(Core::ICore::settings());
    _paneWasVisible = Core::RightPaneWidget::instance()->isShown();
    Core::RightPaneWidget::instance()->setWidget(_previewPane);

    updatePreview();

    return true;
}

void DocmalaPlugin::extensionsInitialized()
{
    // Retrieve objects from the plugin manager's object pool
    // In the extensionsInitialized function, a plugin can be sure that all
    // plugins that depend on it are completely initialized.
}

ExtensionSystem::IPlugin::ShutdownFlag DocmalaPlugin::aboutToShutdown()
{
    // Save settings
    // Disconnect from signals that are not needed during shutdown
    // Hide UI (if you add UI that is not in the main window directly)

    _settings.save();
    Core::RightPaneWidget::instance()->saveSettings(Core::ICore::settings());
    delete _previewPage;
    delete _previewPane;
    delete _renderer;
    return SynchronousShutdown;
}

void DocmalaPlugin::triggerAction()
{
    QMessageBox::information(Core::ICore::mainWindow(),
                             tr("Action Triggered"),
                             tr("This is an action from Docmala."));
}

void DocmalaPlugin::createOptionsPage()
{
    auto page = new OptionsPage(_settings);
    addAutoReleasedObject(page);
    connect(page, &OptionsPage::settingsChanged, this, &DocmalaPlugin::settingsChanged);
}

void DocmalaPlugin::settingsChanged()
{
    _settings.save();
}

void DocmalaPlugin::currentEditorChanged()
{
    auto document = Core::EditorManager::instance()->currentDocument();
    disconnect(_documentChangedConnection);
    if( document ) {
        _document = document;
        _documentChangedConnection = connect( document, &Core::IDocument::contentsChanged, this, &DocmalaPlugin::updatePreview );

        documentChanged();
    }

    auto editor = TextEditor::BaseTextEditor::currentTextEditor();
    if( editor != nullptr ) {
         connect( editor->editorWidget(), &QPlainTextEdit::cursorPositionChanged, [this] {
            updateHighlight();
         });
    }
}

void DocmalaPlugin::documentChanged()
{
    if( !_document )
        return;

    if( !_document->filePath().endsWith(".dml") ) {
        _paneWasVisible = Core::RightPaneWidget::instance()->isShown();
        showPreview(false);
        return;
    } else {
        showPreview(_paneWasVisible);
    }

    if( !Core::RightPaneWidget::instance()->isShown() )
        return;

    updatePreview();
}

void DocmalaPlugin::updatePreview()
{
    if( _document ) {
        _renderer->update({_document->filePath().toString(), _document->contents()});
    }
}
void DocmalaPlugin::updateHighlight()
{
    auto editor = TextEditor::BaseTextEditor::currentTextEditor();
    if( editor ) {
        if( _settings.highlightCurrentLine() ) {
           _previewPage->highlightLine(editor->editorWidget()->textCursor().blockNumber()+1);
        } else {
            _previewPage->highlightLine(-1);
        }

        if( _settings.followCursor() ) {
            _previewPage->scrollToLine(editor->editorWidget()->textCursor().blockNumber()+1);
        }
    }
}

void DocmalaPlugin::showPreview(bool show)
{
    if( show ) {
        updatePreview();
    }
    Core::RightPaneWidget::instance()->setShown(show);
}

void DocmalaPlugin::renderingFinished()
{
    auto result = _renderer->result();
    _previewPage->updateContent(result.document);
    updateHighlight();

    ProjectExplorer::TaskHub::clearTasks(Constants::DOCMALA_TASK_ID);
    for( auto error : result.errors ) {
        ProjectExplorer::TaskHub::addTask(ProjectExplorer::Task(
                                              ProjectExplorer::Task::Error,
                                              QString::fromStdString(error.message),
                                              Utils::FileName::fromString(QString::fromStdString(error.location.fileName)),
                                              error.location.line,
                                              Constants::DOCMALA_TASK_ID));
        for( auto extended : error.extendedInformation ) {
            ProjectExplorer::TaskHub::addTask(ProjectExplorer::Task(
                                                  ProjectExplorer::Task::Unknown,
                                                  QString("    ") + QString::fromStdString(extended.message),
                                                  Utils::FileName::fromString(QString::fromStdString(extended.location.fileName)),
                                                  extended.location.line,
                                                  Constants::DOCMALA_TASK_ID));
        }
    }

    Core::EditorManager::activateEditor(Core::EditorManager::currentEditor());
}

} // namespace Internal
} // namespace Docmala
