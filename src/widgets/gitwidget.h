#ifndef GITWIDGET_H
#define GITWIDGET_H

#include "GitFileStatus.h"
#include "gitfilestatusmodel.h"
#include <QWidget>

namespace Ui {
class GitWidget;
}

class GitWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GitWidget(QWidget *parent = nullptr);
    ~GitWidget();
    void setRepositoryPath(const QString& path);
    void noRepo();
    bool hasRepository() const;
    void gitPush();
    void gitPull();
    void gitStatus();
    void gitLog();
    void gitFetch();
    QString getRepoPath() const;
    QString getBranchName() const;
    QStringList getBranches() const;
    void gitFileDiff(const QString& filePath);
    void gitAddFile(const QString& filePath);
    void openGitCommit();

public slots:
    void setBranch(const QString& name);

private:
    Ui::GitWidget *ui;
    QString repoPath;
    QList<GitFileStatus> untrackedFiles;
    QList<GitFileStatus> modifiedInWorkingDirectory;
    QList<GitFileStatus> modifiedInIndex;
    QList<GitFileStatus> addedInIndex;
    QList<GitFileStatus> addedInWorkingDirectory;
    QList<GitFileStatus> deletedFromWorkingDirectory;
    QList<GitFileStatus> deletedFromIndex;
    GitFileStatusModel* addedModel {nullptr};
    GitFileStatusModel* changedModel {nullptr};
    GitFileStatusModel* untrackedModel {nullptr};
    void setVisibility(bool val);
    void readStatus();
    QList<GitFileStatus> getFilesStatus(const QRegularExpression& regex, const QString& results, const QString& status) const;
    QList<QPair<QString, QPair<QString, QString>>> readDiff() const;
    void applyDiff(QList<GitFileStatus> &files, const QList<QPair<QString, QPair<QString, QString> > > &diffs) const;

private slots:
    void refresh();
    void gitAdd(const QModelIndex& index);
    void gitAddUntracked(const QModelIndex& index);
    void gitDiff(const QModelIndex& index);
    void gitDiffAdded(const QModelIndex& index);
    void openAdded(const QModelIndex& index);
    void openChanged(const QModelIndex& index);
    void openUntracked(const QModelIndex& index);
    void gitCommit(const QString& title, const QString& description);

signals:
    void openInEditor(const QString& text, const QString& title, bool readOnly = true, bool spellChecking = false, bool disableSaveWarning = true);
    void openFile(const QString& absolutePath);
    void addTab(QWidget* widget, const QString& title);
    void branchNameChanged(const QString& name);
    void sendMessage(const QString& message);
};

#endif // GITWIDGET_H
