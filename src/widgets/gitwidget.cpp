#include "gitwidget.h"
#include "gitfilestatusitemdelegate.h"
#include "ui_gitwidget.h"
#include <processmanager.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qlineedit.h>
#include <qmenu.h>
#include <qprocess.h>
#include "editor/editor.h"

GitWidget::GitWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GitWidget)
{
    ui->setupUi(this);

    connect(ui->refreshButton, &QPushButton::clicked, this, &GitWidget::refresh);
    connect(ui->commitButton, &QPushButton::clicked, this, &GitWidget::openGitCommit);
    connect(ui->gitPushButton, &QPushButton::clicked, this, &GitWidget::gitPush);
    connect(ui->gitPullButton, &QPushButton::clicked, this, &GitWidget::gitPull);

    setVisibility(false);
    ui->addedView->setItemDelegate(new GitFileStatusItemDelegate(ui->addedView));
    ui->changedView->setItemDelegate(new GitFileStatusItemDelegate(ui->changedView));
    ui->untrackedView->setItemDelegate(new GitFileStatusItemDelegate(ui->untrackedView));
    QMenu* changedContextMenu = new QMenu(ui->changedView);
    QAction* gitAdd = new QAction(changedContextMenu);
    gitAdd->setText(tr("Add"));
    changedContextMenu->addAction(gitAdd);
    connect(gitAdd, &QAction::triggered, ui->changedView, &GitFilesView::gitAddPressed);
    connect(ui->changedView, &GitFilesView::gitAdd, this, &GitWidget::gitAdd);
    QAction* gitDiff = new QAction(changedContextMenu);
    gitDiff->setText(tr("Diff"));
    changedContextMenu->addAction(gitDiff);
    connect(gitDiff, &QAction::triggered, ui->changedView, &GitFilesView::gitDiffPressed);
    connect(ui->changedView, &GitFilesView::gitDiff, this, &GitWidget::gitDiff);
    QAction* openFile = new QAction(changedContextMenu);
    openFile->setText(tr("Open"));
    changedContextMenu->addAction(openFile);
    connect(openFile, &QAction::triggered, ui->changedView, &GitFilesView::openFilePressed);
    connect(ui->changedView, &GitFilesView::openFile, this, &GitWidget::openChanged);

    ui->changedView->setContextMenu(changedContextMenu);


    QMenu* untrackedContextMenu = new QMenu(ui->untrackedView);
    QAction* utGitAdd = new QAction(untrackedContextMenu);
    utGitAdd->setText(tr("Add"));
    untrackedContextMenu->addAction(utGitAdd);
    connect(utGitAdd, &QAction::triggered, ui->untrackedView, &GitFilesView::gitAddPressed);
    connect(ui->untrackedView, &GitFilesView::gitAdd, this, &GitWidget::gitAddUntracked);
    QAction* utOpenFile = new QAction(untrackedContextMenu);
    utOpenFile->setText(tr("Open"));
    untrackedContextMenu->addAction(utOpenFile);
    connect(utOpenFile, &QAction::triggered, ui->untrackedView, &GitFilesView::openFilePressed);
    connect(ui->untrackedView, &GitFilesView::openFile, this, &GitWidget::openUntracked);

    ui->untrackedView->setContextMenu(untrackedContextMenu);

    QMenu* addedContextMenu = new QMenu(ui->addedView);
    QAction* addedGitDiff = new QAction(addedContextMenu);
    addedGitDiff->setText(tr("Diff"));
    addedContextMenu->addAction(addedGitDiff);
    connect(addedGitDiff, &QAction::triggered, ui->addedView, &GitFilesView::gitDiffPressed);
    connect(ui->addedView, &GitFilesView::gitDiff, this, &GitWidget::gitDiffAdded);
    QAction* addedOpenFile = new QAction(addedContextMenu);
    addedOpenFile->setText(tr("Open"));
    addedContextMenu->addAction(addedOpenFile);
    connect(addedOpenFile, &QAction::triggered, ui->addedView, &GitFilesView::openFilePressed);
    connect(ui->addedView, &GitFilesView::openFile, this, &GitWidget::openAdded);

    ui->addedView->setContextMenu(addedContextMenu);
}

GitWidget::~GitWidget()
{
    delete ui;
}

void GitWidget::setRepositoryPath(const QString &path)
{
    QProcess process;
    process.setWorkingDirectory(path);
    process.startCommand("git status");
    process.waitForStarted();
    process.waitForFinished();
    process.waitForReadyRead();
    if(process.readAllStandardError().isEmpty())
    {
        repoPath = path;
        setVisibility(true);
        readStatus();
        emit branchNameChanged(getBranchName());
    }
}

void GitWidget::noRepo()
{
    setVisibility(false);
    repoPath.clear();
}

bool GitWidget::hasRepository() const
{
    return !repoPath.isEmpty();
}

void GitWidget::setVisibility(bool val)
{
    ui->gitPullButton->setVisible(val);
    ui->gitPushButton->setVisible(val);
    ui->commitButton->setVisible(val);
    ui->refreshButton->setVisible(val);
    ui->addedLabel->setVisible(val);
    ui->changedLabel->setVisible(val);
    ui->untreckedLabel->setVisible(val);
    ui->addedView->setVisible(val);
    ui->changedView->setVisible(val);
    ui->untrackedView->setVisible(val);
    ui->addedTextLabel->setVisible(val);
    ui->changedTextLabel->setVisible(val);
    ui->untrackedTextLabel->setVisible(val);
    ui->noRepoLabel->setVisible(!val);
}

void GitWidget::readStatus()
{
    QProcess process;
    process.setWorkingDirectory(repoPath);
    process.startCommand("git status -s");
    process.waitForStarted();
    process.waitForFinished();
    process.waitForReadyRead();
    const QString results = process.readAllStandardOutput();
    untrackedFiles = getFilesStatus(QRegularExpression(R"(\?\?\s+(.*))"), results, "??");
    modifiedInIndex = getFilesStatus(QRegularExpression(R"(M.\s+(.*))"), results, "M ");
    modifiedInWorkingDirectory = getFilesStatus(QRegularExpression(R"(.M\s+(.*))"), results, " M");
    addedInIndex = getFilesStatus(QRegularExpression(R"(A.\s+(.*))"), results, "A ");
    addedInWorkingDirectory = getFilesStatus(QRegularExpression(R"(.A\s+(.*))"), results, " A");
    deletedFromIndex = getFilesStatus(QRegularExpression(R"(D.\s+(.*))"), results, "D ");
    deletedFromWorkingDirectory = getFilesStatus(QRegularExpression(R"(.D\s+(.*))"), results, " D");

    const auto diff = readDiff();

    applyDiff(untrackedFiles, diff);
    applyDiff(modifiedInIndex, diff);
    applyDiff(modifiedInWorkingDirectory, diff);
    applyDiff(addedInIndex, diff);
    applyDiff(addedInWorkingDirectory, diff);
    applyDiff(deletedFromIndex, diff);
    applyDiff(deletedFromWorkingDirectory, diff);

    addedModel = new GitFileStatusModel(ui->addedView);
    addedModel->setItems(addedInIndex + modifiedInIndex + deletedFromIndex);
    ui->addedView->setModel(addedModel);
    changedModel = new GitFileStatusModel(ui->changedView);
    changedModel->setItems(modifiedInWorkingDirectory + deletedFromWorkingDirectory + addedInWorkingDirectory);
    ui->changedView->setModel(changedModel);
    untrackedModel = new GitFileStatusModel(ui->untrackedView);
    untrackedModel->setItems(untrackedFiles);
    ui->untrackedView->setModel(untrackedModel);

    ui->changedView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->changedView->setColumnWidth(1, 1);
    ui->changedView->setColumnWidth(2, 1);
    ui->changedView->setColumnWidth(3, 4);
    ui->addedView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->addedView->setColumnWidth(1, 1);
    ui->addedView->setColumnWidth(2, 1);
    ui->addedView->setColumnWidth(3, 4);
    ui->untrackedView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->untrackedView->setColumnWidth(1, 1);
    ui->untrackedView->setColumnWidth(2, 1);
    ui->untrackedView->setColumnWidth(3, 4);
}

QList<GitFileStatus> GitWidget::getFilesStatus(const QRegularExpression &regex, const QString &results, const QString &status) const
{
    QList<GitFileStatus> ret;
    QRegularExpressionMatchIterator iterator = regex.globalMatch(results);
    while(iterator.hasNext())
    {
        auto match = iterator.next();
        for(int i=1;i<match.capturedLength(); i++)
        {
            auto tmp = match.captured(i);
            if(!tmp.isEmpty())
            {
                const QString name = QFileInfo(tmp).fileName();
                ret.append(GitFileStatus(name.isEmpty() ? tmp : name, tmp, status, "0", "0"));
            }
        }
    }
    return ret;
}

QList<QPair<QString, QPair<QString, QString>>> GitWidget::readDiff() const
{
    QProcess process;
    process.setWorkingDirectory(repoPath);
    process.startCommand("git diff --numstat");
    process.waitForStarted();
    process.waitForFinished();
    process.waitForReadyRead();

    QList<QPair<QString, QPair<QString, QString>>> changed;

    const QString results = process.readAllStandardOutput();
    static const QRegularExpression regex(R"((\d+)\t(\d+)\t(.+))");
    QRegularExpressionMatchIterator iterator = regex.globalMatch(results);
    while(iterator.hasNext())
    {
        auto match = iterator.next();
        const QString added = match.captured(1);
        const QString removed = match.captured(2);
        const QString name = match.captured(3);
        if(std::find_if(changed.begin(), changed.end(), [&name](const auto& item){return item.first == name;}) == changed.end())
        {
            changed.append(QPair(name, QPair(added, removed)));
        }
    }
    return changed;
}

void GitWidget::applyDiff(QList<GitFileStatus>& files, const QList<QPair<QString, QPair<QString, QString>>>& diffs) const
{
    for(auto& i : files)
    {
        const auto tmp = std::find_if(diffs.begin(), diffs.end(), [&i](const auto& item){return i.path == item.first;});
        if(tmp != diffs.end())
        {
            i.addedLines = tmp->second.first;
            i.removedLines = tmp->second.second;
        }
    }
}

void GitWidget::refresh()
{
    readStatus();
}

void GitWidget::gitAdd(const QModelIndex& index)
{
    gitAddFile(changedModel->getItems().at(index.row()).path);
}

void GitWidget::gitAddUntracked(const QModelIndex &index)
{
    gitAddFile(untrackedModel->getItems().at(index.row()).path);
}

void GitWidget::gitDiff(const QModelIndex &index)
{
    gitFileDiff(changedModel->getItems().at(index.row()).path);
}

void GitWidget::gitDiffAdded(const QModelIndex &index)
{
    gitFileDiff(addedModel->getItems().at(index.row()).path);
}

void GitWidget::openAdded(const QModelIndex &index)
{
    emit openFile(repoPath + QDir::separator() + addedModel->getItems().at(index.row()).path);
}

void GitWidget::openChanged(const QModelIndex &index)
{
    emit openFile(repoPath + QDir::separator() + changedModel->getItems().at(index.row()).path);
}

void GitWidget::openUntracked(const QModelIndex &index)
{
    emit openFile(repoPath + QDir::separator() + untrackedModel->getItems().at(index.row()).path);
}

void GitWidget::openGitCommit()
{
    QWidget* widget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(widget);
    QLineEdit* title = new QLineEdit(widget);
    title->setPlaceholderText(tr("Enter Title"));
    mainLayout->addWidget(title);
    Editor* editor = new Editor(widget);
    editor->setPlaceholderText(tr("Enter Description"));
    mainLayout->addWidget(editor);
    QPushButton* commitButton = new QPushButton(widget);
    commitButton->setText(tr("Commit"));
    commitButton->setIcon(QIcon::fromTheme("list-add"));
    connect(commitButton, &QPushButton::clicked, this, [this, title, editor]{gitCommit(title->text(), editor->toPlainText());});
    mainLayout->addWidget(commitButton);
    widget->setLayout(mainLayout);
    emit addTab(widget, tr("Git Commit"));
}

void GitWidget::closeRepo()
{
    repoPath.clear();
    if(addedModel)
    {
        addedModel->deleteLater();
        addedModel = nullptr;
    }
    if(changedModel)
    {
        changedModel->deleteLater();
        changedModel = nullptr;
    }
    if(untrackedModel)
    {
        untrackedModel->deleteLater();
        untrackedModel = nullptr;
    }
    untrackedFiles.clear();
    modifiedInWorkingDirectory.clear();
    modifiedInIndex.clear();
    addedInIndex.clear();
    addedInWorkingDirectory.clear();
    deletedFromWorkingDirectory.clear();
    deletedFromIndex.clear();
    setVisibility(false);
}

void GitWidget::gitCommit(const QString &title, const QString &description)
{
    QProcess* process = new QProcess(this);
    process->setWorkingDirectory(repoPath);
    process->setProcessChannelMode(QProcess::MergedChannels);
    ProcessManager::getInstance()->registerProcess(process, tr("Git commit"));
    connect(process, &QProcess::readyReadStandardOutput, this, [this, process]{
        refresh();
        emit openInEditor(process->readAllStandardOutput(), tr("Commit results"));
    });
    connect(process, &QProcess::finished, this, [process]{process->deleteLater();});
    process->start("git", {"commit", "-m", title, "-m", description});
    process->waitForStarted();
}

void GitWidget::gitPush()
{
    QProcess* process = new QProcess(this);
    process->setWorkingDirectory(repoPath);
    process->setProcessChannelMode(QProcess::MergedChannels);
    ProcessManager::getInstance()->registerProcess(process, tr("Git push"));
    connect(process, &QProcess::readyReadStandardOutput, this, [this, process]{
        refresh();
        emit openInEditor(process->readAllStandardOutput(), tr("Push results"));
    });
    connect(process, &QProcess::finished, this, [process]{process->deleteLater();});
    process->start("git", {"push"});
    process->waitForStarted();
}

void GitWidget::gitPull()
{
    QProcess* process = new QProcess(this);
    process->setWorkingDirectory(repoPath);
    process->setProcessChannelMode(QProcess::MergedChannels);
    ProcessManager::getInstance()->registerProcess(process, tr("Git pull"));
    connect(process, &QProcess::readyReadStandardOutput, this, [this, process]{
        refresh();
        emit openInEditor(process->readAllStandardOutput(), tr("Pull results"));
    });
    connect(process, &QProcess::finished, this, [process]{process->deleteLater();});
    process->start("git", {"pull"});
    process->waitForStarted();
}

void GitWidget::gitStatus()
{
    QProcess* process = new QProcess(this);
    process->setWorkingDirectory(repoPath);
    process->setProcessChannelMode(QProcess::MergedChannels);
    ProcessManager::getInstance()->registerProcess(process, tr("Git status"));
    connect(process, &QProcess::readyReadStandardOutput, this, [this, process]{
        refresh();
        emit openInEditor(process->readAllStandardOutput(), tr("Status results"));
    });
    connect(process, &QProcess::finished, this, [process]{process->deleteLater();});
    process->start("git", {"status"});
    process->waitForStarted();
}

void GitWidget::gitLog()
{
    QProcess* process = new QProcess(this);
    process->setWorkingDirectory(repoPath);
    process->setProcessChannelMode(QProcess::MergedChannels);
    ProcessManager::getInstance()->registerProcess(process, tr("Git log"));
    connect(process, &QProcess::readyReadStandardOutput, this, [this, process]{
        refresh();
        emit openInEditor(process->readAllStandardOutput(), tr("Log results"));
    });
    connect(process, &QProcess::finished, this, [process]{process->deleteLater();});
    process->start("git", {"log"});
    process->waitForStarted();
}

void GitWidget::gitFetch()
{
    QProcess* process = new QProcess(this);
    process->setWorkingDirectory(repoPath);
    process->setProcessChannelMode(QProcess::MergedChannels);
    ProcessManager::getInstance()->registerProcess(process, tr("Git fetch"));
    connect(process, &QProcess::readyReadStandardOutput, this, [this, process]{
        refresh();
        emit openInEditor(process->readAllStandardOutput(), tr("Fetch results"));
    });
    connect(process, &QProcess::finished, this, [process]{
        process->deleteLater();
    });
    process->start("git", {"fetch"});
    process->waitForStarted();
}

QString GitWidget::getRepoPath() const
{
    return repoPath;
}

QString GitWidget::getBranchName() const
{
    QProcess process;
    process.setWorkingDirectory(repoPath);
    process.startCommand("git rev-parse --abbrev-ref HEAD");
    process.waitForStarted(3000);
    process.waitForFinished(3000);
    process.waitForReadyRead(3000);
    QString output = process.readAllStandardOutput();
    output.remove('\n');
    return output;
}

QStringList GitWidget::getBranches() const
{
    QProcess process;
    process.setWorkingDirectory(repoPath);
    process.startCommand("git branch --format='%(refname:short)'");
    process.waitForStarted(3000);
    process.waitForFinished(3000);
    process.waitForReadyRead(3000);
    const QString results = process.readAllStandardOutput();
    static const QRegularExpression regex(R"((\S+))");
    QRegularExpressionMatchIterator iterator = regex.globalMatch(results);
    QStringList branches;
    while(iterator.hasNext())
    {
        auto match = iterator.next();
        for(int i=1;i<match.capturedLength(); i++)
        {
            auto tmp = match.captured(i);
            if(!tmp.isEmpty())
            {
                branches << tmp;
            }
        }
    }
    return branches;
}

void GitWidget::gitFileDiff(const QString &filePath)
{
    QProcess* process = new QProcess(this);
    process->setWorkingDirectory(repoPath);
    ProcessManager::getInstance()->registerProcess(process, tr("Git diff"));
    connect(process, &QProcess::readyReadStandardOutput, this, [this, process, filePath]{
        emit openInEditor(process->readAllStandardOutput(), tr("git diff %1").arg(filePath));
    });
    connect(process, &QProcess::finished, this, [process]{
        process->deleteLater();
    });
    process->start("git", {"diff", filePath});
    process->waitForStarted();
}

void GitWidget::gitAddFile(const QString &filePath)
{
    QProcess* process = new QProcess(this);
    process->setWorkingDirectory(repoPath);
    ProcessManager::getInstance()->registerProcess(process, tr("Git Add"));
    connect(process, &QProcess::finished, this, [this, process]{
        process->deleteLater();
        refresh();
    });
    process->start("git", {"add", filePath});
    process->waitForStarted();
}

void GitWidget::setBranch(const QString &name)
{
    QProcess* process = new QProcess(this);
    process->setWorkingDirectory(repoPath);
    ProcessManager::getInstance()->registerProcess(process, tr("Setting branch"));
    process->start("git", {"checkout", name});
    connect(process, &QProcess::finished, this, [this, process]{emit branchNameChanged(getBranchName()); process->deleteLater();});
    process->waitForStarted(3000);
}
