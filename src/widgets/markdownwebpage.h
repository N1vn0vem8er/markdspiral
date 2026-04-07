#ifndef MARKDOWNWEBPAGE_H
#define MARKDOWNWEBPAGE_H

#include <QWebEnginePage>

class MarkdownWebPage : public QWebEnginePage
{
    Q_OBJECT
public:
    explicit MarkdownWebPage(QObject *parent = nullptr);

signals:
    void localLinkClicked(const QUrl& url);

protected:
    bool acceptNavigationRequest(const QUrl& url, NavigationType type, bool isMainFrame) override;
};

#endif // MARKDOWNWEBPAGE_H
