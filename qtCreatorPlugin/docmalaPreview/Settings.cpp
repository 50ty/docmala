#include "Settings.h"
#include "DocmalaConstants.h"

#include <coreplugin/icore.h>

#include <QSettings>

void Settings::save() const
{
    auto settings = Core::ICore::settings();
    settings->beginGroup(QLatin1String(docmala::Constants::SETTINGS_GROUP));
    settings->setValue(QLatin1String(docmala::Constants::DOCMALA_DIR), _docmalaInstallDir.absolutePath());
    settings->setValue(QLatin1String(docmala::Constants::HIGHLIGHT_CURRENT_LINE), _highlightCurrentLine);
    settings->setValue(QLatin1String(docmala::Constants::FOLLOW_CURSOR), _followCursor);
    settings->setValue(QLatin1String(docmala::Constants::ZOOM), _zoom);

    settings->endGroup();
    settings->sync();
}

void Settings::load()
{
    auto settings = Core::ICore::settings();
    settings->beginGroup(QLatin1String(docmala::Constants::SETTINGS_GROUP));
    QString path = settings->value(QLatin1String(docmala::Constants::DOCMALA_DIR)).toString();
    _docmalaInstallDir.setPath(path);

    _highlightCurrentLine = settings->value(QLatin1String(docmala::Constants::HIGHLIGHT_CURRENT_LINE), true).toBool();
    _followCursor = settings->value(QLatin1String(docmala::Constants::FOLLOW_CURSOR), true).toBool();
    _zoom = settings->value(QLatin1String(docmala::Constants::ZOOM), 1.0).toDouble();
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

bool Settings::highlightCurrentLine() const
{
    return _highlightCurrentLine;
}

void Settings::setHighlightCurrentLine(bool highlightCurrentLine)
{
    _highlightCurrentLine = highlightCurrentLine;
}

bool Settings::followCursor() const
{
    return _followCursor;
}

void Settings::setFollowCursor(bool followCursor)
{
    _followCursor = followCursor;
}

double Settings::zoom() const
{
    return _zoom;
}

void Settings::setZoom(double zoom)
{
    _zoom = zoom;
}
