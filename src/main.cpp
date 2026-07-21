#include <QApplication>
#include <QStyleFactory>
#include <QPalette>
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QTimer>
#include <QStandardPaths>
#include <QDateTime>
#include <QMessageLogContext>

#include "insynic_mainwindow.h"
#include "insynic_scrcpy.h"

// Global log file for debug output
static QFile *g_logFile = nullptr;

static void
messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (g_logFile && g_logFile->isOpen()) {
        QString typeName;
        switch (type) {
        case QtDebugMsg:    typeName = "DEBUG";   break;
        case QtInfoMsg:     typeName = "INFO";    break;
        case QtWarningMsg:  typeName = "WARN";    break;
        case QtCriticalMsg: typeName = "CRIT";     break;
        case QtFatalMsg:    typeName = "FATAL";    break;
        }
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
        QString line = QString("[%1] [%2] %3\n").arg(timestamp, typeName, msg);
        g_logFile->write(line.toUtf8());
        g_logFile->flush();
    }
    fprintf(stderr, "%s\n", msg.toUtf8().constData());
}

int main(int argc, char *argv[])
{
    // Setup log file
    QString logDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/insynic_log";
    QDir().mkpath(logDir);
    QString logPath = logDir + "/insynic_debug_" +
                      QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".log";
    g_logFile = new QFile(logPath);
    if (g_logFile->open(QIODevice::WriteOnly | QIODevice::Append)) {
        qInstallMessageHandler(messageHandler);
        // Redirect stderr to log file to capture SDL/printf logs
        freopen(logPath.toUtf8().constData(), "a", stderr);
        fprintf(stderr, "=== insynic debug log started at %s ===\n",
                QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toUtf8().constData());
        fprintf(stderr, "Log file: %s\n", logPath.toUtf8().constData());
    } else {
        fprintf(stderr, "Failed to create log file: %s\n", logPath.toUtf8().constData());
        delete g_logFile;
        g_logFile = nullptr;
    }

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

    int ret = app.exec();

    if (g_logFile) {
        g_logFile->close();
        delete g_logFile;
        g_logFile = nullptr;
    }

    return ret;
}
