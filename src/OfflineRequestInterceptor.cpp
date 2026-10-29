#include "OfflineRequestInterceptor.h"

#include <QWebEngineUrlRequestInfo>

OfflineRequestInterceptor::OfflineRequestInterceptor(QObject* parent)
    : QWebEngineUrlRequestInterceptor(parent)
{
}

void OfflineRequestInterceptor::interceptRequest(QWebEngineUrlRequestInfo& info)
{
    const QString scheme = info.requestUrl().scheme().toLower();
    const bool allowed = scheme.isEmpty()
        || scheme == QStringLiteral("file")
        || scheme == QStringLiteral("data")
        || scheme == QStringLiteral("qrc")
        || scheme == QStringLiteral("about")
        || scheme == QStringLiteral("blob");

    if (!allowed) {
        info.block(true);
    }
}
