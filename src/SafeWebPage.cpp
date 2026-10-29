#include "SafeWebPage.h"

#include <QDesktopServices>
#include <QUrl>

SafeWebPage::SafeWebPage(QObject* parent)
    : QWebEnginePage(parent)
{
}

bool SafeWebPage::acceptNavigationRequest(const QUrl& url,
                                          NavigationType type,
                                          bool isMainFrame)
{
    Q_UNUSED(isMainFrame)

    if (type == QWebEnginePage::NavigationTypeLinkClicked) {
        if (url.scheme() == QStringLiteral("file")) {
            QDesktopServices::openUrl(url);
            return false;
        }
        if (url.scheme() == QStringLiteral("http") || url.scheme() == QStringLiteral("https")
            || url.scheme() == QStringLiteral("mailto")) {
            QDesktopServices::openUrl(url);
            return false;
        }
    }

    return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
}
