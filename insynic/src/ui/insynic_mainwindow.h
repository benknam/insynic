#ifndef INSYNIC_MAIN_WINDOW_H
#define INSYNIC_MAIN_WINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QHBoxLayout>
#include <QTimer>
#include <QCloseEvent>
#include <QProcess>
#include <QCoreApplication>
#include <QDir>
#include <QTranslator>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QMap>

#include "insynic_controlpanel.h"
#include "insynic_customtranslator.h"
#include "insynic_filemanager.h"
#include "insynic_scrcpy.h"
#include "insynic_settingsdialog.h"
#include "insynic_networkdialog.h"
#include "insynic_virtualkey.h"
#include "insynic_profilemanager.h"
#include "insynic_draggablekey.h"
#include "insynic_keyconfigdialog.h"

class InsynicFileBrowserDialog;

class InsynicMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit InsynicMainWindow(QWidget *parent = nullptr);
    ~InsynicMainWindow();
    void recreateMenuBar();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onConnectClicked();
    void onDisconnectClicked();
    void onFileManagerClicked();
    void onDeviceSelected(const QString &serial);
    void checkState();
    void onOtgInputClicked();
    void onAboutClicked();
    void onLanguageChanged(QAction *action);
    void onSettingsClicked();
    void onSettingsChanged(int maxSize, int maxFps, int videoBitRate);
    void onNetworkConnectOptionSelected();
    void onAddKeyClicked();
    void onKeyConfigRequested(InsynicDraggableKey *key);
    void onKeyDeleteRequested(InsynicDraggableKey *key);
    void onKeyPositionChanged(int x, int y);
    void onProfileSelected(const QString &name);
    void onSaveProfileClicked();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void setupUi();
    void startScrcpy();
    void stopScrcpy();
    static void stateCallback(enum insynic_scrcpy_state state, void *userdata);
    void handleStateChange(enum insynic_scrcpy_state state);
    void createMenuBar();
    void createTrayIcon();
    void addVirtualKey(const VirtualKey &key);
    void removeAllVirtualKeys();
    void updateKeyContainerPosition();

    QWidget *m_centralWidget;
    QHBoxLayout *m_mainLayout;

    InsynicControlPanel *m_controlPanel;

    struct insynic_scrcpy *m_scrcpy;
    InsynicFileManager *m_fileManager;
    InsynicFileBrowserDialog *m_fileBrowser;

    QString m_selectedSerial;
    QString m_adbPath;
    QString m_serverPath;

    QTimer *m_stateTimer;
    volatile bool m_stateChangePending;
    volatile int m_pendingState;
    bool m_otgMode;
    QProcess *m_otgProcess;
    InsynicCustomTranslator m_translator;
    
    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;
    QString m_currentLanguage;

    int m_maxSize;
    int m_maxFps;
    int m_videoBitRate;
    
    QList<InsynicDraggableKey*> m_virtualKeys;
    InsynicProfileManager *m_profileManager;
    QWidget *m_keyContainer;
    bool m_isDraggingKey;
    QMap<int, bool> m_toggleStates;
};

#endif
