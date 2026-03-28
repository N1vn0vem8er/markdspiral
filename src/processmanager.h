#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include "qprocess.h"
#include <QObject>
#include <QMap>
#include <mutex>

class ProcessManager : public QObject
{
    Q_OBJECT
private:
    explicit ProcessManager(QObject *parent = nullptr);
    ProcessManager(const ProcessManager&) = delete;
    ProcessManager& operator=(const ProcessManager&) = delete;
    QMap<QProcess*, QString> processes;
    static std::mutex mutex;

private slots:
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);

public:
    static ProcessManager* getInstance();
    void registerProcess(QProcess* process, const QString& description);
    const QMap<QProcess*, QString>& getRunningPRocesses() const;

signals:
    void processesChanged();
    void processAdded(QProcess* process, const QString& description);
    void processRemoved(QProcess* process);
};

#endif // PROCESSMANAGER_H
