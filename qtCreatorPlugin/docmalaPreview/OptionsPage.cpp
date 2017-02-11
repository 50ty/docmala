#include "OptionsPage.h"

#include "OptionsDialog.h"

namespace docmala {
namespace Internal {


OptionsPage::OptionsPage(Settings &settings, QObject *parent)
    : IOptionsPage(parent)
    , _settings(settings)
{
    setId("DocmalaSettings");
    setDisplayName(tr("Docmala"));
    setCategory("Docmala");
    setDisplayCategory(tr("Docmala"));
   // setCategoryIcon(Utils::Icon(Constants::ICON_TODO));
}

QWidget *OptionsPage::widget()
{
    if (!_widget) {
        _widget = new OptionsDialog;
        _widget->setInstallDir(_settings.docmalaInstallDir().path());
    }
    return _widget;}

void OptionsPage::apply()
{
    if( _settings.docmalaInstallDir().path() != _widget->installDir() )
    {
        _settings.setDocmalaInstallDir(QDir(_widget->installDir()));
        emit settingsChanged(_settings);
    }
}

void OptionsPage::finish()
{
    delete _widget;
}

}
}
