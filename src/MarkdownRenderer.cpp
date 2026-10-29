#include "MarkdownRenderer.h"

#include <QFile>
#include <QTextStream>

#include <md4c-html.h>
#include <md4c.h>

namespace
{
void appendHtml(const MD_CHAR* chunk, MD_SIZE size, void* userData)
{
    auto* output = static_cast<QByteArray*>(userData);
    output->append(chunk, static_cast<qsizetype>(size));
}

QString loadResource(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromUtf8(file.readAll());
}
} // namespace

MarkdownRenderer::Result MarkdownRenderer::render(const QString& markdown,
                                                   const QString& documentTitle,
                                                   Theme theme)
{
    const QByteArray input = markdown.toUtf8();
    QByteArray body;

    const unsigned parserFlags = MD_DIALECT_GITHUB
        | MD_FLAG_NOHTML
        | MD_FLAG_COLLAPSEWHITESPACE;
    const unsigned rendererFlags = MD_HTML_FLAG_SKIP_UTF8_BOM;

    const int status = md_html(input.constData(),
                               static_cast<MD_SIZE>(input.size()),
                               appendHtml,
                               &body,
                               parserFlags,
                               rendererFlags);
    if (status != 0) {
        return {{}, QStringLiteral("Markdown parser returned error %1.").arg(status)};
    }

    const QString style = loadResource(QStringLiteral(":/web/style.css"));
    if (style.isEmpty()) {
        return {{}, QStringLiteral("The embedded application style is unavailable.")};
    }

    QString html;
    QTextStream stream(&html);
    stream << QStringLiteral("<!doctype html><html lang=\"en\"><head>")
           << QStringLiteral("<meta charset=\"utf-8\">")
           << QStringLiteral("<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">")
           << QStringLiteral("<meta name=\"color-scheme\" content=\"light dark\">")
           << QStringLiteral("<title>") << escapeHtml(documentTitle) << QStringLiteral("</title>")
           << QStringLiteral("<style>") << style << QStringLiteral("</style>")
           << QStringLiteral("</head><body data-theme=\"") << themeName(theme)
           << QStringLiteral("\"><main class=\"markdown-body\">")
           << QString::fromUtf8(body)
           << QStringLiteral("</main></body></html>");

    return {html, {}};
}

QString MarkdownRenderer::escapeHtml(const QString& value)
{
    QString escaped = value;
    escaped.replace(QLatin1Char('&'), QStringLiteral("&amp;"));
    escaped.replace(QLatin1Char('<'), QStringLiteral("&lt;"));
    escaped.replace(QLatin1Char('>'), QStringLiteral("&gt;"));
    escaped.replace(QLatin1Char('"'), QStringLiteral("&quot;"));
    escaped.replace(QLatin1Char('\''), QStringLiteral("&#39;"));
    return escaped;
}

QString MarkdownRenderer::themeName(Theme theme)
{
    switch (theme) {
    case Theme::Light:
        return QStringLiteral("light");
    case Theme::Dark:
        return QStringLiteral("dark");
    case Theme::System:
        return QStringLiteral("system");
    }
    return QStringLiteral("system");
}
