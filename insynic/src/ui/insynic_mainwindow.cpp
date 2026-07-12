#include "insynic_mainwindow.h"
#include "insynic_filebrowser.h"
#include "insynic_settingsdialog.h"
#include "insynic_customtranslator.h"
#include "insynic_draggablekey.h"
#include "insynic_saveprofiledialog.h"

#include <QMainWindow>
#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include <QStyle>
#include <QDir>
#include <QLocale>
#include <QCoreApplication>
#include <QFile>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>
#include <QMenu>
#include <QMenuBar>
#include <QWindow>
#include <QSettings>

extern "C" void createStatusBarMenu();
extern "C" void setupNativeMenuBar();

class InsynicMainWindow;
static void eventCallback(void *userdata);

static QString
findAdbPath()
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

static QString
findServerPath()
{
    QString envPath = qgetenv("INSYNIC_SERVER");
    if (!envPath.isEmpty() && QFile::exists(envPath)) {
        return envPath;
    }

    QString bundlePath = QCoreApplication::applicationDirPath()
        + "/../Resources/scrcpy-server";
    if (QFile::exists(bundlePath)) {
        return QDir::cleanPath(bundlePath);
    }

    QString devPath = "/Users/avenue/IDE/scrcpy-macos-x86_64-v4.0/scrcpy-server";
    if (QFile::exists(devPath)) {
        return devPath;
    }

    return "scrcpy-server";
}

InsynicMainWindow::InsynicMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_scrcpy(nullptr)
    , m_fileManager(nullptr)
    , m_fileBrowser(nullptr)
    , m_stateChangePending(false)
    , m_pendingState(0)
    , m_currentLanguage("en")
    , m_maxSize(1080)
    , m_maxFps(60)
    , m_videoBitRate(8)
    , m_profileManager(new InsynicProfileManager(this))
    , m_keyContainer(nullptr)
    , m_isDraggingKey(false)
{
    m_adbPath = findAdbPath();
    m_serverPath = findServerPath();
    m_otgMode = false;
    m_otgProcess = nullptr;

    m_fileManager = new InsynicFileManager(this);
    m_fileManager->setAdbPath(m_adbPath);

    QSettings settings("insynic", "insynic");
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

    m_stateTimer = new QTimer(this);
    m_stateTimer->setInterval(100);
    connect(m_stateTimer, &QTimer::timeout,
            this, &InsynicMainWindow::checkState);
    m_stateTimer->start();

    createTrayIcon();

    QStringList devices = m_fileManager->listDevices();
    m_controlPanel->updateDeviceList(devices);
}

InsynicMainWindow::~InsynicMainWindow()
{
    stopScrcpy();
}

void
InsynicMainWindow::setupUi()
{
    setWindowTitle(tr("insynic"));

    m_centralWidget = new QWidget(this);
    m_mainLayout = new QHBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    m_controlPanel = new InsynicControlPanel(m_centralWidget);
    m_mainLayout->addWidget(m_controlPanel);

    setCentralWidget(m_centralWidget);

    connect(m_controlPanel, &InsynicControlPanel::connectRequested,
            this, &InsynicMainWindow::onConnectClicked);
    connect(m_controlPanel, &InsynicControlPanel::disconnectRequested,
            this, &InsynicMainWindow::onDisconnectClicked);
    connect(m_controlPanel, &InsynicControlPanel::fileManagerRequested,
            this, &InsynicMainWindow::onFileManagerClicked);
    connect(m_controlPanel, &InsynicControlPanel::deviceSelected,
            this, &InsynicMainWindow::onDeviceSelected);
    connect(m_controlPanel, &InsynicControlPanel::otgInputRequested,
            this, &InsynicMainWindow::onOtgInputClicked);
    connect(m_controlPanel, &InsynicControlPanel::networkConnectOptionSelected,
            this, &InsynicMainWindow::onNetworkConnectOptionSelected);
    connect(m_controlPanel, &InsynicControlPanel::addKeyRequested,
            this, &InsynicMainWindow::onAddKeyClicked);
    connect(m_controlPanel, &InsynicControlPanel::profileSelected,
            this, &InsynicMainWindow::onProfileSelected);
    connect(m_controlPanel, &InsynicControlPanel::saveProfileRequested,
            this, &InsynicMainWindow::onSaveProfileClicked);

    adjustSize();
    setMinimumSize(size());

    qApp->installEventFilter(this);

    QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
    int x = screenGeometry.width() - width() - 20;
    int y = screenGeometry.height() - height() - 80;
    move(x, y);
}



void
InsynicMainWindow::onDeviceSelected(const QString &serial)
{
    m_selectedSerial = serial;
    m_fileManager->setSerial(serial);
    m_controlPanel->setSerial(serial);
    if (!serial.contains(":")) {
        m_controlPanel->setNetworkConnected(false);
    }
}

void
InsynicMainWindow::onConnectClicked()
{
    QStringList devices = m_fileManager->listDevices();
    m_controlPanel->updateDeviceList(devices);

    if (devices.isEmpty()) {
        QMessageBox::warning(this, "No Device",
            "No Android device found.\n\n"
            "Please connect your device via USB and enable USB debugging.");
        return;
    }

    if (m_selectedSerial.isEmpty() && !devices.isEmpty()) {
        m_selectedSerial = devices.first();
    }

    startScrcpy();
}

void
InsynicMainWindow::onNetworkConnectOptionSelected()
{
    InsynicNetworkDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        m_controlPanel->updateDeviceList(m_fileManager->listDevices());
        return;
    }

    QString ip = dialog.ipAddress();
    int port = dialog.port();

    if (ip.isEmpty()) {
        QMessageBox::warning(this, tr("Input Error"),
            tr("Please enter the device IP address."));
        m_controlPanel->updateDeviceList(m_fileManager->listDevices());
        return;
    }

    bool connected = m_fileManager->connectDevice(ip, port);
    if (!connected) {
        QMessageBox::warning(this, tr("Connection Failed"),
            tr("Failed to connect to device via network.\n\n"
               "Please ensure:\n"
               "1. Device has USB debugging enabled\n"
               "2. Device is on the same network as your computer\n"
               "3. Wireless debugging is enabled on the device\n"
               "4. IP address and port are correct"));
        m_controlPanel->updateDeviceList(m_fileManager->listDevices());
        return;
    }

    QStringList devices = m_fileManager->listDevices();
    m_controlPanel->updateDeviceList(devices);

    QString networkSerial = QString("%1:%2").arg(ip).arg(port);
    int idx = devices.indexOf(networkSerial);
    if (idx >= 0) {
        m_selectedSerial = networkSerial;
        m_fileManager->setSerial(networkSerial);
        m_controlPanel->setSerial(networkSerial);
        m_controlPanel->setNetworkConnected(true);
    }
}

void
InsynicMainWindow::onDisconnectClicked()
{
    stopScrcpy();
}

void
InsynicMainWindow::onOtgInputClicked()
{
    if (m_otgMode) {
        if (m_otgProcess) {
            m_otgProcess->terminate();
            m_otgProcess = nullptr;
        }
        m_otgMode = false;
        m_controlPanel->setOtgMode(false);
        return;
    }

    QString scrcpyPath = "/Users/avenue/IDE/scrcpy-macos-x86_64-v4.0/scrcpy";
    if (!QFile::exists(scrcpyPath)) {
        scrcpyPath = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("scrcpy");
    }
    if (!QFile::exists(scrcpyPath)) {
        scrcpyPath = "scrcpy";
    }

    QStringList args;
    args << "--otg";
    if (!m_selectedSerial.isEmpty()) {
        args << "-s" << m_selectedSerial;
    }

    QRect geo = geometry();
    int otgX = geo.x();
    int otgY = geo.y() - 200;
    if (otgY < 0) {
        otgY = geo.y() + geo.height() + 10;
    }
    args << "--window-x" << QString::number(otgX);
    args << "--window-y" << QString::number(otgY);

    m_otgProcess = new QProcess(this);
    connect(m_otgProcess, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        Q_UNUSED(exitCode);
        Q_UNUSED(exitStatus);
        m_otgMode = false;
        m_controlPanel->setOtgMode(false);
        if (m_otgProcess) {
            m_otgProcess->deleteLater();
            m_otgProcess = nullptr;
        }
    });

    m_otgProcess->start(scrcpyPath, args);

    if (!m_otgProcess->waitForStarted()) {
        QMessageBox::critical(this, "Error", "Failed to start OTG scrcpy");
        m_otgProcess->deleteLater();
        m_otgProcess = nullptr;
        return;
    }

    m_otgMode = true;
    m_controlPanel->setOtgMode(true);
}

void
InsynicMainWindow::onFileManagerClicked()
{
    if (!m_fileBrowser) {
        m_fileBrowser = new InsynicFileBrowserDialog(m_fileManager, this);
    }
    m_fileBrowser->refresh();
    m_fileBrowser->show();
    m_fileBrowser->raise();
    m_fileBrowser->activateWindow();
}

void
InsynicMainWindow::startScrcpy()
{
    if (m_scrcpy) {
        return;
    }

    struct insynic_scrcpy_config config;
    memset(&config, 0, sizeof(config));

    QByteArray serialArr = m_selectedSerial.toUtf8();
    if (!m_selectedSerial.isEmpty()) {
        config.serial = serialArr.constData();
    }

    QByteArray adbArr = m_adbPath.toUtf8();
    config.adb_path = adbArr.constData();

    QByteArray serverArr = m_serverPath.toUtf8();
    config.server_path = serverArr.constData();
    config.max_size = m_maxSize;
    config.max_fps = m_maxFps;
    config.video_bit_rate = m_videoBitRate;
    config.video_enabled = true;
    config.audio_enabled = false;
    config.control_enabled = true;
    config.turn_screen_off = false;

    config.window_width = 0;
    config.window_height = 0;
    config.window_x = 0;
    config.window_y = 0;

    m_scrcpy = insynic_scrcpy_create(&config);
    if (!m_scrcpy) {
        QMessageBox::critical(this, "Error", "Failed to create scrcpy instance");
        return;
    }

    insynic_scrcpy_set_state_callback(m_scrcpy, stateCallback, this);
    insynic_scrcpy_set_event_callback(m_scrcpy, eventCallback, this);

    m_controlPanel->setScrcpy(m_scrcpy);

    if (!insynic_scrcpy_start(m_scrcpy)) {
        QMessageBox::critical(this, "Error", "Failed to start scrcpy");
        insynic_scrcpy_destroy(m_scrcpy);
        m_scrcpy = nullptr;
        m_controlPanel->setScrcpy(nullptr);
        return;
    }
}

void
InsynicMainWindow::stopScrcpy()
{
    if (!m_scrcpy) {
        return;
    }

    removeAllVirtualKeys();

    struct insynic_scrcpy *scrcpy = m_scrcpy;
    m_scrcpy = nullptr;
    m_controlPanel->setScrcpy(nullptr);
    m_controlPanel->setConnected(false);

    auto watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, this, [this, scrcpy, watcher]() {
        insynic_scrcpy_destroy(scrcpy);
        watcher->deleteLater();
    });

    QtConcurrent::run([scrcpy]() {
        insynic_scrcpy_stop(scrcpy);
    });
}

void
InsynicMainWindow::stateCallback(enum insynic_scrcpy_state state,
                                 void *userdata)
{
    InsynicMainWindow *self = static_cast<InsynicMainWindow *>(userdata);
    self->m_pendingState = state;
    self->m_stateChangePending = true;
}

static void
eventCallback(void *userdata) {
    InsynicMainWindow *self = static_cast<InsynicMainWindow *>(userdata);
    QMetaObject::invokeMethod(self, "checkState", Qt::QueuedConnection);
}

void
InsynicMainWindow::checkState()
{
    if (m_scrcpy) {
        insynic_scrcpy_process_events(m_scrcpy);
    }
    
    if (!m_stateChangePending) {
        return;
    }
    m_stateChangePending = false;
    int state = m_pendingState;
    handleStateChange((enum insynic_scrcpy_state)state);
}

void
InsynicMainWindow::createMenuBar()
{
    QMenuBar *menuBar = this->menuBar();
    if (!menuBar) {
        menuBar = new QMenuBar(this);
        setMenuBar(menuBar);
    }

    menuBar->clear();
    menuBar->setNativeMenuBar(true);

    // On macOS, the first menu becomes the app menu (bold title in menu bar).
    // Use a minimal app menu with just About and Quit.
    QMenu *appMenu = menuBar->addMenu(tr("insynic"));

    QAction *aboutAction = new QAction(tr("About insynic"), this);
    connect(aboutAction, &QAction::triggered, this, &InsynicMainWindow::onAboutClicked);
    appMenu->addAction(aboutAction);

    appMenu->addSeparator();

    QAction *quitAction = new QAction(tr("Quit insynic"), this);
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, []() {
        QCoreApplication::quit();
    });
    appMenu->addAction(quitAction);

    // Setting menu (second menu)
    QMenu *settingMenu = menuBar->addMenu(tr("Setting"));

    QMenu *languageMenu = settingMenu->addMenu(tr("Language"));

    QAction *englishAction = new QAction(tr("English"), this);
    englishAction->setData("en");
    englishAction->setCheckable(true);
    englishAction->setChecked(m_currentLanguage == "en");
    connect(englishAction, &QAction::triggered, this, [this, englishAction]() {
        onLanguageChanged(englishAction);
    });
    languageMenu->addAction(englishAction);

    QAction *chineseAction = new QAction(tr("Chinese"), this);
    chineseAction->setData("zh");
    chineseAction->setCheckable(true);
    chineseAction->setChecked(m_currentLanguage == "zh");
    connect(chineseAction, &QAction::triggered, this, [this, chineseAction]() {
        onLanguageChanged(chineseAction);
    });
    languageMenu->addAction(chineseAction);

    settingMenu->addSeparator();

    QAction *settingsAction = new QAction(tr("Streaming Settings"), this);
    connect(settingsAction, &QAction::triggered, this, &InsynicMainWindow::onSettingsClicked);
    settingMenu->addAction(settingsAction);

    // File menu
    menuBar->addMenu(tr("File"));

    // Edit menu
    QMenu *editMenu = menuBar->addMenu(tr("Edit"));
    QAction *cutAction = new QAction(tr("Cut"), this);
    cutAction->setShortcut(QKeySequence::Cut);
    editMenu->addAction(cutAction);
    QAction *copyAction = new QAction(tr("Copy"), this);
    copyAction->setShortcut(QKeySequence::Copy);
    editMenu->addAction(copyAction);
    QAction *pasteAction = new QAction(tr("Paste"), this);
    pasteAction->setShortcut(QKeySequence::Paste);
    editMenu->addAction(pasteAction);
    QAction *selectAllAction = new QAction(tr("Select All"), this);
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    editMenu->addAction(selectAllAction);

    // Window menu
    QMenu *windowMenu = menuBar->addMenu(tr("Window"));
    QAction *minimizeAction = new QAction(tr("Minimize"), this);
    minimizeAction->setShortcut(QKeySequence(Qt::Key_M | Qt::MetaModifier));
    connect(minimizeAction, &QAction::triggered, this, &QWidget::showMinimized);
    windowMenu->addAction(minimizeAction);
}

void
InsynicMainWindow::recreateMenuBar()
{
    createMenuBar();
}

void
InsynicMainWindow::handleStateChange(enum insynic_scrcpy_state state)
{
    switch (state) {
    case INSYNIC_SCRCPY_STATE_CONNECTED:
        m_controlPanel->setConnected(true);
        break;
    case INSYNIC_SCRCPY_STATE_DISCONNECTED:
        m_controlPanel->setConnected(false);
        break;
    case INSYNIC_SCRCPY_STATE_ERROR:
        m_controlPanel->setConnected(false);
        QMessageBox::warning(this, "Connection Error",
            "Failed to connect to device.");
        break;
    case INSYNIC_SCRCPY_STATE_CONNECTING:
    case INSYNIC_SCRCPY_STATE_IDLE:
    default:
        break;
    }
}

void
InsynicMainWindow::onAboutClicked()
{
    QMessageBox::about(this, tr("About insynic"),
        tr("insynic 1.11 Lite\n\n"
           "A macOS application for controlling Android devices.\n\n"
           "Features:\n"
           "- Screen mirroring via scrcpy\n"
           "- File management via ADB\n"
           "- OTG input support\n"
           "- Remote control buttons\n"
           "- Network (WiFi) connection\n"
           "- Virtual key mapping\n"
           "- Profile management\n\n"
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

    QSettings settings("insynic", "insynic");
    settings.setValue("language", lang);

    qApp->removeTranslator(&m_translator);

    if (lang != "en") {
        QString translationPath = QCoreApplication::applicationDirPath()
            + "/../Resources/insynic_" + lang + ".ts";
        if (QFile::exists(translationPath)) {
            if (m_translator.load(translationPath)) {
                qApp->installTranslator(&m_translator);
            }
        }
    }

    // Recreate menu bar so menu items pick up the new language
    createMenuBar();

    // Update window title
    setWindowTitle(tr("insynic"));

    // Recreate tray icon menu
    if (m_trayMenu) {
        delete m_trayMenu;
        m_trayMenu = nullptr;
    }
    createTrayIcon();

    // Update control panel labels by reconstructing it
    if (m_controlPanel && m_centralWidget) {
        bool wasConnected = m_controlPanel->isConnected();
        bool wasOtgMode = m_otgMode;
        QString currentSerial = m_selectedSerial;
        QStringList devices;
        if (m_fileManager) {
            devices = m_fileManager->listDevices();
        }

        m_mainLayout->removeWidget(m_controlPanel);
        delete m_controlPanel;
        m_controlPanel = nullptr;

        m_controlPanel = new InsynicControlPanel(m_centralWidget);
        m_mainLayout->addWidget(m_controlPanel);

        connect(m_controlPanel, &InsynicControlPanel::connectRequested,
                this, &InsynicMainWindow::onConnectClicked);
        connect(m_controlPanel, &InsynicControlPanel::disconnectRequested,
                this, &InsynicMainWindow::onDisconnectClicked);
        connect(m_controlPanel, &InsynicControlPanel::fileManagerRequested,
                this, &InsynicMainWindow::onFileManagerClicked);
        connect(m_controlPanel, &InsynicControlPanel::deviceSelected,
                this, &InsynicMainWindow::onDeviceSelected);
        connect(m_controlPanel, &InsynicControlPanel::otgInputRequested,
                this, &InsynicMainWindow::onOtgInputClicked);
        connect(m_controlPanel, &InsynicControlPanel::networkConnectOptionSelected,
                this, &InsynicMainWindow::onNetworkConnectOptionSelected);
        connect(m_controlPanel, &InsynicControlPanel::addKeyRequested,
                this, &InsynicMainWindow::onAddKeyClicked);
        connect(m_controlPanel, &InsynicControlPanel::profileSelected,
                this, &InsynicMainWindow::onProfileSelected);
        connect(m_controlPanel, &InsynicControlPanel::saveProfileRequested,
                this, &InsynicMainWindow::onSaveProfileClicked);

        if (m_scrcpy) {
            m_controlPanel->setScrcpy(m_scrcpy);
        }
        m_controlPanel->setConnected(wasConnected);
        m_controlPanel->updateDeviceList(devices);
        if (!currentSerial.isEmpty()) {
            m_controlPanel->setSerial(currentSerial);
        }
        if (wasOtgMode) {
            m_controlPanel->setOtgMode(true);
        }

        adjustSize();
    }

    if (m_fileBrowser) {
        delete m_fileBrowser;
        m_fileBrowser = nullptr;
    }
}

void
InsynicMainWindow::onSettingsClicked()
{
    InsynicSettingsDialog dialog(this);
    dialog.setMaxSize(m_maxSize);
    dialog.setMaxFps(m_maxFps);
    dialog.setVideoBitRate(m_videoBitRate);

    connect(&dialog, &InsynicSettingsDialog::settingsChanged,
            this, &InsynicMainWindow::onSettingsChanged);

    dialog.exec();
}

void
InsynicMainWindow::onSettingsChanged(int maxSize, int maxFps, int videoBitRate)
{
    m_maxSize = maxSize;
    m_maxFps = maxFps;
    m_videoBitRate = videoBitRate;
}

void
InsynicMainWindow::createTrayIcon()
{
    m_trayIcon = new QSystemTrayIcon(this);
    QIcon icon = QIcon::fromTheme("network-wireless");
    if (icon.isNull()) {
        QPixmap pixmap(32, 32);
        pixmap.fill(QColor(66, 133, 244));
        icon = QIcon(pixmap);
    }
    m_trayIcon->setIcon(icon);
    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        m_trayIcon->show();
    }
    
    m_trayMenu = new QMenu(this);
    
    QAction *quitAction = new QAction(tr("Quit"), this);
    connect(quitAction, &QAction::triggered, this, &QWidget::close);
    m_trayMenu->addAction(quitAction);
    
    m_trayIcon->setContextMenu(m_trayMenu);
}

void
InsynicMainWindow::closeEvent(QCloseEvent *event)
{
    if (m_scrcpy) {
        struct insynic_scrcpy *scrcpy = m_scrcpy;
        m_scrcpy = nullptr;
        m_controlPanel->setScrcpy(nullptr);

        QtConcurrent::run([scrcpy]() {
            insynic_scrcpy_stop(scrcpy);
            insynic_scrcpy_destroy(scrcpy);
        });
    }
    QMainWindow::closeEvent(event);
}

void
InsynicMainWindow::onAddKeyClicked()
{
    double centerX = 0.5;
    double centerY = 0.5;
    VirtualKey key(Qt::Key_A, "A", centerX, centerY, 40);
    addVirtualKey(key);
}

void
InsynicMainWindow::addVirtualKey(const VirtualKey &key)
{
    if (!m_scrcpy) {
        return;
    }
    
    if (!m_keyContainer) {
        m_keyContainer = new QWidget();
        m_keyContainer->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool | Qt::WindowTransparentForInput);
        m_keyContainer->setAttribute(Qt::WA_TranslucentBackground);
        
        int x, y, w, h;
        if (insynic_scrcpy_get_window_position(m_scrcpy, &x, &y) &&
            insynic_scrcpy_get_window_size(m_scrcpy, &w, &h)) {
            m_keyContainer->setGeometry(x, y, w, h);
        }
        
        m_keyContainer->show();
        
        QTimer *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &InsynicMainWindow::updateKeyContainerPosition);
        timer->start(100);
    }
    
    InsynicDraggableKey *draggableKey = new InsynicDraggableKey(key);
    draggableKey->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    draggableKey->setAttribute(Qt::WA_TranslucentBackground);
    draggableKey->setFocusPolicy(Qt::StrongFocus);
    
    int x, y, w, h;
    insynic_scrcpy_get_window_position(m_scrcpy, &x, &y);
    insynic_scrcpy_get_window_size(m_scrcpy, &w, &h);
    
    int screenX = key.getScreenX(w);
    int screenY = key.getScreenY(h);
    draggableKey->move(x + screenX - key.size / 2, y + screenY - key.size / 2);
    draggableKey->setScreenSize(w, h);
    draggableKey->setScrcpyPosition(x, y);
    draggableKey->show();
    
    connect(draggableKey, &InsynicDraggableKey::configRequested,
            this, &InsynicMainWindow::onKeyConfigRequested);
    connect(draggableKey, &InsynicDraggableKey::deleteRequested,
            this, &InsynicMainWindow::onKeyDeleteRequested);
    connect(draggableKey, &InsynicDraggableKey::positionChanged,
            this, &InsynicMainWindow::onKeyPositionChanged);
    connect(draggableKey, &InsynicDraggableKey::draggingStarted,
            this, [this]() { m_isDraggingKey = true; });
    connect(draggableKey, &InsynicDraggableKey::draggingEnded,
            this, [this]() { m_isDraggingKey = false; });
    
    m_virtualKeys.append(draggableKey);
}

void
InsynicMainWindow::removeAllVirtualKeys()
{
    foreach (InsynicDraggableKey *key, m_virtualKeys) {
        delete key;
    }
    m_virtualKeys.clear();
    
    if (m_keyContainer) {
        delete m_keyContainer;
        m_keyContainer = nullptr;
    }
}

void
InsynicMainWindow::updateKeyContainerPosition()
{
    if (!m_keyContainer || !m_scrcpy || m_isDraggingKey) {
        return;
    }
    
    int x, y, w, h;
    if (insynic_scrcpy_get_window_position(m_scrcpy, &x, &y) &&
        insynic_scrcpy_get_window_size(m_scrcpy, &w, &h)) {
        m_keyContainer->setGeometry(x, y, w, h);
        
        foreach (InsynicDraggableKey *key, m_virtualKeys) {
            key->setScreenSize(w, h);
            key->setScrcpyPosition(x, y);
            VirtualKey vk = key->getKey();
            int screenX = vk.getScreenX(w);
            int screenY = vk.getScreenY(h);
            key->move(x + screenX - vk.size / 2, y + screenY - vk.size / 2);
        }
    }
}

void
InsynicMainWindow::onKeyConfigRequested(InsynicDraggableKey *key)
{
    bool keyDeleted = false;
    VirtualKey currentKey = key->getKey();
    InsynicKeyConfigDialog dialog(currentKey, this);
    
    connect(&dialog, &InsynicKeyConfigDialog::keyDeleted,
            this, [this, key, &keyDeleted]() {
        onKeyDeleteRequested(key);
        keyDeleted = true;
    });
    
    if (dialog.exec() == QDialog::Accepted && !keyDeleted) {
        VirtualKey newKey = dialog.getKey();
        key->setKey(newKey);
    }
}

void
InsynicMainWindow::onKeyDeleteRequested(InsynicDraggableKey *key)
{
    m_virtualKeys.removeOne(key);
    delete key;
}

void
InsynicMainWindow::onKeyPositionChanged(int x, int y)
{
    Q_UNUSED(x);
    Q_UNUSED(y);
}

void
InsynicMainWindow::onSaveProfileClicked()
{
    QStringList existingNames = m_profileManager->getProfileNames();
    InsynicSaveProfileDialog dialog(existingNames, this);
    if (dialog.exec() == QDialog::Accepted) {
        QString name = dialog.getProfileName();
        Profile profile(name);
        
        foreach (InsynicDraggableKey *dk, m_virtualKeys) {
            profile.keys.append(dk->getKey());
        }
        
        if (m_profileManager->saveProfile(profile)) {
            QMessageBox::information(this, tr("Success"), tr("Profile saved successfully."));
            m_controlPanel->updateProfileCombo();
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Failed to save profile."));
        }
    }
}

void
InsynicMainWindow::onProfileSelected(const QString &name)
{
    Profile profile;
    if (m_profileManager->loadProfile(name, profile)) {
        removeAllVirtualKeys();
        foreach (const VirtualKey &key, profile.keys) {
            addVirtualKey(key);
        }
    }
}

void
InsynicMainWindow::keyPressEvent(QKeyEvent *event)
{
    int keyCode = event->key();
    
    int devW, devH;
    if (!insynic_scrcpy_get_device_size(m_scrcpy, &devW, &devH)) {
        QMainWindow::keyPressEvent(event);
        return;
    }
    
    foreach (InsynicDraggableKey *key, m_virtualKeys) {
        VirtualKey vk = key->getKey();
        if (vk.keyCode == keyCode && m_scrcpy) {
            int screenX = vk.getScreenX(devW);
            int screenY = vk.getScreenY(devH);
            insynic_scrcpy_inject_touch(m_scrcpy, screenX, screenY, devW, devH);
            break;
        }
    }
    
    QMainWindow::keyPressEvent(event);
}

bool
InsynicMainWindow::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);
    
    if (event->type() == QEvent::KeyPress && !m_virtualKeys.isEmpty() && m_scrcpy) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        int keyCode = keyEvent->key();
        
        int devW, devH;
        if (insynic_scrcpy_get_device_size(m_scrcpy, &devW, &devH)) {
            foreach (InsynicDraggableKey *key, m_virtualKeys) {
                VirtualKey vk = key->getKey();
                if (vk.keyCode == keyCode) {
                    int screenX = vk.getScreenX(devW);
                    int screenY = vk.getScreenY(devH);
                    insynic_scrcpy_inject_touch(m_scrcpy, screenX, screenY, devW, devH);
                    return true;
                }
            }
        }
    }
    
    return false;
}
