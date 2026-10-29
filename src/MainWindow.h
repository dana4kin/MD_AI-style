#pragma once

#include "MarkdownRenderer.h"

#include <QMainWindow>

class QAction;
class QFileSystemWatcher;
class QLineEdit;
class QMenu;
class QTimer;
class QWebEngineView;

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

    bool openFile(const QString& filePath);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    void chooseFile();
    void reloadFile();
    void exportPdf();
    void showFindBar();
    void findNext();
    void setSystemTheme();
    void setLightTheme();
    void setDarkTheme();
    void updateAfterFileChange(const QString& path);

private:
    void createActions();
    void createMenusAndToolbar();
    void createFindBar();
    void showWelcome();
    void renderCurrentDocument();
    void updateRecentFiles(const QString& path);
    void rebuildRecentMenu();
    void applyTheme(MarkdownRenderer::Theme theme);
    [[nodiscard]] static bool isMarkdownFile(const QString& path);
    [[nodiscard]] static QString displayNameFor(const QString& path);

    QWebEngineView* m_view = nullptr;
    QFileSystemWatcher* m_watcher = nullptr;
    QTimer* m_reloadTimer = nullptr;
    QWidget* m_findBar = nullptr;
    QLineEdit* m_findEdit = nullptr;
    QMenu* m_recentMenu = nullptr;
    QAction* m_systemThemeAction = nullptr;
    QAction* m_lightThemeAction = nullptr;
    QAction* m_darkThemeAction = nullptr;
    QString m_currentPath;
    QString m_markdown;
    MarkdownRenderer::Theme m_theme = MarkdownRenderer::Theme::System;
};
