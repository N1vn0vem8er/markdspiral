#ifndef RUNNINGPROCESSESLABEL_H
#define RUNNINGPROCESSESLABEL_H

#include "qprocess.h"
#include <QListView>
#include <QLabel>

class RunningProcessesLabel : public QLabel
{
    Q_OBJECT
public:
    explicit RunningProcessesLabel(const QString& text, QWidget* parent = nullptr);

public slots:
    void addProcess(QProcess* process, const QString& description);
    void removeProcess(QProcess* process);

private:
    QWidget* toolTipWidget {nullptr};
    QListView* listView {nullptr};
    int processesCount {0};
    QMap<QProcess*, QString> processes;
    void init();
    void showRunningProcesses();

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
};

#endif // RUNNINGPROCESSESLABEL_H
