#include "markdownwebpage.h"
#include <QDesktopServices>

MarkdownWebPage::MarkdownWebPage(QObject *parent) : QWebEnginePage(parent) {}

bool MarkdownWebPage::acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame)
{
    if(type == NavigationTypeLinkClicked)
    {
        if(url.scheme() == "http" || url.scheme() == "https")
        {
            QDesktopServices::openUrl(url);
            return false;
        }
        else if(url.isLocalFile())
        {
            emit localLinkClicked(url);
            return false;
        }
    }
    return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
}
