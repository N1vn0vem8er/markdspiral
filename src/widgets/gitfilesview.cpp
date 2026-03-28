#include "gitfilesview.h"
#include <qmenu.h>

GitFilesView::GitFilesView(QWidget *parent) : QTableView(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &GitFilesView::customContextMenuRequested, this, &GitFilesView::openContextMenu);
}

void GitFilesView::setContextMenu(QMenu *contextMenu)
{
    this->contextMenu = contextMenu;
}

void GitFilesView::gitAddPressed()
{
    const auto index = indexAt(currentPoint);
    if(index.isValid())
        emit gitAdd(index);
}

void GitFilesView::gitDiffPressed()
{
    const auto index = indexAt(currentPoint);
    if(index.isValid())
        emit gitDiff(index);
}

void GitFilesView::openFilePressed()
{
    const auto index = indexAt(currentPoint);
    if(index.isValid())
        emit openFile(index);
}

void GitFilesView::openContextMenu(const QPoint& point)
{
    assert((contextMenu != nullptr) && "Context menu cannot be nullptr");
    currentPoint = point;
    contextMenu->popup(mapToGlobal(point));
}
