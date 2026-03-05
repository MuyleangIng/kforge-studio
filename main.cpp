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

    // Enable HiDPI
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);
    app.setFont(QFont("Consolas", 12));

    MainWindow w;
    w.show();

    return app.exec();
}
