#pragma once

#include <QWidget>

namespace Ui {
class OptionsDialog;
}

class OptionsDialog : public QWidget
{
    Q_OBJECT

public:
    explicit OptionsDialog(QWidget *parent = 0);
    ~OptionsDialog();

    QString installDir() const;
    void setInstallDir(const QString &installDir);

private slots:
    void on_browseButton_clicked();

private:
    Ui::OptionsDialog *ui;
};

