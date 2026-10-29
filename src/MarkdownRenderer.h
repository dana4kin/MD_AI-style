#pragma once

#include <QString>

class MarkdownRenderer final
{
public:
    enum class Theme
    {
        System,
        Light,
        Dark
    };

    struct Result
    {
        QString html;
        QString error;

        [[nodiscard]] bool ok() const noexcept { return error.isEmpty(); }
    };

    [[nodiscard]] static Result render(const QString& markdown,
                                       const QString& documentTitle,
                                       Theme theme);

private:
    [[nodiscard]] static QString escapeHtml(const QString& value);
    [[nodiscard]] static QString themeName(Theme theme);
};
