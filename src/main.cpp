#include "MainWindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

int main(int argc, char* argv[])
{
    QApplication application(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("MD_AI-style"));
    QCoreApplication::setApplicationVersion(QStringLiteral(MD_AI_STYLE_VERSION));
    QCoreApplication::setOrganizationName(QStringLiteral("dana4kin"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("github.com/dana4kin"));

    MainWindow window;
    window.show();

    if (argc > 1) {
        const QString path = QFileInfo(QString::fromLocal8Bit(argv[1])).absoluteFilePath();
        window.openFile(QDir::cleanPath(path));
    }

    return application.exec();
}
