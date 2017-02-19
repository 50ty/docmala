#pragma once

#include <QtGlobal>
#include <QDir>

QT_FORWARD_DECLARE_CLASS(QSettings)

class Settings
{
public:
    void save() const;
    void load();
    
    QDir docmalaInstallDir() const;
    void setDocmalaInstallDir(const QDir &docmalaInstallDir);
    
    bool highlightCurrentLine() const;
    void setHighlightCurrentLine(bool highlightCurrentLine);

    bool followCursor() const;
    void setFollowCursor(bool followCursor);

private:
    QDir _docmalaInstallDir;
    bool _highlightCurrentLine;
    bool _followCursor;
};
