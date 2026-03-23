#include "mainwindow.h"
#include "editor/editor.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    addEditor();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addEditor()
{
    ui->tabWidget->addTab(new Editor(ui->tabWidget), "editor");
}
