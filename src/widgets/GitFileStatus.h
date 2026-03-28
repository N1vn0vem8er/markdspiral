#ifndef GITFILESTATUS_H
#define GITFILESTATUS_H

#include <QString>


struct GitFileStatus{
    GitFileStatus() {};
    GitFileStatus(QString name, QString path, QString status, QString addedLines, QString removedLines) : name(name), path(path), status(status), addedLines(addedLines), removedLines(removedLines) {};
    QString name;
    QString path;
    QString status;
    QString addedLines;
    QString removedLines;
};

#endif // GITFILESTATUS_H
