#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <QDialog>
#include <AllSettings.h>

namespace Ui {
class SettingDialog;
}

class SettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingDialog(QWidget *parent = nullptr);
    ~SettingDialog();

private:
    Ui::SettingDialog *ui;

signals:
    void update_settings(AllSettings const&);

private slots:
    void collect_settings();
};

#endif // SETTINGDIALOG_H
