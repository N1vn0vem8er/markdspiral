#include "runningprocesseslabel.h"
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QStringListModel>

RunningProcessesLabel::RunningProcessesLabel(const QString &text, QWidget *parent) : QLabel(text, parent)
{
    init();
}

void RunningProcessesLabel::addProcess(QProcess *process, const QString &description)
{
    processes.insert(process, description);
    QStringList list;
    for(const auto& i : std::as_const(processes))
    {
        list.append(i);
    }
    if(listView->model())
        listView->model()->deleteLater();
    processesCount = list.size();
    if(processesCount > 0)
        setText(tr("Processes: %1").arg(processesCount));
    else
        setText("");
    QStringListModel* model = new QStringListModel(list, listView);
    listView->setModel(model);
}

void RunningProcessesLabel::removeProcess(QProcess *process)
{
    processes.remove(process);
    QStringList list;
    for(const auto& i : std::as_const(processes))
    {
        list.append(i);
    }
    if(listView->model())
        listView->model()->deleteLater();
    processesCount = list.size();
    if(processesCount > 0)
        setText(tr("Processes: %1").arg(processesCount));
    else
        setText("");
    QStringListModel* model = new QStringListModel(list, listView);
    listView->setModel(model);
}

void RunningProcessesLabel::init()
{
    setMouseTracking(true);
    toolTipWidget = new QWidget(this);
    toolTipWidget->setWindowFlag(Qt::ToolTip);
    listView = new QListView(toolTipWidget);
    QHBoxLayout* layout = new QHBoxLayout(toolTipWidget);
    layout->addWidget(listView);
    toolTipWidget->setLayout(layout);
    listView->setModel(nullptr);
}

void RunningProcessesLabel::enterEvent(QEnterEvent *event)
{
    if(processesCount > 0&& event->type() == QEvent::Enter)
    {
        toolTipWidget->move(QCursor::pos());
        toolTipWidget->show();
    }
    QLabel::enterEvent(event);
}

void RunningProcessesLabel::leaveEvent(QEvent *event)
{
    if(processesCount > 0 && event->type() == QEvent::Leave)
    {
        toolTipWidget->hide();
    }
    QLabel::leaveEvent(event);
}
