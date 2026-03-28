#ifndef GITFILESTATUSMODEL_H
#define GITFILESTATUSMODEL_H

#include "GitFileStatus.h"
#include <QAbstractListModel>

class GitFileStatusModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit GitFileStatusModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    void setItems(const QList<GitFileStatus> items);

    QList<GitFileStatus> getItems() const;

private:
    QList<GitFileStatus> items;
};

#endif // GITFILESTATUSMODEL_H
