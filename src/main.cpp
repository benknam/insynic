#include <QApplication>
#include <QStyleFactory>
#include <QPalette>
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QTimer>

#include "insynic_mainwindow.h"
#include "insynic_scrcpy.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setAttribute(Qt::AA_DontUseNativeMenuBar, false);

    QString appDir = app.applicationDirPath();
    QString adbPath = QDir(appDir).absoluteFilePath("../Resources/adb");
    if (!QFile::exists(adbPath)) {
        adbPath = "adb";
    }

    system(QString(adbPath + " forward --remove-all").toStdString().c_str());
    system(QString(adbPath + " shell pkill -f scrcpy-server").toStdString().c_str());

    app.setApplicationName("insynic");
    app.setApplicationDisplayName("insynic");
    app.setOrganizationName("insynic");
    app.setOrganizationDomain("insynic.com");

    QPalette palette;
    palette.setColor(QPalette::Window, QColor(42, 42, 42));
    palette.setColor(QPalette::WindowText, QColor(230, 230, 230));
    palette.setColor(QPalette::Base, QColor(30, 30, 30));
    palette.setColor(QPalette::AlternateBase, QColor(42, 42, 42));
    palette.setColor(QPalette::ToolTipBase, QColor(255, 255, 255));
    palette.setColor(QPalette::ToolTipText, QColor(0, 0, 0));
    palette.setColor(QPalette::Text, QColor(230, 230, 230));
    palette.setColor(QPalette::Button, QColor(53, 53, 53));
    palette.setColor(QPalette::ButtonText, QColor(230, 230, 230));
    palette.setColor(QPalette::BrightText, QColor(255, 0, 0));
    palette.setColor(QPalette::Link, QColor(42, 130, 218));
    palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    palette.setColor(QPalette::HighlightedText, QColor(0, 0, 0));
    app.setPalette(palette);

    InsynicMainWindow w;
    w.show();

    if (!insynic_scrcpy_init_sdl()) {
        fprintf(stderr, "Failed to initialize SDL\n");
        return 1;
    }

    QTimer::singleShot(500, [&w]() {
        w.recreateMenuBar();
    });

    return app.exec();
}
