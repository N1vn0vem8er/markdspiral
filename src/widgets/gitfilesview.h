#ifndef GITFILESVIEW_H
#define GITFILESVIEW_H

#include "GitFileStatus.h"
#include <QTableView>

class GitFilesView : public QTableView
{
    Q_OBJECT
public:
    explicit GitFilesView(QWidget *parent = nullptr);
    void setContextMenu(QMenu* contextMenu);

private:
    QMenu* contextMenu {nullptr};
    QPoint currentPoint;

public slots:
    void gitAddPressed();
    void gitDiffPressed();
    void openFilePressed();

private slots:
    void openContextMenu(const QPoint &point);

signals:
    void gitAdd(const QModelIndex& index);
    void gitDiff(const QModelIndex& index);
    void openFile(const QModelIndex& index);

};

#endif // GITFILESVIEW_H
