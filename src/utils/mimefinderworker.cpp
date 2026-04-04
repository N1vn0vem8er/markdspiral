#include "mimefinderworker.h"
#include <QDir>
#include <QRegularExpressionMatchIterator>

MimeFinderWorker::MimeFinderWorker(QObject *parent)
    : QObject{parent}
{}

void MimeFinderWorker::start()
{
    const QString generalPath = "/usr/share/applications";
    const QString userPath = QDir::homePath() + "/.local/share/applications";
    QList<FileSystemTree::MimeApplication> apps;
    apps.append(find(generalPath));
    const auto user = find(userPath);
    for(const auto& i : user)
    {
        if(std::find_if(apps.begin(), apps.end(), [&i](const auto& val){return i.name == val.name;}) == apps.end())

        {
            apps.append(i);
        }
    }
    emit resultsReady(apps);
    emit finished();
}

QList<FileSystemTree::MimeApplication> MimeFinderWorker::find(const QString &path) const
{
    QList<FileSystemTree::MimeApplication> ret;
    const auto dirs = QDir(path).entryList(QStringList() << "*.desktop", QDir::Files);
    for(const auto& desktopFile : dirs)
    {
        QFile file(path + "/" + desktopFile);
        file.open(QIODevice::ReadOnly);
        if(file.isOpen())
        {
            const QString text = file.readAll();
            static QRegularExpression regex(R"(MimeType=(.*))");
            QRegularExpressionMatchIterator i = regex.globalMatch(text);
            while(i.hasNext())
            {
                QRegularExpressionMatch match = i.next();
                if(match.hasMatch())
                {
                    FileSystemTree::MimeApplication app;
                    for(int i = 1; i<match.capturedLength(); i++)
                    {
                        if(!match.captured(i).isEmpty())
                        {
                            auto tmp = match.captured(i).split(";");
                            tmp.removeAll(QString(""));
                            app.mimeTypes.append(tmp);
                        }
                    }
                    if(!app.mimeTypes.isEmpty())
                    {
                        static QRegularExpression regex("\\bName=(\\w+)");
                        auto match = regex.match(text);
                        if(match.hasMatch())
                            app.name = match.captured(1);
                        if(!app.name.isEmpty())
                        {
                            static QRegularExpression regex1("\\bIcon=(\\w+)");
                            auto match = regex1.match(text);
                            if(match.hasMatch())
                                app.icon = match.captured(1);
                            static QRegularExpression regex2("\\bExec=(\\w+)");
                            auto matchExec = regex2.match(text);
                            if(matchExec.hasMatch())
                                app.exec = matchExec.captured(1);
                            ret.append(app);
                        }
                    }
                }
            }
            file.close();
        }
    }
    return ret;
}
