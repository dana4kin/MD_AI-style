#include "MainWindow.h"

#include "OfflineRequestInterceptor.h"
#include "SafeWebPage.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QSettings>
#include <QShortcut>
#include <QStatusBar>
#include <QTimer>
#include <QToolBar>
#include <QUrl>
#include <QWebEngineFindTextResult>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineView>

namespace
{
constexpr auto kRecentFilesKey = "recentFiles";
constexpr auto kThemeKey = "theme";
constexpr int kRecentFileLimit = 10;

QString initialWelcomeMarkdown()
{
    return QStringLiteral(
        "# MD_AI-style\n\n"
        "Откройте файл `.md` или перетащите его в это окно.\n\n"
        "- оформление работает полностью офлайн;\n"
        "- поддерживаются таблицы, списки задач, зачёркивание и автоматические ссылки;\n"
        "- локальные изображения загружаются относительно файла Markdown;\n"
        "- удалённое содержимое не загружается внутри документа.\n\n"
        "> Совет: нажмите **Ctrl+O**, чтобы выбрать документ.\n");
}
} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_view(new QWebEngineView(this))
    , m_watcher(new QFileSystemWatcher(this))
    , m_reloadTimer(new QTimer(this))
{
    setAcceptDrops(true);
    resize(1100, 780);
    setMinimumSize(640, 480);
    setWindowIcon(QIcon(QStringLiteral(":/icons/app.svg")));

    auto* page = new SafeWebPage(m_view);
    auto* interceptor = new OfflineRequestInterceptor(page);
    page->profile()->setUrlRequestInterceptor(interceptor);
    page->profile()->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
    page->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, false);
    page->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, false);
    page->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);
    m_view->setPage(page);
    setCentralWidget(m_view);

    m_reloadTimer->setSingleShot(true);
    m_reloadTimer->setInterval(180);
    connect(m_reloadTimer, &QTimer::timeout, this, &MainWindow::reloadFile);
    connect(m_watcher, &QFileSystemWatcher::fileChanged,
            this, &MainWindow::updateAfterFileChange);

    createActions();
    createMenusAndToolbar();
    createFindBar();

    const QString storedTheme = QSettings().value(QLatin1String(kThemeKey), QStringLiteral("system")).toString();
    if (storedTheme == QStringLiteral("light")) {
        applyTheme(MarkdownRenderer::Theme::Light);
    } else if (storedTheme == QStringLiteral("dark")) {
        applyTheme(MarkdownRenderer::Theme::Dark);
    } else {
        applyTheme(MarkdownRenderer::Theme::System);
    }

    showWelcome();
}

bool MainWindow::openFile(const QString& filePath)
{
    const QFileInfo info(filePath);
    if (!info.exists() || !info.isFile()) {
        QMessageBox::warning(this, tr("MD_AI-style"),
                             tr("The selected file does not exist."));
        return false;
    }
    if (!isMarkdownFile(info.absoluteFilePath())) {
        QMessageBox::warning(this, tr("MD_AI-style"),
                             tr("Choose a Markdown file with the .md or .markdown extension."));
        return false;
    }

    QFile file(info.absoluteFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("MD_AI-style"),
                              tr("The file cannot be read:\n%1").arg(file.errorString()));
        return false;
    }

    const QByteArray bytes = file.readAll();
    m_markdown = QString::fromUtf8(bytes);
    m_currentPath = info.absoluteFilePath();

    const QStringList watched = m_watcher->files();
    if (!watched.isEmpty()) {
        m_watcher->removePaths(watched);
    }
    m_watcher->addPath(m_currentPath);

    updateRecentFiles(m_currentPath);
    renderCurrentDocument();
    statusBar()->showMessage(tr("Opened %1").arg(info.fileName()), 2500);
    return true;
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        const QList<QUrl> urls = event->mimeData()->urls();
        if (!urls.isEmpty() && urls.first().isLocalFile()
            && isMarkdownFile(urls.first().toLocalFile())) {
            event->acceptProposedAction();
        }
    }
}

void MainWindow::dropEvent(QDropEvent* event)
{
    const QList<QUrl> urls = event->mimeData()->urls();
    if (!urls.isEmpty() && urls.first().isLocalFile()
        && openFile(urls.first().toLocalFile())) {
        event->acceptProposedAction();
    }
}

void MainWindow::chooseFile()
{
    const QString startPath = m_currentPath.isEmpty()
        ? QSettings().value(QStringLiteral("lastDirectory")).toString()
        : QFileInfo(m_currentPath).absolutePath();
    const QString path = QFileDialog::getOpenFileName(
        this,
        tr("Open Markdown"),
        startPath,
        tr("Markdown files (*.md *.markdown);;All files (*)"));
    if (path.isEmpty()) {
        return;
    }
    QSettings().setValue(QStringLiteral("lastDirectory"), QFileInfo(path).absolutePath());
    openFile(path);
}

void MainWindow::reloadFile()
{
    if (m_currentPath.isEmpty()) {
        return;
    }
    const QString path = m_currentPath;
    if (openFile(path)) {
        statusBar()->showMessage(tr("Reloaded %1").arg(QFileInfo(path).fileName()), 1800);
    }
}

void MainWindow::exportPdf()
{
    if (m_currentPath.isEmpty()) {
        QMessageBox::information(this, tr("Export PDF"), tr("Open a Markdown file first."));
        return;
    }
    const QString suggested = QFileInfo(m_currentPath).absolutePath()
        + QLatin1Char('/') + QFileInfo(m_currentPath).completeBaseName() + QStringLiteral(".pdf");
    const QString path = QFileDialog::getSaveFileName(
        this, tr("Export PDF"), suggested, tr("PDF files (*.pdf)"));
    if (path.isEmpty()) {
        return;
    }
    m_view->page()->printToPdf(path);
    statusBar()->showMessage(tr("PDF export started."), 2500);
}

void MainWindow::showFindBar()
{
    m_findBar->setVisible(true);
    m_findEdit->setFocus();
    m_findEdit->selectAll();
}

void MainWindow::findNext()
{
    m_view->findText(m_findEdit->text());
}

void MainWindow::setSystemTheme()
{
    applyTheme(MarkdownRenderer::Theme::System);
}

void MainWindow::setLightTheme()
{
    applyTheme(MarkdownRenderer::Theme::Light);
}

void MainWindow::setDarkTheme()
{
    applyTheme(MarkdownRenderer::Theme::Dark);
}

void MainWindow::updateAfterFileChange(const QString& path)
{
    if (path == m_currentPath) {
        m_reloadTimer->start();
    }
}

void MainWindow::createActions()
{
    auto* openAction = new QAction(tr("&Open…"), this);
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::chooseFile);
    addAction(openAction);

    auto* reloadAction = new QAction(tr("&Reload"), this);
    reloadAction->setShortcut(QKeySequence::Refresh);
    connect(reloadAction, &QAction::triggered, this, &MainWindow::reloadFile);
    addAction(reloadAction);

    auto* exportAction = new QAction(tr("Export &PDF…"), this);
    exportAction->setShortcut(QKeySequence::Print);
    connect(exportAction, &QAction::triggered, this, &MainWindow::exportPdf);
    addAction(exportAction);

    auto* findAction = new QAction(tr("&Find…"), this);
    findAction->setShortcut(QKeySequence::Find);
    connect(findAction, &QAction::triggered, this, &MainWindow::showFindBar);
    addAction(findAction);

    m_systemThemeAction = new QAction(tr("System"), this);
    m_lightThemeAction = new QAction(tr("Light"), this);
    m_darkThemeAction = new QAction(tr("Dark"), this);
    for (QAction* action : {m_systemThemeAction, m_lightThemeAction, m_darkThemeAction}) {
        action->setCheckable(true);
    }
    auto* themeGroup = new QActionGroup(this);
    themeGroup->setExclusive(true);
    themeGroup->addAction(m_systemThemeAction);
    themeGroup->addAction(m_lightThemeAction);
    themeGroup->addAction(m_darkThemeAction);
    connect(m_systemThemeAction, &QAction::triggered, this, &MainWindow::setSystemTheme);
    connect(m_lightThemeAction, &QAction::triggered, this, &MainWindow::setLightTheme);
    connect(m_darkThemeAction, &QAction::triggered, this, &MainWindow::setDarkTheme);
}

void MainWindow::createMenusAndToolbar()
{
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(actions().at(0));
    m_recentMenu = fileMenu->addMenu(tr("Open &Recent"));
    rebuildRecentMenu();
    fileMenu->addSeparator();
    fileMenu->addAction(actions().at(1));
    fileMenu->addAction(actions().at(2));
    fileMenu->addSeparator();
    QAction* quitAction = fileMenu->addAction(tr("E&xit"));
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    QMenu* viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(actions().at(3));
    QMenu* themeMenu = viewMenu->addMenu(tr("Theme"));
    themeMenu->addAction(m_systemThemeAction);
    themeMenu->addAction(m_lightThemeAction);
    themeMenu->addAction(m_darkThemeAction);
    viewMenu->addSeparator();

    QAction* zoomInAction = viewMenu->addAction(tr("Zoom in"));
    zoomInAction->setShortcut(QKeySequence::ZoomIn);
    connect(zoomInAction, &QAction::triggered, this, [this] {
        m_view->setZoomFactor(qMin(3.0, m_view->zoomFactor() + 0.1));
    });
    QAction* zoomOutAction = viewMenu->addAction(tr("Zoom out"));
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    connect(zoomOutAction, &QAction::triggered, this, [this] {
        m_view->setZoomFactor(qMax(0.5, m_view->zoomFactor() - 0.1));
    });
    QAction* actualSizeAction = viewMenu->addAction(tr("Actual size"));
    actualSizeAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+0")));
    connect(actualSizeAction, &QAction::triggered, this, [this] { m_view->setZoomFactor(1.0); });

    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction* aboutAction = helpMenu->addAction(tr("About MD_AI-style"));
    connect(aboutAction, &QAction::triggered, this, [this] {
        QMessageBox::about(this, tr("About MD_AI-style"),
            tr("<b>MD_AI-style %1</b><br><br>Offline Markdown viewer with a clean AI-chat style."
               "<br>GitHub Flavored Markdown is rendered locally; remote document resources are blocked.")
                .arg(QStringLiteral(MD_AI_STYLE_VERSION)));
    });

    auto* toolbar = addToolBar(tr("Main"));
    toolbar->setMovable(false);
    toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolbar->addAction(actions().at(0));
    toolbar->addAction(actions().at(1));
    toolbar->addAction(actions().at(2));

    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::createFindBar()
{
    m_findBar = new QWidget(this);
    auto* layout = new QHBoxLayout(m_findBar);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->addWidget(new QLabel(tr("Find:"), m_findBar));
    m_findEdit = new QLineEdit(m_findBar);
    layout->addWidget(m_findEdit, 1);
    QAction* closeAction = m_findEdit->addAction(QIcon::fromTheme(QStringLiteral("window-close")),
                                                 QLineEdit::TrailingPosition);
    connect(closeAction, &QAction::triggered, m_findBar, &QWidget::hide);
    connect(m_findEdit, &QLineEdit::returnPressed, this, &MainWindow::findNext);
    m_findBar->setVisible(false);

    auto* toolbar = new QToolBar(tr("Find"), this);
    toolbar->setMovable(false);
    toolbar->addWidget(m_findBar);
    addToolBar(Qt::BottomToolBarArea, toolbar);
}

void MainWindow::showWelcome()
{
    m_markdown = initialWelcomeMarkdown();
    m_currentPath.clear();
    renderCurrentDocument();
}

void MainWindow::renderCurrentDocument()
{
    const QString title = m_currentPath.isEmpty()
        ? QStringLiteral("MD_AI-style")
        : QFileInfo(m_currentPath).fileName();
    const MarkdownRenderer::Result result = MarkdownRenderer::render(m_markdown, title, m_theme);
    if (!result.ok()) {
        QMessageBox::critical(this, tr("Rendering failed"), result.error);
        return;
    }

    QUrl baseUrl(QStringLiteral("qrc:/web/"));
    if (!m_currentPath.isEmpty()) {
        QString directory = QFileInfo(m_currentPath).absolutePath();
        if (!directory.endsWith(QLatin1Char('/'))) {
            directory += QLatin1Char('/');
        }
        baseUrl = QUrl::fromLocalFile(directory);
    }
    m_view->setHtml(result.html, baseUrl);
    setWindowTitle(m_currentPath.isEmpty()
        ? QStringLiteral("MD_AI-style")
        : QStringLiteral("%1 — MD_AI-style").arg(title));
}

void MainWindow::updateRecentFiles(const QString& path)
{
    QSettings settings;
    QStringList files = settings.value(QLatin1String(kRecentFilesKey)).toStringList();
    files.removeAll(path);
    files.prepend(path);
    while (files.size() > kRecentFileLimit) {
        files.removeLast();
    }
    settings.setValue(QLatin1String(kRecentFilesKey), files);
    rebuildRecentMenu();
}

void MainWindow::rebuildRecentMenu()
{
    if (m_recentMenu == nullptr) {
        return;
    }
    m_recentMenu->clear();
    QStringList files = QSettings().value(QLatin1String(kRecentFilesKey)).toStringList();
    files.removeIf([](const QString& path) { return !QFileInfo::exists(path); });
    QSettings().setValue(QLatin1String(kRecentFilesKey), files);

    if (files.isEmpty()) {
        QAction* emptyAction = m_recentMenu->addAction(tr("No recent files"));
        emptyAction->setEnabled(false);
        return;
    }

    for (const QString& path : files) {
        QAction* action = m_recentMenu->addAction(displayNameFor(path));
        action->setToolTip(path);
        connect(action, &QAction::triggered, this, [this, path] { openFile(path); });
    }
}

void MainWindow::applyTheme(MarkdownRenderer::Theme theme)
{
    m_theme = theme;
    m_systemThemeAction->setChecked(theme == MarkdownRenderer::Theme::System);
    m_lightThemeAction->setChecked(theme == MarkdownRenderer::Theme::Light);
    m_darkThemeAction->setChecked(theme == MarkdownRenderer::Theme::Dark);

    QString setting = QStringLiteral("system");
    if (theme == MarkdownRenderer::Theme::Light) {
        setting = QStringLiteral("light");
    } else if (theme == MarkdownRenderer::Theme::Dark) {
        setting = QStringLiteral("dark");
    }
    QSettings().setValue(QLatin1String(kThemeKey), setting);
    renderCurrentDocument();
}

bool MainWindow::isMarkdownFile(const QString& path)
{
    const QString suffix = QFileInfo(path).suffix().toLower();
    return suffix == QStringLiteral("md") || suffix == QStringLiteral("markdown");
}

QString MainWindow::displayNameFor(const QString& path)
{
    const QFileInfo info(path);
    return QStringLiteral("%1 — %2").arg(info.fileName(), info.absolutePath());
}
