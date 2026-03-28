#include "gitfilestatusitemdelegate.h"
#include <qapplication.h>
#include <QPalette>

GitFileStatusItemDelegate::GitFileStatusItemDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

void GitFileStatusItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    QStyleOptionButton button;
    switch(index.column())
    {
    case 0:
    case 3:
        QStyledItemDelegate::paint(painter, opt, index);
        break;
    case 1:
    {
        auto palette = QPalette(opt.palette);
        palette.setColor(QPalette::Text, QColor::fromRgb(255, 0, 0));
        QApplication::style()->drawItemText(painter, opt.rect, Qt::AlignLeft, palette, opt.state & QStyle::State_Enabled, "+ " + opt.text);
    }
        break;
    case 2:
        auto palette = QPalette(opt.palette);
        palette.setColor(QPalette::Text, QColor::fromRgb(0, 255, 0));
        QApplication::style()->drawItemText(painter, opt.rect, Qt::AlignLeft, palette, opt.state & QStyle::State_Enabled, "- " + opt.text);
        break;
    }
}

QSize GitFileStatusItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

bool GitFileStatusItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_UNUSED(event);
    Q_UNUSED(model);
    Q_UNUSED(option);
    Q_UNUSED(index);
    return true;
}
