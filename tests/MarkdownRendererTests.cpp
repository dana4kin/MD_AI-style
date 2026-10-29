#include "../src/MarkdownRenderer.h"

#include <QtTest>

class MarkdownRendererTests final : public QObject
{
    Q_OBJECT

private slots:
    void rendersGitHubFlavoredMarkdown();
    void blocksRawHtml();
    void emitsSelectedTheme();
};

void MarkdownRendererTests::rendersGitHubFlavoredMarkdown()
{
    const QString markdown = QStringLiteral(
        "# Title\n\n"
        "| A | B |\n|---|---|\n| 1 | 2 |\n\n"
        "- [x] done\n\n~~old~~");
    const auto result = MarkdownRenderer::render(markdown, QStringLiteral("Test"),
                                                  MarkdownRenderer::Theme::System);
    QVERIFY2(result.ok(), qPrintable(result.error));
    QVERIFY(result.html.contains(QStringLiteral("<table>")));
    QVERIFY(result.html.contains(QStringLiteral("type=\"checkbox\"")));
    QVERIFY(result.html.contains(QStringLiteral("<del>old</del>")));
}

void MarkdownRendererTests::blocksRawHtml()
{
    const auto result = MarkdownRenderer::render(
        QStringLiteral("<script>alert('x')</script>\n\n<strong>unsafe</strong>"),
        QStringLiteral("Safety"),
        MarkdownRenderer::Theme::System);
    QVERIFY2(result.ok(), qPrintable(result.error));
    QVERIFY(!result.html.contains(QStringLiteral("<script>")));
    QVERIFY(!result.html.contains(QStringLiteral("<strong>unsafe</strong>")));
    QVERIFY(result.html.contains(QStringLiteral("&lt;script&gt;")));
}

void MarkdownRendererTests::emitsSelectedTheme()
{
    const auto result = MarkdownRenderer::render(
        QStringLiteral("text"), QStringLiteral("Theme"), MarkdownRenderer::Theme::Dark);
    QVERIFY2(result.ok(), qPrintable(result.error));
    QVERIFY(result.html.contains(QStringLiteral("data-theme=\"dark\"")));
}

QTEST_APPLESS_MAIN(MarkdownRendererTests)
#include "MarkdownRendererTests.moc"
