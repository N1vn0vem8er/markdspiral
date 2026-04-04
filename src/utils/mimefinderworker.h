#ifndef MIMEFINDERWORKER_H
#define MIMEFINDERWORKER_H

#include <QObject>
#include <QList>
#include "widgets/filesystemtree.h"

class MimeFinderWorker : public QObject
{
    Q_OBJECT
public:
    explicit MimeFinderWorker(QObject *parent = nullptr);

public slots:
    void start();

private:
    QList<FileSystemTree::MimeApplication> find(const QString& path) const;

signals:
    void resultsReady(QList<FileSystemTree::MimeApplication> ret);
    void finished();
};

#endif // MIMEFINDERWORKER_H
