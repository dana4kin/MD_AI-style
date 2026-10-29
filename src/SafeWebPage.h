#pragma once

#include <QWebEnginePage>

class SafeWebPage final : public QWebEnginePage
{
    Q_OBJECT

public:
    explicit SafeWebPage(QObject* parent = nullptr);

protected:
    bool acceptNavigationRequest(const QUrl& url,
                                 NavigationType type,
                                 bool isMainFrame) override;
};
