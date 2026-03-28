#include "gitfilestatusmodel.h"

GitFileStatusModel::GitFileStatusModel(QObject *parent)
    : QAbstractListModel{parent}
{}

int GitFileStatusModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : items.count();
}

int GitFileStatusModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 4;
}

QVariant GitFileStatusModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid() || index.row() >= items.count())
    {
        return QVariant();
    }
    if(role == Qt::DisplayRole)
        switch(index.column())
        {
        case 0:
            return items.at(index.row()).name;
            break;
        case 1:
            return items.at(index.row()).addedLines;
            break;
        case 2:
            return items.at(index.row()).removedLines;
            break;
        case 3:
            return items.at(index.row()).status;
            break;
        }
    return QVariant();
}

void GitFileStatusModel::setItems(const QList<GitFileStatus> items)
{
    this->items = items;
}

QList<GitFileStatus> GitFileStatusModel::getItems() const
{
    return items;
}
