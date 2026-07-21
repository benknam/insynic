#include "insynic_mainwindow.h"
#include "insynic_filebrowser.h"
#include "insynic_settingsdialog.h"
#include "insynic_recorddialog.h"
#include "insynic_customtranslator.h"
#include "insynic_devicewindow.h"
#include "insynic_devicesettingsprofile.h"

#include <QMainWindow>
#include <QApplication>
#include <QGuiApplication>
#include <QMessageBox>
#include <QMenuBar>
#include <QActionGroup>
#include <QDebug>
#include <QStyle>
#include <QDir>
#include <QLocale>
#include <QCoreApplication>
#include <QFile>
#include <QSettings>

InsynicMainWindow::InsynicMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_fileBrowser(nullptr)
    , m_maxSize(0)
    , m_maxFps(60)
    , m_videoBitRate(8000000)
    , m_touchSyncEnabled(false)
    , m_isDisconnectingAll(false)
{
    QCoreApplication::setOrganizationName("insynic");
    QCoreApplication::setApplicationName("insynic");

    m_adbPath = findAdbPath();
    m_serverPath = findServerPath();

    QSettings settings;
    m_maxSize = settings.value("maxSize", 0).toInt();
    m_maxFps = settings.value("maxFps", 60).toInt();
    m_videoBitRate = settings.value("videoBitRate", 8000000).toInt();

    QString lang = settings.value("language").toString();
    if (lang.isEmpty()) {
        QString locale = QLocale::system().name();
        lang = locale.left(2);
    }
    QString appDir = QCoreApplication::applicationDirPath();
    QString tsPath = appDir + "/../Resources/insynic_" + lang + ".ts";
    if (QFile::exists(tsPath)) {
        if (m_translator.load(tsPath)) {
            qApp->installTranslator(&m_translator);
            m_currentLanguage = lang;
        }
    } else {
        m_currentLanguage = "en";
    }

    setupUi();
    createMenuBar();
    createTrayIcon();

    setObjectName("insynic_mainwindow");

    m_fileManager = new InsynicFileManager(this);
    m_fileManager->setAdbPath(m_adbPath);

    refreshDeviceList();
}

InsynicMainWindow::~InsynicMainWindow()
{
    for (InsynicDeviceWindow *w : m_deviceWindows) {
        delete w;
    }
    m_deviceWindows.clear();
}

QString
InsynicMainWindow::findAdbPath()
{
    QString envPath = qgetenv("INSYNIC_ADB");
    if (!envPath.isEmpty() && QFile::exists(envPath)) {
        return envPath;
    }

    QString appDir = QCoreApplication::applicationDirPath();
    QString bundlePath = QDir(appDir).absoluteFilePath("../Resources/adb");
    if (QFile::exists(bundlePath)) {
        return QDir::cleanPath(bundlePath);
    }

    QString devPath = "/Users/avenue/IDE/scrcpy-macos-x86_64-v4.0/adb";
    if (QFile::exists(devPath)) {
        return devPath;
    }

    return "adb";
}

QString
InsynicMainWindow::findServerPath()
{
    QString envPath = qgetenv("INSYNIC_SERVER");
    if (!envPath.isEmpty() && QFile::exists(envPath)) {
        return envPath;
    }

    QString appDir = QCoreApplication::applicationDirPath();
    QString bundlePath = QDir(appDir).absoluteFilePath("../Resources/scrcpy-server");
    if (QFile::exists(bundlePath)) {
        return QDir::cleanPath(bundlePath);
    }

    QString devPath = "/Users/avenue/IDE/scrcpy-macos-x86_64-v4.0/scrcpy-server";
    if (QFile::exists(devPath)) {
        return devPath;
    }

    return "scrcpy-server";
}

void
InsynicMainWindow::refreshDeviceList()
{
    qDebug() << "[MainWindow] refreshDeviceList called";
    QStringList devices = m_fileManager->listDevices();
    qDebug() << "[MainWindow] Found" << devices.size() << "device(s)";

    QMap<QString, QString> deviceNames;
    for (const QString &serial : devices) {
        QString name = m_fileManager->getDeviceName(serial);
        if (!name.isEmpty()) {
            deviceNames[serial] = name;
            qDebug() << "[MainWindow] Device" << serial << "name:" << name;
        }
    }

    m_controlPanel->updateDeviceList(devices, deviceNames);
}

void
InsynicMainWindow::setupUi()
{
    setWindowTitle("insynic");

    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);

    m_mainLayout = new QHBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    m_controlPanel = new InsynicControlPanel(m_centralWidget);
    m_mainLayout->addWidget(m_controlPanel);

    connect(m_controlPanel, &InsynicControlPanel::connectRequested,
            this, &InsynicMainWindow::onConnectClicked);
    connect(m_controlPanel, &InsynicControlPanel::disconnectRequested,
            this, &InsynicMainWindow::onDisconnectClicked);
    connect(m_controlPanel, &InsynicControlPanel::disconnectAllRequested,
            this, &InsynicMainWindow::onDisconnectAllClicked);
    connect(m_controlPanel, &InsynicControlPanel::connectAllRequested,
            this, &InsynicMainWindow::onConnectAllClicked);
    connect(m_controlPanel, &InsynicControlPanel::fileManagerRequested,
            this, &InsynicMainWindow::onFileManagerClicked);
    connect(m_controlPanel, &InsynicControlPanel::networkConnectOptionSelected,
            this, &InsynicMainWindow::onNetworkConnectOptionSelected);
    connect(m_controlPanel, &InsynicControlPanel::disconnectNetworkDeviceRequested,
            this, &InsynicMainWindow::onDisconnectNetworkDeviceRequested);
    connect(m_controlPanel, &InsynicControlPanel::deviceSettingsRequested,
            this, &InsynicMainWindow::onDeviceSettingsRequested);
    connect(m_controlPanel, &InsynicControlPanel::recordSettingsRequested,
            this, &InsynicMainWindow::onRecordSettingsRequested);
    connect(m_controlPanel, &InsynicControlPanel::refreshRequested,
            this, &InsynicMainWindow::onRefreshClicked);

    m_sdlEventTimer = new QTimer(this);
    m_sdlEventTimer->setInterval(16);
    connect(m_sdlEventTimer, &QTimer::timeout, this, &InsynicMainWindow::processGlobalSdlEvents);
    m_sdlEventTimer->start();
}

void
InsynicMainWindow::createMenuBar()
{
    QMenuBar *menuBar = this->menuBar();

    QMenu *appMenu = menuBar->addMenu(tr("insynic"));

    appMenu->addSeparator();

    QMenu *languageMenu = appMenu->addMenu(tr("Language"));
    QActionGroup *langGroup = new QActionGroup(this);
    langGroup->setExclusive(true);

    QAction *enAction = new QAction("English", this);
    enAction->setCheckable(true);
    enAction->setData("en");
    if (m_currentLanguage == "en") {
        enAction->setChecked(true);
    }
    langGroup->addAction(enAction);
    languageMenu->addAction(enAction);

    QAction *zhAction = new QAction("中文", this);
    zhAction->setCheckable(true);
    zhAction->setData("zh");
    if (m_currentLanguage == "zh") {
        zhAction->setChecked(true);
    }
    langGroup->addAction(zhAction);
    languageMenu->addAction(zhAction);

    connect(langGroup, &QActionGroup::triggered,
            this, &InsynicMainWindow::onLanguageChanged);

    appMenu->addSeparator();

    QAction *aboutAction = new QAction(tr("About insynic"), this);
    connect(aboutAction, &QAction::triggered, this, &InsynicMainWindow::onAboutClicked);
    appMenu->addAction(aboutAction);

    QAction *quitAction = new QAction(tr("Quit insynic"), this);
    quitAction->setShortcut(QKeySequence("Ctrl+Q"));
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
    appMenu->addAction(quitAction);
}

void
InsynicMainWindow::createTrayIcon()
{
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(QIcon(":/icons/insynic.png"));

    m_trayMenu = new QMenu(this);
    QAction *showAction = new QAction(tr("Show"), this);
    connect(showAction, &QAction::triggered, this, [this]() {
        show();
        raise();
        activateWindow();
    });
    m_trayMenu->addAction(showAction);

    m_trayMenu->addSeparator();

    QAction *quitAction = new QAction(tr("Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
    m_trayMenu->addAction(quitAction);

    m_trayIcon->setContextMenu(m_trayMenu);
    m_trayIcon->show();
}

void
InsynicMainWindow::onConnectClicked(const QString &serial)
{
    qDebug() << "[MainWindow] ===== onConnectClicked called =====";
    qDebug() << "[MainWindow] Requested serial:" << (serial.isEmpty() ? "(empty - will use first device)" : serial);
    qDebug() << "[MainWindow] Current device windows count:" << m_deviceWindows.size()
             << "isDisconnectingAll:" << m_isDisconnectingAll;

    QString useSerial = serial;

    if (useSerial.isEmpty()) {
        qDebug() << "[MainWindow] Serial is empty, refreshing device list...";
        QStringList devices = m_fileManager->listDevices();
        m_controlPanel->updateDeviceList(devices);

        qDebug() << "[MainWindow] Found" << devices.size() << "device(s):" << devices;

        if (devices.isEmpty()) {
            qWarning() << "[MainWindow] No devices found!";
            QMessageBox::warning(this, tr("No Device"),
                tr("No Android device found.\n\n"
                   "Please connect your device via USB and enable USB debugging."));
            return;
        }
        useSerial = devices.first();
        qDebug() << "[MainWindow] Using first device:" << useSerial;
    }

    qDebug() << "[MainWindow] Checking if device window already exists for:" << useSerial;
    for (InsynicDeviceWindow *w : m_deviceWindows) {
        qDebug() << "[MainWindow]   existing window serial:" << w->serial()
                 << "addr=" << w;
        if (w->serial() == useSerial) {
            qDebug() << "[MainWindow] Device window already exists, raising it";
            w->raise();
            w->activateWindow();
            return;
        }
    }

    qDebug() << "[MainWindow] Creating new device window...";
    qDebug() << "[MainWindow] adbPath:" << m_adbPath;
    qDebug() << "[MainWindow] serverPath:" << m_serverPath;
    qDebug() << "[MainWindow] maxSize:" << m_maxSize;
    qDebug() << "[MainWindow] maxFps:" << m_maxFps;
    qDebug() << "[MainWindow] videoBitRate:" << m_videoBitRate;

    DeviceStreamingSettings devSettings =
        InsynicDeviceSettingsProfile::instance().loadSettings(useSerial);
    int useMaxSize = devSettings.maxSize > 0 ? devSettings.maxSize : m_maxSize;
    int useMaxFps = devSettings.maxFps > 0 ? devSettings.maxFps : m_maxFps;
    int useVideoBitRate = devSettings.videoBitRate > 0
        ? devSettings.videoBitRate * 1000000
        : m_videoBitRate;

    // Recording settings: only pass file path if recording is enabled
    QString recordFilePath = devSettings.recordEnabled ? devSettings.recordFilePath : QString();

    InsynicDeviceWindow *deviceWindow = new InsynicDeviceWindow(
        useSerial, m_adbPath, m_serverPath,
        useMaxSize, useMaxFps, useVideoBitRate,
        devSettings.turnScreenOff,
        devSettings.stayAwake,
        devSettings.powerOn,
        devSettings.disableScreensaver,
        devSettings.controlEnabled,
        devSettings.audioEnabled,
        devSettings.audioBitRate,
        devSettings.audioCodec,
        devSettings.audioSource,
        recordFilePath,
        devSettings.recordFormat,
        devSettings.recordVideo,
        devSettings.recordAudio);

    qDebug() << "[MainWindow] Connecting deviceWindow signals...";
    connect(deviceWindow, &InsynicDeviceWindow::disconnected,
            this, &InsynicMainWindow::onDeviceWindowClosed);
    connect(deviceWindow, &InsynicDeviceWindow::connectionMessage,
            this, [this](const QString &serial, const QString &message) {
        m_controlPanel->updateConnectionMessage(serial, message);
    });

    m_deviceWindows.append(deviceWindow);
    qDebug() << "[MainWindow] Total device windows now:" << m_deviceWindows.size();
    m_controlPanel->updateConnectionStatus(useSerial, true);
    m_controlPanel->updateConnectionMessage(useSerial, tr("Connecting..."));
    qDebug() << "[MainWindow] onConnectClicked completed (device window hidden, SDL window will show on connect)";
}

void
InsynicMainWindow::onDisconnectClicked(const QString &serial)
{
    qDebug() << "[MainWindow] ===== onDisconnectClicked called, serial:" << serial << "=====";
    qDebug() << "[MainWindow] Current device windows count:" << m_deviceWindows.size();

    for (int i = 0; i < m_deviceWindows.size(); i++) {
        if (m_deviceWindows[i]->serial() == serial) {
            qDebug() << "[MainWindow] Found device window at index" << i << ", closing it";
            m_sdlEventTimer->stop();
            m_deviceWindows[i]->close();
            break;
        }
    }
}

void
InsynicMainWindow::onDisconnectAllClicked()
{
    if (m_deviceWindows.isEmpty()) {
        return;
    }

    if (m_isDisconnectingAll) {
        return;
    }

    m_isDisconnectingAll = true;
    m_sdlEventTimer->stop();

    disconnectNextDevice();
}

void
InsynicMainWindow::onConnectAllClicked()
{
    QStringList devices = m_fileManager->listDevices();
    
    if (devices.isEmpty()) {
        QMessageBox::warning(this, tr("No Device"),
            tr("No Android device found.\n\n"
               "Please connect your device via USB and enable USB debugging."));
        return;
    }

    for (const QString &serial : devices) {
        bool alreadyConnected = false;
        for (InsynicDeviceWindow *w : m_deviceWindows) {
            if (w->serial() == serial) {
                alreadyConnected = true;
                break;
            }
        }
        if (!alreadyConnected) {
            onConnectClicked(serial);
        }
    }
}

void
InsynicMainWindow::disconnectNextDevice()
{
    if (m_deviceWindows.isEmpty()) {
        qDebug() << "[MainWindow] All devices disconnected";
        m_isDisconnectingAll = false;
        return;
    }

    InsynicDeviceWindow *w = m_deviceWindows.first();
    qDebug() << "[MainWindow] Disconnecting next device:" << w->serial();
    w->close();
}

void
InsynicMainWindow::onFileManagerClicked()
{
    qDebug() << "[MainWindow] ===== onFileManagerClicked called =====";

    QString selectedSerial = m_controlPanel->currentDevice();
    qDebug() << "[MainWindow] Control panel selected device:" << selectedSerial;

    if (selectedSerial.isEmpty()) {
        qDebug() << "[MainWindow] No device selected";
        QMessageBox::warning(this, tr("No Device Selected"),
            tr("Please select a device first."));
        return;
    }

    qDebug() << "[MainWindow] Using device serial for file manager:" << selectedSerial;
    m_fileManager->setSerial(selectedSerial);

    if (!m_fileBrowser) {
        qDebug() << "[MainWindow] Creating new file browser dialog";
        m_fileBrowser = new InsynicFileBrowserDialog(m_fileManager, this);
    }
    m_fileBrowser->show();
    m_fileBrowser->raise();
    m_fileBrowser->activateWindow();
}

void
InsynicMainWindow::onRefreshClicked()
{
    refreshDeviceList();
}

void
InsynicMainWindow::onAboutClicked()
{
    QMessageBox::about(this, tr("About insynic"),
        tr("insynic 1.22\n\n"
           "A macOS application for controlling Android devices.\n\n"
           "Features:\n"
           "- Screen mirroring via scrcpy\n"
           "- File management via ADB\n"
           "- OTG input support\n"
           "- Remote control buttons\n"
           "- Network (WiFi) connection\n"
           "- Virtual key mapping\n"
           "- Profile management\n"
           "- Multi-device independent control\n\n"
           "Author: 厨房 / OMADAFAKA\n\n"
           "Credits:\n"
           "- scrcpy (screen mirroring)\n"
           "- Android Debug Bridge (ADB)\n"
           "- Qt framework\n"
           "- SDL (video rendering)\n"
           "- FFmpeg (video decoding)\n\n"
           "Thanks to all the developers of these great tools."));
}

void
InsynicMainWindow::onLanguageChanged(QAction *action)
{
    QString lang = action->data().toString();
    m_currentLanguage = lang;

    qApp->removeTranslator(&m_translator);

    QString appDir = QCoreApplication::applicationDirPath();
    QString tsPath = appDir + "/../Resources/insynic_" + lang + ".ts";
    if (QFile::exists(tsPath)) {
        if (m_translator.load(tsPath)) {
            qApp->installTranslator(&m_translator);
        }
    }

    QSettings settings;
    settings.setValue("language", lang);

    this->recreateMenuBar();

    QCoreApplication::processEvents();
    this->update();
    this->repaint();

    if (m_controlPanel) {
        m_controlPanel->retranslateUi();
    }

    if (m_fileBrowser) {
        m_fileBrowser->retranslateUi();
    }

    for (InsynicDeviceWindow *w : m_deviceWindows) {
        w->retranslateUi();
    }
}

void
InsynicMainWindow::recreateMenuBar()
{
    menuBar()->clear();
    createMenuBar();
}

void
InsynicMainWindow::onDeviceSettingsRequested(const QString &serial)
{
    if (serial.isEmpty()) {
        return;
    }

    DeviceStreamingSettings s = InsynicDeviceSettingsProfile::instance().loadSettings(serial);

    InsynicSettingsDialog dialog(this);
    dialog.setMaxSize(s.maxSize);
    dialog.setMaxFps(s.maxFps);
    dialog.setVideoBitRate(s.videoBitRate);
    dialog.setTurnScreenOff(s.turnScreenOff);
    dialog.setStayAwake(s.stayAwake);
    dialog.setPowerOn(s.powerOn);
    dialog.setDisableScreensaver(s.disableScreensaver);
    dialog.setControlEnabled(s.controlEnabled);
    dialog.setAudioEnabled(s.audioEnabled);
    dialog.setAudioBitRate(s.audioBitRate);
    dialog.setAudioCodec(s.audioCodec);
    dialog.setAudioSource(s.audioSource);
    dialog.setWindowTitle(tr("Streaming Settings - %1").arg(serial));

    if (dialog.exec() == QDialog::Accepted) {
        s.maxSize = dialog.maxSize();
        s.maxFps = dialog.maxFps();
        s.videoBitRate = dialog.videoBitRate();
        s.turnScreenOff = dialog.turnScreenOff();
        s.stayAwake = dialog.stayAwake();
        s.powerOn = dialog.powerOn();
        s.disableScreensaver = dialog.disableScreensaver();
        s.controlEnabled = dialog.controlEnabled();
        s.audioEnabled = dialog.audioEnabled();
        s.audioBitRate = dialog.audioBitRate();
        s.audioCodec = dialog.audioCodec();
        s.audioSource = dialog.audioSource();
        InsynicDeviceSettingsProfile::instance().saveSettings(serial, s);

        for (InsynicDeviceWindow *w : m_deviceWindows) {
            if (w->serial() == serial) {
                QMessageBox::information(this, tr("Streaming Settings"),
                    tr("New settings will be applied on next connection.\n"
                       "Please disconnect and reconnect the device."));
                break;
            }
        }
    }
}

void
InsynicMainWindow::onRecordSettingsRequested(const QString &serial)
{
    if (serial.isEmpty()) {
        return;
    }

    DeviceStreamingSettings s = InsynicDeviceSettingsProfile::instance().loadSettings(serial);

    InsynicRecordDialog dialog(this);
    dialog.setRecordEnabled(s.recordEnabled);
    dialog.setFilePath(s.recordFilePath);
    dialog.setFormat(s.recordFormat);
    dialog.setRecordVideo(s.recordVideo);
    dialog.setRecordAudio(s.recordAudio);
    dialog.setWindowTitle(tr("Record Settings - %1").arg(serial));

    if (dialog.exec() == QDialog::Accepted) {
        s.recordEnabled = dialog.recordEnabled();
        s.recordFilePath = dialog.filePath();
        s.recordFormat = dialog.format();
        s.recordVideo = dialog.recordVideo();
        s.recordAudio = dialog.recordAudio();
        InsynicDeviceSettingsProfile::instance().saveSettings(serial, s);

        for (InsynicDeviceWindow *w : m_deviceWindows) {
            if (w->serial() == serial) {
                QMessageBox::information(this, tr("Record Settings"),
                    tr("New settings will be applied on next connection.\n"
                       "Please disconnect and reconnect the device."));
                break;
            }
        }
    }
}

void
InsynicMainWindow::onNetworkConnectOptionSelected()
{
    InsynicNetworkDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString ip = dialog.ipAddress();
        int port = dialog.port();

        if (ip.isEmpty()) {
            QMessageBox::warning(this, tr("Error"), tr("IP address cannot be empty."));
            return;
        }

        m_fileManager->connectDevice(ip, port);

        refreshDeviceList();

        QString serial = QString("%1:%2").arg(ip).arg(port);
        if (m_fileManager->listDevices().contains(serial)) {
            QMessageBox::information(this, tr("Success"),
                tr("Connected to device at %1:%2").arg(ip).arg(port));
        } else {
            QMessageBox::warning(this, tr("Error"),
                tr("Failed to connect to device at %1:%2").arg(ip).arg(port));
        }
    }

    refreshDeviceList();
}

void
InsynicMainWindow::onDisconnectNetworkDeviceRequested(const QString &serial)
{
    // Confirm before disconnecting
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("Disconnect Network Device"),
        tr("Disconnect network device %1?\n\nThis will run 'adb disconnect' for this device.")
            .arg(serial),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    if (reply != QMessageBox::Yes) {
        return;
    }

    // If device window is connected, close it first
    for (InsynicDeviceWindow *w : m_deviceWindows) {
        if (w->serial() == serial) {
            w->close();
            break;
        }
    }

    bool ok = m_fileManager->disconnectDevice(serial);
    refreshDeviceList();

    if (ok) {
        QMessageBox::information(this, tr("Success"),
            tr("Network device %1 has been disconnected.").arg(serial));
    } else {
        QMessageBox::warning(this, tr("Error"),
            tr("Failed to disconnect network device %1.").arg(serial));
    }
}

void
InsynicMainWindow::setTouchSyncEnabled(bool enabled)
{
    if (m_touchSyncEnabled == enabled) {
        return;
    }
    m_touchSyncEnabled = enabled;
    qDebug() << "[MainWindow] Touch sync" << (enabled ? "enabled" : "disabled");
    emit touchSyncToggled(enabled);
}

void
InsynicMainWindow::onDeviceWindowClosed(const QString &serial)
{
    qDebug() << "[MainWindow] ===== onDeviceWindowClosed called, serial:" << serial << "=====";
    qDebug() << "[MainWindow] Current device windows count:" << m_deviceWindows.size();
    qDebug() << "[MainWindow] isDisconnectingAll:" << m_isDisconnectingAll;

    m_controlPanel->updateConnectionStatus(serial, false);

    qDebug() << "[MainWindow] Searching for window to remove...";
    for (int i = 0; i < m_deviceWindows.size(); i++) {
        qDebug() << "[MainWindow]   index" << i << "serial:" << m_deviceWindows[i]->serial()
                 << "addr=" << m_deviceWindows[i];
        if (m_deviceWindows[i]->serial() == serial) {
            qDebug() << "[MainWindow] Removing device window at index" << i;
            m_deviceWindows.removeAt(i);
            qDebug() << "[MainWindow] Device windows remaining:" << m_deviceWindows.size();
            break;
        }
    }

    if (m_isDisconnectingAll) {
        qDebug() << "[MainWindow] Disconnect all in progress, checking next device...";
        if (m_deviceWindows.isEmpty()) {
            m_isDisconnectingAll = false;
            m_sdlEventTimer->start();
            qDebug() << "[MainWindow] All devices disconnected, restarting SDL event timer";
        } else {
            QTimer::singleShot(50, this, &InsynicMainWindow::disconnectNextDevice);
        }
    } else if (m_deviceWindows.isEmpty()) {
        m_sdlEventTimer->start();
    }
}

void
InsynicMainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    
    static bool firstShow = true;
    if (firstShow) {
        firstShow = false;
        QScreen *screen = QGuiApplication::primaryScreen();
        QRect screenRect = screen->availableGeometry();
        int margin = 0;
        int x = screenRect.width() - width() - margin;
        int y = screenRect.height() - height() - margin;
        move(x, y);
        qDebug() << "[MainWindow] Positioned at bottom-right (" << x << ", " << y << ")";
    }
}

void
InsynicMainWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << "[MainWindow] closeEvent called";

    if (m_sdlEventTimer) {
        m_sdlEventTimer->stop();
    }

    for (InsynicDeviceWindow *w : m_deviceWindows) {
        w->close();
    }
    m_deviceWindows.clear();

    if (m_trayIcon) {
        m_trayIcon->hide();
    }

    event->accept();
}

void
InsynicMainWindow::processGlobalSdlEvents()
{
    if (m_deviceWindows.isEmpty()) {
        return;
    }

    static int s_eventLogCounter = 0;
    int eventsProcessed = 0;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        eventsProcessed++;
        bool handled = false;

        if (event.type == SDL_EVENT_QUIT) {
            SDL_PushEvent(&event);
            for (InsynicDeviceWindow *w : m_deviceWindows) {
                w->processSdlEvent(&event);
            }
            continue;
        }

        SDL_WindowID targetWindowID = 0;
        bool hasWindowID = false;

        if (event.type >= SDL_EVENT_WINDOW_FIRST && event.type <= SDL_EVENT_WINDOW_LAST) {
            targetWindowID = event.window.windowID;
            hasWindowID = true;
        } else if (event.type == SDL_EVENT_MOUSE_MOTION) {
            targetWindowID = event.motion.windowID;
            hasWindowID = true;
        } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
                   event.type == SDL_EVENT_MOUSE_BUTTON_UP ||
                   event.type == SDL_EVENT_MOUSE_WHEEL) {
            targetWindowID = event.button.windowID;
            hasWindowID = true;
        } else if (event.type == SDL_EVENT_FINGER_DOWN ||
                   event.type == SDL_EVENT_FINGER_UP ||
                   event.type == SDL_EVENT_FINGER_MOTION) {
            targetWindowID = event.tfinger.windowID;
            hasWindowID = true;
        } else if (event.type == SDL_EVENT_KEY_DOWN ||
                   event.type == SDL_EVENT_KEY_UP) {
            targetWindowID = event.key.windowID;
            hasWindowID = true;
        } else if (event.type >= SDL_EVENT_USER) {
            void *data1 = event.user.data1;
            if (data1) {
                for (InsynicDeviceWindow *w : m_deviceWindows) {
                    struct insynic_scrcpy *sc = w->scrcpy();
                    if (sc && insynic_scrcpy_is_screen_initialized(sc)) {
                        struct sc_screen *screen = insynic_scrcpy_get_screen(sc);
                        if (screen == data1) {
                            w->processSdlEvent(&event);
                            handled = true;
                            break;
                        }
                    }
                }
            }
            continue;
        }

        if (hasWindowID) {
            bool isInputEvent =
                (event.type == SDL_EVENT_MOUSE_MOTION) ||
                (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) ||
                (event.type == SDL_EVENT_MOUSE_BUTTON_UP) ||
                (event.type == SDL_EVENT_MOUSE_WHEEL) ||
                (event.type == SDL_EVENT_FINGER_DOWN) ||
                (event.type == SDL_EVENT_FINGER_UP) ||
                (event.type == SDL_EVENT_FINGER_MOTION) ||
                (event.type == SDL_EVENT_KEY_DOWN) ||
                (event.type == SDL_EVENT_KEY_UP);

            if (m_touchSyncEnabled && isInputEvent) {
                for (InsynicDeviceWindow *w : m_deviceWindows) {
                    w->processSdlEvent(&event);
                }
                handled = true;
            } else {
                for (InsynicDeviceWindow *w : m_deviceWindows) {
                    SDL_Window *win = w->sdlWindow();
                    if (win && SDL_GetWindowID(win) == targetWindowID) {
                        w->processSdlEvent(&event);
                        handled = true;
                        break;
                    }
                }
            }
        }
    }

    // Periodic state log (every ~5 seconds = ~300 ticks at 16ms)
    s_eventLogCounter++;
    if (s_eventLogCounter >= 300) {
        s_eventLogCounter = 0;
        qDebug() << "[SDL] Periodic state - deviceWindows:" << m_deviceWindows.size()
                 << "eventsProcessed:" << eventsProcessed;
        for (int i = 0; i < m_deviceWindows.size(); i++) {
            InsynicDeviceWindow *w = m_deviceWindows[i];
            struct insynic_scrcpy *sc = w->scrcpy();
            if (sc) {
                enum insynic_scrcpy_state st = insynic_scrcpy_get_state(sc);
                bool running = insynic_scrcpy_is_running(sc);
                bool screenInit = insynic_scrcpy_is_screen_initialized(sc);
                SDL_Window *win = insynic_scrcpy_get_window(sc);
                SDL_WindowID winId = win ? SDL_GetWindowID(win) : 0;
                qDebug() << "[SDL]   Device[" << i << "] serial:" << w->serial()
                         << "state:" << st << "running:" << running
                         << "screenInit:" << screenInit
                         << "windowID:" << winId
                         << "otgMode:" << w->isOtgMode();
            } else {
                qDebug() << "[SDL]   Device[" << i << "] serial:" << w->serial()
                         << "scrcpy: NULL";
            }
        }
    }
}
