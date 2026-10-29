#pragma once

#include <QWebEngineUrlRequestInterceptor>

class OfflineRequestInterceptor final : public QWebEngineUrlRequestInterceptor
{
    Q_OBJECT

public:
    explicit OfflineRequestInterceptor(QObject* parent = nullptr);

    void interceptRequest(QWebEngineUrlRequestInfo& info) override;
};
