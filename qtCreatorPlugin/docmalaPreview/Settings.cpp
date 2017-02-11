#include "Settings.h"
#include "DocmalaConstants.h"

#include <QSettings>

void Settings::save(QSettings *settings) const
{
    settings->beginGroup(QLatin1String(docmala::Constants::SETTINGS_GROUP));
    settings->setValue(QLatin1String(docmala::Constants::DOCMALA_DIR), _docmalaInstallDir.absolutePath());

    settings->endGroup();
    settings->sync();
}

void Settings::load(QSettings *settings)
{
    settings->beginGroup(QLatin1String(docmala::Constants::SETTINGS_GROUP));
    QString path = settings->value(QLatin1String(docmala::Constants::DOCMALA_DIR)).toString();
    _docmalaInstallDir.setPath(path);
    settings->endGroup();
}

QDir Settings::docmalaInstallDir() const
{
    return _docmalaInstallDir;
}

void Settings::setDocmalaInstallDir(const QDir &docmalaInstallDir)
{
    _docmalaInstallDir = docmalaInstallDir;
}
