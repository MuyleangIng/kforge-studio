#include <QApplication>
#include <QFont>
#include <QIcon>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication::setApplicationName("KForge GUI");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("KhmerStack");
    QApplication::setOrganizationDomain("khmerstack.dev");

    // HiDPI is automatic in Qt6; these attributes only needed for Qt5
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication app(argc, argv);
    app.setFont(QFont("Consolas", 12));

    MainWindow w;
    w.show();

    return app.exec();
}
