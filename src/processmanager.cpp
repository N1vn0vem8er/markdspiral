#include "processmanager.h"
#include "qdebug.h"

ProcessManager::ProcessManager(QObject *parent)
    : QObject{parent}
{}

void ProcessManager::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
    QProcess* process = qobject_cast<QProcess*>(sender());
    if(process)
    {
        processes.remove(process);
        emit processRemoved(process);
        emit processesChanged();
    }
}

ProcessManager *ProcessManager::getInstance()
{
    static ProcessManager inst;
    return &inst;
}

void ProcessManager::registerProcess(QProcess *process, const QString &description)
{
    std::lock_guard<std::mutex> lock(mutex);
    if(!process || processes.contains(process))
        return;
    processes.insert(process, description);
    connect(process, &QProcess::finished, this, &ProcessManager::processFinished);
    emit processAdded(process, description);
    emit processesChanged();
}

const QMap<QProcess *, QString> &ProcessManager::getRunningPRocesses() const
{
    return processes;
}

std::mutex ProcessManager::mutex;
