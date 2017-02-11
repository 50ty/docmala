#pragma once

#include <coreplugin/dialogs/ioptionspage.h>

#include <QPointer>

#include "Settings.h"

class OptionsDialog;

namespace docmala {
namespace Internal {

class OptionsPage : public Core::IOptionsPage
{
    Q_OBJECT

public:
    OptionsPage(Settings &settings, QObject *parent = 0);

    QWidget *widget();
    void apply();
    void finish();

signals:
    void settingsChanged(const Settings &settings);

private:
    QPointer<OptionsDialog> _widget;
    Settings &_settings;
};

} // namespace Internal
} // namespace Docmala
