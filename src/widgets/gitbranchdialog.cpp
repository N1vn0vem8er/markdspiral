#include "gitbranchdialog.h"
#include "ui_gitbranchdialog.h"

GitBranchDialog::GitBranchDialog(const QString &path, const QStringList &branches, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::GitBranchDialog)
{
    ui->setupUi(this);
    repoPath = path;
    ui->comboBox->addItems(branches);
    connect(ui->pushButton, &QPushButton::clicked, this, [&](){emit setBranch(ui->comboBox->currentText());});
}

GitBranchDialog::~GitBranchDialog()
{
    delete ui;
}
