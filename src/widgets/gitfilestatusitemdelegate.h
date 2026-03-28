#ifndef GITFILESTATUSITEMDELEGATE_H
#define GITFILESTATUSITEMDELEGATE_H

#include <QStyledItemDelegate>

class GitFileStatusItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    GitFileStatusItemDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
};

#endif // GITFILESTATUSITEMDELEGATE_H
