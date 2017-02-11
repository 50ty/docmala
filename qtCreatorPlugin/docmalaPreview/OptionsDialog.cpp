#include "OptionsDialog.h"
#include "ui_OptionsDialog.h"

#include <QFileDialog>

OptionsDialog::OptionsDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

QString OptionsDialog::installDir() const
{
    return ui->pathEdit->text();
}

void OptionsDialog::setInstallDir(const QString &installDir)
{
    ui->pathEdit->setText(installDir);
}

void OptionsDialog::on_browseButton_clicked()
{
    ui->pathEdit->setText( QFileDialog::getExistingDirectory(this, "Select Docmala installation directory") );
}
