#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    QMainWindow window;
    window.setWindowTitle("insynic");
    
    QMenuBar *menuBar = window.menuBar();
    menuBar->setNativeMenuBar(true);
    
    QMenu *appMenu = menuBar->addMenu("insynic");
    appMenu->addAction("About insynic");
    appMenu->addSeparator();
    
    QMenu *settingMenu = appMenu->addMenu("Setting");
    QMenu *languageMenu = settingMenu->addMenu("Language");
    languageMenu->addAction("English");
    languageMenu->addAction("Chinese");
    
    appMenu->addSeparator();
    
    QAction *quitAction = new QAction("Quit");
    quitAction->setShortcut(QKeySequence::Quit);
    QObject::connect(quitAction, &QAction::triggered, &app, &QApplication::quit);
    appMenu->addAction(quitAction);
    
    window.show();
    
    return app.exec();
}