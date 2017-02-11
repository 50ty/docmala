#pragma once

#include <QtGlobal>
#include <QDir>

QT_FORWARD_DECLARE_CLASS(QSettings)

class Settings
{
public:
    void save(QSettings *settings) const;
    void load(QSettings *settings);
    
    QDir docmalaInstallDir() const;
    void setDocmalaInstallDir(const QDir &docmalaInstallDir);
    
private:
    QDir _docmalaInstallDir;
};
