#include "settingdialog.h"
#include "ui_settingdialog.h"
#include <QDialogButtonBox>

SettingDialog::SettingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingDialog)
{
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SettingDialog::collect_settings);
}

SettingDialog::~SettingDialog()
{
    delete ui;
}

void SettingDialog::collect_settings()
{
    emit update_settings({ui->spinBox->value(),
                          static_cast<size_t>(ui->horizontalSlider->value())});
}
