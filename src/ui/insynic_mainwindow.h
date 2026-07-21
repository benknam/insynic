#ifndef INSYNIC_MAIN_WINDOW_H
#define INSYNIC_MAIN_WINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QHBoxLayout>
#include <QCloseEvent>
#include <QTranslator>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QTimer>
#include <SDL3/SDL.h>

#include "insynic_controlpanel.h"
#include "insynic_customtranslator.h"
#include "insynic_filemanager.h"
#include "insynic_settingsdialog.h"
#include "insynic_networkdialog.h"

class InsynicFileBrowserDialog;
class InsynicDeviceWindow;

class InsynicMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit InsynicMainWindow(QWidget *parent = nullptr);
    ~InsynicMainWindow();
    void recreateMenuBar();

    bool isTouchSyncEnabled() const { return m_touchSyncEnabled; }

public slots:
    void setTouchSyncEnabled(bool enabled);

signals:
    void touchSyncToggled(bool enabled);

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onConnectClicked(const QString &serial);
    void onDisconnectClicked(const QString &serial);
    void onDisconnectAllClicked();
    void onConnectAllClicked();
    void onFileManagerClicked();
    void onRefreshClicked();
    void onAboutClicked();
    void onLanguageChanged(QAction *action);
    void onNetworkConnectOptionSelected();
    void onDisconnectNetworkDeviceRequested(const QString &serial);
    void onDeviceSettingsRequested(const QString &serial);
    void onRecordSettingsRequested(const QString &serial);
    void onDeviceWindowClosed(const QString &serial);
    void processGlobalSdlEvents();
    void disconnectNextDevice();

private:
    void setupUi();
    void createMenuBar();
    void createTrayIcon();
    QString findAdbPath();
    QString findServerPath();
    void refreshDeviceList();

    QWidget *m_centralWidget;
    QHBoxLayout *m_mainLayout;

    InsynicControlPanel *m_controlPanel;

    InsynicFileManager *m_fileManager;
    InsynicFileBrowserDialog *m_fileBrowser;

    QString m_adbPath;
    QString m_serverPath;

    InsynicCustomTranslator m_translator;

    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;
    QString m_currentLanguage;

    int m_maxSize;
    int m_maxFps;
    int m_videoBitRate;

    QTimer *m_sdlEventTimer;
    QList<InsynicDeviceWindow*> m_deviceWindows;
    bool m_touchSyncEnabled;
    bool m_isDisconnectingAll;
};

#endif
