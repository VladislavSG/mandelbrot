#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPushButton>
#include <QDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , DialogWindow(new SettingDialog(this))
{
    ui->setupUi(this);
    connect(ui->pushButtonSettings, &QPushButton::clicked, this, &MainWindow::setting_show);
    connect(DialogWindow, &SettingDialog::update_settings, ui->widget, &mandelbrot_widget::settingsUpdate);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setting_show()
{
    DialogWindow->show();
}
