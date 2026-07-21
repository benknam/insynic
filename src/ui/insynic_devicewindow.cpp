#include "insynic_devicewindow.h"
#include <QMessageBox>
#include <QtConcurrent>
#include <QFutureWatcher>
#include <QKeyEvent>
#include <QWindow>
#include <QGuiApplication>
#include <QApplication>
#include <QVBoxLayout>
#include <QDebug>
#include <time.h>
#include "insynic_keyconfigdialog.h"
#include "insynic_saveprofiledialog.h"
#include "insynic_profilemanager.h"

InsynicDeviceWindow::InsynicDeviceWindow(const QString &serial, const QString &adbPath,
                                         const QString &serverPath, int maxSize, int maxFps,
                                         int videoBitRate,
                                         bool turnScreenOff,
                                         bool stayAwake,
                                         bool powerOn,
                                         bool disableScreensaver,
                                         bool controlEnabled,
                                         bool audioEnabled,
                                         int audioBitRate,
                                         int audioCodec,
                                         int audioSource,
                                         const QString &recordFilePath,
                                         int recordFormat,
                                         bool recordVideo,
                                         bool recordAudio,
                                         QWidget *parent)
    : QWidget(parent)
    , m_serial(serial)
    , m_adbPath(adbPath)
    , m_serverPath(serverPath)
    , m_maxSize(maxSize)
    , m_maxFps(maxFps)
    , m_videoBitRate(videoBitRate)
    , m_turnScreenOff(turnScreenOff)
    , m_stayAwake(stayAwake)
    , m_powerOn(powerOn)
    , m_disableScreensaver(disableScreensaver)
    , m_controlEnabled(controlEnabled)
    , m_audioEnabled(audioEnabled)
    , m_audioBitRate(audioBitRate)
    , m_audioCodec(audioCodec)
    , m_audioSource(audioSource)
    , m_recordFilePath(recordFilePath)
    , m_recordFormat(recordFormat)
    , m_recordVideo(recordVideo)
    , m_recordAudio(recordAudio)
    , m_scrcpy(nullptr)
    , m_connected(false)
    , m_isClosing(false)
    , m_closePollCount(0)
    , m_stateChangePending(false)
    , m_pendingState(INSYNIC_SCRCPY_STATE_IDLE)
    , m_keyContainer(nullptr)
    , m_isDraggingKey(false)
    , m_scrcpyWindowX(0)
    , m_scrcpyWindowY(0)
    , m_scrcpyWindowWidth(0)
    , m_scrcpyWindowHeight(0)
    , m_otgProcess(nullptr)
    , m_otgMode(false)
    , m_scrcpyProcess(nullptr)
    , m_isNetworkConnection(serial.contains(":") && serial.contains("."))
{
    qDebug() << "[DeviceWindow] ===== Creating device window =====";
    qDebug() << "[DeviceWindow] serial:" << serial;
    qDebug() << "[DeviceWindow] adbPath:" << adbPath;
    qDebug() << "[DeviceWindow] serverPath:" << serverPath;
    qDebug() << "[DeviceWindow] maxSize:" << maxSize;
    qDebug() << "[DeviceWindow] maxFps:" << maxFps;
    qDebug() << "[DeviceWindow] videoBitRate:" << videoBitRate;
    qDebug() << "[DeviceWindow] isNetworkConnection:" << m_isNetworkConnection;

    setWindowTitle(tr("insynic - %1").arg(serial));
    setVisible(false);

    m_controlBar = new InsynicControlBar(this);
    m_controlBar->setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    m_controlBar->setAttribute(Qt::WA_TranslucentBackground, false);

    m_sidePanel = new InsynicSidePanel(this);
    m_sidePanel->setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    m_sidePanel->setAttribute(Qt::WA_TranslucentBackground, false);

    connect(m_controlBar, &InsynicControlBar::addKeyRequested,
            this, &InsynicDeviceWindow::onAddKeyRequested);
    connect(m_controlBar, &InsynicControlBar::profileSelected,
            this, &InsynicDeviceWindow::onProfileSelected);
    connect(m_controlBar, &InsynicControlBar::saveProfileRequested,
            this, &InsynicDeviceWindow::onSaveProfileRequested);

    connect(m_sidePanel, &InsynicSidePanel::otgInputRequested,
            this, &InsynicDeviceWindow::onOtgInputRequested);
    connect(m_sidePanel, &InsynicSidePanel::disconnectRequested,
            this, &InsynicDeviceWindow::close);

    qDebug() << "[DeviceWindow] Configuring scrcpy...";

    struct insynic_scrcpy_config config;
    memset(&config, 0, sizeof(config));

    QByteArray serialArr = serial.toUtf8();
    if (!serial.isEmpty()) {
        config.serial = serialArr.constData();
    }

    QByteArray adbArr = adbPath.toUtf8();
    config.adb_path = adbArr.constData();

    QByteArray serverArr = serverPath.toUtf8();
    config.server_path = serverArr.constData();
    config.max_size = maxSize;
    config.max_fps = maxFps;
    config.video_bit_rate = videoBitRate;
    config.video_enabled = true;
    config.audio_enabled = audioEnabled;
    config.audio_bit_rate = audioBitRate * 1000;  // convert kbps to bps
    config.audio_codec = audioCodec;
    config.audio_source = audioSource;
    config.control_enabled = controlEnabled;
    config.turn_screen_off = turnScreenOff;
    config.stay_awake = stayAwake;
    config.power_on = powerOn;
    config.disable_screensaver = disableScreensaver;

    config.window_width = 0;
    config.window_height = 0;
    config.window_x = 0;
    config.window_y = 0;

    // Recording options: only apply if a record file path is provided
    if (!recordFilePath.isEmpty()) {
        // Save the byte array as member to prevent dangling pointer
        m_recordFileData = recordFilePath.toUtf8();
        config.record_filename = m_recordFileData.constData();
        config.record_format = recordFormat;
        config.record_video = recordVideo;
        config.record_audio = recordAudio;
    } else {
        config.record_filename = NULL;
        config.record_format = 0;
        config.record_video = false;
        config.record_audio = false;
    }

    qDebug() << "[DeviceWindow] Creating scrcpy instance...";
    m_scrcpy = insynic_scrcpy_create(&config);
    
    if (!m_scrcpy) {
        qCritical() << "[DeviceWindow] ERROR: Failed to create scrcpy instance!";
        QMessageBox::critical(this, tr("Error"), tr("Failed to create scrcpy instance"));
        return;
    }
    qDebug() << "[DeviceWindow] scrcpy instance created successfully";

    insynic_scrcpy_set_state_callback(m_scrcpy, stateCallback, this);

    m_controlBar->setScrcpy(m_scrcpy);
    m_sidePanel->setScrcpy(m_scrcpy);

    qDebug() << "[DeviceWindow] Starting scrcpy...";
    if (!insynic_scrcpy_start(m_scrcpy)) {
        qCritical() << "[DeviceWindow] ERROR: Failed to start scrcpy!";
        QMessageBox::critical(this, tr("Error"), tr("Failed to start scrcpy"));
        insynic_scrcpy_destroy(m_scrcpy);
        m_scrcpy = nullptr;
        return;
    }
    qDebug() << "[DeviceWindow] scrcpy started successfully, waiting for connection...";

    m_stateTimer = new QTimer(this);
    m_stateTimer->setInterval(100);
    connect(m_stateTimer, &QTimer::timeout, this, &InsynicDeviceWindow::checkState);
    m_stateTimer->start();

    m_keyContainerTimer = new QTimer(this);
    m_keyContainerTimer->setInterval(100);
    connect(m_keyContainerTimer, &QTimer::timeout, this, &InsynicDeviceWindow::updateKeyContainerPosition);
    m_keyContainerTimer->start();

    m_sdlEventTimer = new QTimer(this);
    m_sdlEventTimer->setInterval(16);
    connect(m_sdlEventTimer, &QTimer::timeout, this, &InsynicDeviceWindow::processSdlEvents);

    qApp->installEventFilter(this);
    qDebug() << "[DeviceWindow] Device window created successfully";
}

InsynicDeviceWindow::~InsynicDeviceWindow()
{
    qDebug() << "[DeviceWindow] ===== ~InsynicDeviceWindow destructor called, serial:" << m_serial << "=====";
    qDebug() << "[DeviceWindow] m_scrcpy:" << m_scrcpy
             << "m_isClosing:" << m_isClosing
             << "m_connected:" << m_connected;

    if (m_scrcpy) {
        qWarning() << "[DeviceWindow] WARNING: m_scrcpy still exists in destructor! Cleaning up...";
        insynic_scrcpy_destroy(m_scrcpy);
        m_scrcpy = nullptr;
    }

    if (m_otgProcess) {
        qDebug() << "[DeviceWindow] Cleaning up OTG process";
        m_otgProcess->kill();
        m_otgProcess->waitForFinished();
        delete m_otgProcess;
        m_otgProcess = nullptr;
    }

    if (m_scrcpyProcess) {
        qDebug() << "[DeviceWindow] Cleaning up scrcpy process";
        m_scrcpyProcess->kill();
        m_scrcpyProcess->waitForFinished();
        delete m_scrcpyProcess;
        m_scrcpyProcess = nullptr;
    }

    qDebug() << "[DeviceWindow] ===== destructor completed =====";
}

void
InsynicDeviceWindow::stateCallback(enum insynic_scrcpy_state state, void *userdata)
{
    InsynicDeviceWindow *self = static_cast<InsynicDeviceWindow*>(userdata);
    qDebug() << "[DeviceWindow] stateCallback called, state:" << state;
    self->m_pendingState = state;
    self->m_stateChangePending = true;
}

void
InsynicDeviceWindow::checkState()
{
    if (m_isClosing) {
        return;
    }
    if (!m_stateChangePending) {
        return;
    }
    m_stateChangePending = false;
    int state = m_pendingState;
    qDebug() << "[DeviceWindow] checkState: processing state change, state:" << state;
    handleStateChange((enum insynic_scrcpy_state)state);
}

void
InsynicDeviceWindow::handleStateChange(enum insynic_scrcpy_state state)
{
    qDebug() << "[DeviceWindow] ===== handleStateChange:" << state
             << "this=" << this << "serial:" << m_serial
             << "m_isClosing=" << m_isClosing << "=====";

    switch (state) {
    case INSYNIC_SCRCPY_STATE_CONNECTING:
        qDebug() << "[DeviceWindow] State: CONNECTING";
        emit connectionMessage(m_serial, tr("Starting scrcpy server..."));
        break;
    case INSYNIC_SCRCPY_STATE_CONNECTED:
        qDebug() << "[DeviceWindow] State: CONNECTED!";
        qDebug() << "[DeviceWindow] Initializing main thread (screen, decoders, etc.)...";
        if (!insynic_scrcpy_init_main_thread_safe(m_scrcpy)) {
            qCritical() << "[DeviceWindow] ERROR: Failed to initialize main thread!";
            QMessageBox::critical(this, tr("Error"), tr("Failed to initialize screen"));
            close();
            return;
        }
        qDebug() << "[DeviceWindow] Main thread initialized successfully";
        m_connected = true;
        if (m_controlBar) {
            m_controlBar->setConnected(true);
        }
        if (m_sidePanel) {
            m_sidePanel->setConnected(true, m_isNetworkConnection);
        }
        qDebug() << "[DeviceWindow] Control bar and side panel updated";
        emit connectionMessage(m_serial, tr("Connected"));
        break;
    case INSYNIC_SCRCPY_STATE_DISCONNECTED:
        qDebug() << "[DeviceWindow] State: DISCONNECTED";
        m_connected = false;
        if (m_controlBar) {
            m_controlBar->setConnected(false);
        }
        if (m_sidePanel) {
            m_sidePanel->setConnected(false, m_isNetworkConnection);
        }
        if (!m_isClosing) {
            qDebug() << "[DeviceWindow] Emitting disconnected signal";
            emit disconnected(m_serial);
        }
        break;
    case INSYNIC_SCRCPY_STATE_ERROR:
        qCritical() << "[DeviceWindow] State: ERROR - Connection failed!";
        m_connected = false;
        if (m_controlBar) {
            m_controlBar->setConnected(false);
        }
        if (m_sidePanel) {
            m_sidePanel->setConnected(false, m_isNetworkConnection);
        }
        if (!m_isClosing) {
            qCritical() << "[DeviceWindow] Showing connection error message box";
            emit connectionMessage(m_serial, tr("Connection failed"));
            emit disconnected(m_serial);
            QMessageBox::warning(this, tr("Connection Error"),
                tr("Failed to connect to device."));
        }
        break;
    case INSYNIC_SCRCPY_STATE_IDLE:
        qDebug() << "[DeviceWindow] State: IDLE";
        break;
    default:
        qWarning() << "[DeviceWindow] Unknown state:" << state;
        break;
    }
}

void
InsynicDeviceWindow::updateKeyContainerPosition()
{
    if (m_isClosing || !m_scrcpy || m_isDraggingKey) {
        return;
    }

    int x, y, w, h;
    bool pos_ok = insynic_scrcpy_get_window_position(m_scrcpy, &x, &y);
    bool size_ok = insynic_scrcpy_get_window_size(m_scrcpy, &w, &h);
    
    if (pos_ok && size_ok) {
        m_scrcpyWindowX = x;
        m_scrcpyWindowY = y;
        m_scrcpyWindowWidth = w;
        m_scrcpyWindowHeight = h;

        if (m_keyContainer) {
            m_keyContainer->setGeometry(x, y, w, h);
        }

        for (InsynicDraggableKey *dk : m_virtualKeys) {
            dk->setScreenSize(w, h);
            dk->setScrcpyPosition(x, y);
            VirtualKey vk = dk->getKey();
            int screenX = vk.getScreenX(w);
            int screenY = vk.getScreenY(h);
            dk->move(x + screenX - vk.size / 2, y + screenY - vk.size / 2);
        }

        if (m_controlBar && m_connected) {
                int barHeight = m_controlBar->minimumSizeHint().height();
                m_controlBar->resize(w, barHeight);
                int barY = y + h;
                m_controlBar->move(x, barY);
                if (!m_controlBar->isVisible()) {
                    m_controlBar->show();
                }
            }

            if (m_sidePanel && m_connected) {
                int panelWidth = m_sidePanel->width();
                int panelX = x + w;
                m_sidePanel->setFixedHeight(h);
                m_sidePanel->move(panelX, y);
                if (!m_sidePanel->isVisible()) {
                    m_sidePanel->show();
                }
            }
    }
}

void
InsynicDeviceWindow::addVirtualKey(const VirtualKey &key)
{
    if (!m_scrcpy) {
        return;
    }

    if (!m_keyContainer) {
        m_keyContainer = new QWidget(this);
        m_keyContainer->setWindowFlags(Qt::FramelessWindowHint | Qt::Window | Qt::WindowTransparentForInput);
        m_keyContainer->setAttribute(Qt::WA_TranslucentBackground);

        if (insynic_scrcpy_get_window_position(m_scrcpy, &m_scrcpyWindowX, &m_scrcpyWindowY) &&
            insynic_scrcpy_get_window_size(m_scrcpy, &m_scrcpyWindowWidth, &m_scrcpyWindowHeight)) {
            m_keyContainer->setGeometry(m_scrcpyWindowX, m_scrcpyWindowY, m_scrcpyWindowWidth, m_scrcpyWindowHeight);
        }

        m_keyContainer->show();
    }

    InsynicDraggableKey *dk = new InsynicDraggableKey(key);
    dk->setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    dk->setAttribute(Qt::WA_TranslucentBackground);
    dk->setFocusPolicy(Qt::StrongFocus);

    int x = m_scrcpyWindowX;
    int y = m_scrcpyWindowY;
    int w = m_scrcpyWindowWidth;
    int h = m_scrcpyWindowHeight;

    int screenX = key.getScreenX(w);
    int screenY = key.getScreenY(h);
    dk->move(x + screenX - key.size / 2, y + screenY - key.size / 2);
    dk->setScreenSize(w, h);
    dk->setScrcpyPosition(x, y);
    dk->show();

    connect(dk, &InsynicDraggableKey::destroyed, this, [this, dk]() {
        m_virtualKeys.removeOne(dk);
    });
    connect(dk, &InsynicDraggableKey::draggingStarted,
            this, [this]() { m_isDraggingKey = true; });
    connect(dk, &InsynicDraggableKey::draggingEnded,
            this, [this]() { m_isDraggingKey = false; });
    connect(dk, &InsynicDraggableKey::configRequested,
            this, [this](InsynicDraggableKey *key) {
        VirtualKey vk = key->getKey();
        InsynicKeyConfigDialog dialog(vk, this);
        bool keyDeleted = false;
        connect(&dialog, &InsynicKeyConfigDialog::keyDeleted,
                this, [this, key, &keyDeleted]() {
            m_virtualKeys.removeOne(key);
            delete key;
            keyDeleted = true;
        });
        if (dialog.exec() == QDialog::Accepted && !keyDeleted) {
            VirtualKey newKey = dialog.getKey();
            key->setKey(newKey);
        }
    });

    m_virtualKeys.append(dk);
}

void
InsynicDeviceWindow::removeAllVirtualKeys()
{
    for (InsynicDraggableKey *dk : m_virtualKeys) {
        delete dk;
    }
    m_virtualKeys.clear();

    if (m_keyContainer) {
        delete m_keyContainer;
        m_keyContainer = nullptr;
    }
}

void
InsynicDeviceWindow::applyProfile(const QString &profileName)
{
    InsynicProfileManager manager;
    Profile profile;
    if (manager.loadProfile(profileName, profile)) {
        removeAllVirtualKeys();
        for (const VirtualKey &key : profile.keys) {
            addVirtualKey(key);
        }
    }
}

void
InsynicDeviceWindow::onAddKeyRequested()
{
    VirtualKey defaultKey(Qt::Key_A, "A", 0.5, 0.5, 40);
    InsynicKeyConfigDialog dialog(defaultKey, this);
    if (dialog.exec() == QDialog::Accepted) {
        VirtualKey key = dialog.getKey();
        addVirtualKey(key);
    }
}

void
InsynicDeviceWindow::onProfileSelected(const QString &name)
{
    applyProfile(name);
}

void
InsynicDeviceWindow::onSaveProfileRequested()
{
    InsynicProfileManager manager;
    QStringList existingNames = manager.getProfileNames();
    InsynicSaveProfileDialog dialog(existingNames, this);
    if (dialog.exec() == QDialog::Accepted) {
        QString name = dialog.getProfileName();
        if (!name.isEmpty()) {
            Profile profile;
            profile.name = name;
            profile.windowWidth = width();
            profile.windowHeight = height();
            profile.windowX = x();
            profile.windowY = y();

            for (InsynicDraggableKey *dk : m_virtualKeys) {
                profile.keys.append(dk->getKey());
            }

            manager.saveProfile(profile);
            m_controlBar->updateProfileCombo();
            QMessageBox::information(this, tr("Success"), tr("Profile saved successfully."));
        }
    }
}

void
InsynicDeviceWindow::onCloseRequested()
{
    close();
}

void
InsynicDeviceWindow::processSdlEvents()
{
    if (!m_scrcpy) {
        return;
    }
    // Note: This function calls SDL_PollEvent which competes with MainWindow's processGlobalSdlEvents
    // The MainWindow's timer handles all event polling; this is kept for backwards compatibility
    insynic_scrcpy_process_events(m_scrcpy);
}

void
InsynicDeviceWindow::processSdlEvent(const SDL_Event *event)
{
    if (!m_scrcpy) {
        return;
    }

    if (event->type == SDL_EVENT_WINDOW_FOCUS_GAINED) {
        if (m_controlBar && m_connected) {
            m_controlBar->raise();
        }
        if (m_sidePanel && m_connected) {
            m_sidePanel->raise();
        }
        if (m_keyContainer) {
            m_keyContainer->raise();
        }
        for (InsynicDraggableKey *dk : m_virtualKeys) {
            dk->raise();
        }
    }

    insynic_scrcpy_handle_event(m_scrcpy, event);
}

SDL_Window*
InsynicDeviceWindow::sdlWindow() const
{
    if (!m_scrcpy) {
        return nullptr;
    }
    return insynic_scrcpy_get_window(m_scrcpy);
}

void
InsynicDeviceWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << "[DeviceWindow] ===== closeEvent called, this=" << this
             << "serial:" << m_serial
             << "m_isClosing:" << m_isClosing
             << "m_scrcpy:" << m_scrcpy
             << "m_connected:" << m_connected << "=====";

    if (m_isClosing) {
        qDebug() << "[DeviceWindow] Already closing (m_isClosing=true), ignore";
        event->ignore();
        return;
    }

    qDebug() << "[DeviceWindow] Setting isClosing flag, this=" << this;
    m_isClosing = true;
    m_closePollCount = 0;
    event->ignore();

    qDebug() << "[DeviceWindow] Stopping timers";
    if (m_stateTimer) {
        qDebug() << "[DeviceWindow]   Stopping m_stateTimer";
        m_stateTimer->stop();
    }
    if (m_keyContainerTimer) {
        qDebug() << "[DeviceWindow]   Stopping m_keyContainerTimer";
        m_keyContainerTimer->stop();
    }
    if (m_sdlEventTimer) {
        qDebug() << "[DeviceWindow]   Stopping m_sdlEventTimer";
        m_sdlEventTimer->stop();
    }

    qDebug() << "[DeviceWindow] Removing event filter";
    qApp->removeEventFilter(this);

    if (m_controlBar) {
        qDebug() << "[DeviceWindow] Closing control bar, addr=" << m_controlBar;
        disconnect(m_controlBar, nullptr, this, nullptr);
        m_controlBar->close();
        m_controlBar = nullptr;
        qDebug() << "[DeviceWindow] Control bar closed";
    }

    if (m_sidePanel) {
        qDebug() << "[DeviceWindow] Closing side panel, addr=" << m_sidePanel;
        disconnect(m_sidePanel, nullptr, this, nullptr);
        m_sidePanel->close();
        m_sidePanel = nullptr;
        qDebug() << "[DeviceWindow] Side panel closed";
    }

    qDebug() << "[DeviceWindow] Removing all virtual keys";
    removeAllVirtualKeys();
    qDebug() << "[DeviceWindow] Virtual keys removed";

    if (m_scrcpy) {
        qDebug() << "[DeviceWindow] Requesting scrcpy stop (async), scrcpy=" << m_scrcpy;
        insynic_scrcpy_request_stop(m_scrcpy);
        qDebug() << "[DeviceWindow] scrcpy stop requested";

        qDebug() << "[DeviceWindow] Starting close progress timer";
        QTimer::singleShot(10, this, &InsynicDeviceWindow::checkCloseProgress);
    } else {
        qDebug() << "[DeviceWindow] No scrcpy, finish closing immediately";
        finishClose();
    }

    qDebug() << "[DeviceWindow] closeEvent returning (async close started)";
}

void
InsynicDeviceWindow::checkCloseProgress()
{
    qDebug() << "[DeviceWindow] checkCloseProgress called, this=" << this
             << "serial:" << m_serial
             << "m_scrcpy:" << m_scrcpy
             << "m_closePollCount:" << m_closePollCount;

    if (!m_scrcpy) {
        qDebug() << "[DeviceWindow] checkCloseProgress: no scrcpy, finish close";
        finishClose();
        return;
    }

    bool threadExited = insynic_scrcpy_is_thread_exited(m_scrcpy);
    qDebug() << "[DeviceWindow] checkCloseProgress: insynic_scrcpy_is_thread_exited returned" << threadExited;

    if (!threadExited) {
        m_closePollCount++;
        qDebug() << "[DeviceWindow] checkCloseProgress: thread still alive, poll" << m_closePollCount;
        if (m_closePollCount <= 20) {
            QTimer::singleShot(10, this, &InsynicDeviceWindow::checkCloseProgress);
            return;
        }
        qDebug() << "[DeviceWindow] checkCloseProgress: poll timeout (20 polls), blocking join";
        insynic_scrcpy_join(m_scrcpy);
        qDebug() << "[DeviceWindow] checkCloseProgress: join completed";
    }

    qDebug() << "[DeviceWindow] checkCloseProgress: thread fully exited, proceeding with destroy, scrcpy=" << m_scrcpy;

    bool screenInitialized = insynic_scrcpy_is_screen_initialized(m_scrcpy);
    qDebug() << "[DeviceWindow] checkCloseProgress: screen initialized=" << screenInitialized;

    if (screenInitialized) {
        qDebug() << "[DeviceWindow] Hiding screen";
        insynic_scrcpy_hide_screen(m_scrcpy);
        qDebug() << "[DeviceWindow] Screen hidden";
    }

    qDebug() << "[DeviceWindow] Destroying scrcpy, addr=" << m_scrcpy;
    insynic_scrcpy_destroy(m_scrcpy);
    qDebug() << "[DeviceWindow] scrcpy_destroy completed";
    m_scrcpy = NULL;
    qDebug() << "[DeviceWindow] m_scrcpy set to NULL";

    qDebug() << "[DeviceWindow] scrcpy destroyed, finishing close";
    finishClose();
}

void
InsynicDeviceWindow::finishClose()
{
    qDebug() << "[DeviceWindow] ===== finishClose called, this=" << this
             << "serial:" << m_serial
             << "m_scrcpy:" << m_scrcpy
             << "m_connected:" << m_connected << "=====";

    qDebug() << "[DeviceWindow] Emitting disconnected signal for serial:" << m_serial;
    emit disconnected(m_serial);
    qDebug() << "[DeviceWindow] Disconnected signal emitted";

    qDebug() << "[DeviceWindow] finishClose completed, calling deleteLater on this=" << this;
    deleteLater();
    qDebug() << "[DeviceWindow] deleteLater called";
}

void
InsynicDeviceWindow::retranslateUi()
{
    setWindowTitle(tr("insynic - %1").arg(m_serial));
    
    if (m_controlBar) {
        m_controlBar->retranslateUi();
    }
    
    if (m_sidePanel) {
        m_sidePanel->retranslateUi();
    }
}

void
InsynicDeviceWindow::keyPressEvent(QKeyEvent *event)
{
    int keyCode = event->key();

    int devW, devH;
    if (!m_scrcpy || !insynic_scrcpy_get_device_size(m_scrcpy, &devW, &devH)) {
        QWidget::keyPressEvent(event);
        return;
    }

    for (InsynicDraggableKey *dk : m_virtualKeys) {
        VirtualKey vk = dk->getKey();
        if (vk.keyCode == keyCode) {
            int screenX = vk.getScreenX(devW);
            int screenY = vk.getScreenY(devH);

            if (vk.toggle) {
                bool isPressed = m_toggleStates.value(keyCode, false);
                insynic_scrcpy_inject_touch_action(m_scrcpy, screenX, screenY, devW, devH, !isPressed);
                m_toggleStates[keyCode] = !isPressed;
            } else {
                insynic_scrcpy_inject_touch(m_scrcpy, screenX, screenY, devW, devH);
            }
            break;
        }
    }

    QWidget::keyPressEvent(event);
}

bool
InsynicDeviceWindow::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);

    if (event->type() == QEvent::KeyPress && !m_virtualKeys.isEmpty() && m_scrcpy) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        int keyCode = keyEvent->key();

        int devW, devH;
        if (insynic_scrcpy_get_device_size(m_scrcpy, &devW, &devH)) {
            for (InsynicDraggableKey *dk : m_virtualKeys) {
                VirtualKey vk = dk->getKey();
                if (vk.keyCode == keyCode) {
                    int screenX = vk.getScreenX(devW);
                    int screenY = vk.getScreenY(devH);

                    if (vk.toggle) {
                        bool isPressed = m_toggleStates.value(keyCode, false);
                        insynic_scrcpy_inject_touch_action(m_scrcpy, screenX, screenY, devW, devH, !isPressed);
                        m_toggleStates[keyCode] = !isPressed;
                    } else {
                        insynic_scrcpy_inject_touch(m_scrcpy, screenX, screenY, devW, devH);
                    }
                    return true;
                }
            }
        }
    }

    return false;
}

void
InsynicDeviceWindow::onOtgInputRequested()
{
    qDebug() << "[OTG] ===== onOtgInputRequested called, serial:" << m_serial
             << "m_otgMode:" << m_otgMode << "m_connected:" << m_connected << "=====";

    if (m_otgMode) {
        qDebug() << "[OTG] Stopping OTG mode for serial:" << m_serial;
        if (m_otgProcess) {
            qDebug() << "[OTG] Terminating OTG process, pid:" << m_otgProcess->processId();
            m_otgProcess->terminate();
            m_otgProcess = nullptr;
        }
        m_otgMode = false;
        qDebug() << "[OTG] OTG mode stopped for serial:" << m_serial;
        return;
    }

    qDebug() << "[OTG] Starting OTG mode for serial:" << m_serial;

    // Check scrcpy connection state before starting OTG
    if (m_scrcpy) {
        enum insynic_scrcpy_state st = insynic_scrcpy_get_state(m_scrcpy);
        qDebug() << "[OTG] Current scrcpy state:" << st
                 << "(0=NONE,1=CONNECTING,2=CONNECTED,3=DISCONNECTED,4=ERROR)";
        bool running = insynic_scrcpy_is_running(m_scrcpy);
        qDebug() << "[OTG] Scrcpy is running:" << running;
        bool screenInit = insynic_scrcpy_is_screen_initialized(m_scrcpy);
        qDebug() << "[OTG] Screen initialized:" << screenInit;
    } else {
        qDebug() << "[OTG] WARNING: m_scrcpy is null!";
    }

    QString scrcpyPath;
    QString appDir = QCoreApplication::applicationDirPath();
    // Look in Contents/Resources/ (app bundle)
    QString bundlePath = QDir(appDir).absoluteFilePath("../Resources/scrcpy");
    if (QFile::exists(bundlePath)) {
        scrcpyPath = QDir::cleanPath(bundlePath);
    }
    // Fallback: dev environment
    if (scrcpyPath.isEmpty()) {
        QString devPath = "/Users/avenue/IDE/scrcpy-macos-x86_64-v4.0/scrcpy";
        if (QFile::exists(devPath)) {
            scrcpyPath = devPath;
        }
    }
    // Fallback: system PATH
    if (scrcpyPath.isEmpty()) {
        scrcpyPath = "scrcpy";
    }
    qDebug() << "[OTG] Using scrcpy path:" << scrcpyPath;

    QStringList args;
    args << "--otg";
    if (!m_serial.isEmpty()) {
        args << "-s" << m_serial;
    }

    QWidget *mainWindow = parentWidget();
    if (!mainWindow) {
        for (QWidget *widget : QApplication::topLevelWidgets()) {
            if (widget->objectName() == "insynic_mainwindow") {
                mainWindow = widget;
                break;
            }
        }
    }
    if (!mainWindow) {
        mainWindow = QApplication::topLevelWidgets().first();
    }
    QRect mainGeo = mainWindow->geometry();
    
    int otgX = mainGeo.x() + (mainGeo.width() - 200) / 2;
    int otgY = mainGeo.y() + 20;
    args << "--window-x" << QString::number(otgX);
    args << "--window-y" << QString::number(otgY);
    args << "--window-width" << "200";
    args << "--window-height" << "100";
    args << "--window-title" << "OTG Input - Cmd+Q to exit";

    qDebug() << "[OTG] Starting OTG process with args:" << args;

    m_otgProcess = new QProcess(this);
    connect(m_otgProcess, &QProcess::started, this, [this]() {
        qDebug() << "[OTG] OTG process STARTED successfully, pid:" << m_otgProcess->processId()
                 << "for serial:" << m_serial;
    });
    connect(m_otgProcess, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        qDebug() << "[OTG] OTG process ERROR:" << error << "for serial:" << m_serial;
    });
    connect(m_otgProcess, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        qDebug() << "[OTG] OTG process FINISHED, exitCode:" << exitCode
                 << "exitStatus:" << exitStatus << "for serial:" << m_serial;
        m_otgMode = false;
        if (m_otgProcess) {
            m_otgProcess->deleteLater();
            m_otgProcess = nullptr;
        }
    });
    connect(m_otgProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        QByteArray data = m_otgProcess->readAllStandardOutput();
        qDebug() << "[OTG] OTG stdout:" << data;
    });
    connect(m_otgProcess, &QProcess::readyReadStandardError, this, [this]() {
        QByteArray data = m_otgProcess->readAllStandardError();
        qDebug() << "[OTG] OTG stderr:" << data;
    });

    qDebug() << "[OTG] About to call m_otgProcess->start()";
    m_otgProcess->start(scrcpyPath, args);

    qDebug() << "[OTG] Waiting for OTG process to start...";
    if (!m_otgProcess->waitForStarted(5000)) {
        qDebug() << "[OTG] ERROR: Failed to start OTG process within 5s";
        QMessageBox::critical(this, tr("Error"), tr("Failed to start OTG scrcpy"));
        m_otgProcess->deleteLater();
        m_otgProcess = nullptr;
        return;
    }
    qDebug() << "[OTG] OTG process started, pid:" << m_otgProcess->processId();

    m_otgMode = true;
    qDebug() << "[OTG] OTG mode enabled for serial:" << m_serial;

    // Check scrcpy connection state AFTER starting OTG
    if (m_scrcpy) {
        enum insynic_scrcpy_state st = insynic_scrcpy_get_state(m_scrcpy);
        qDebug() << "[OTG] Scrcpy state AFTER OTG start:" << st;
        bool running = insynic_scrcpy_is_running(m_scrcpy);
        qDebug() << "[OTG] Scrcpy running AFTER OTG start:" << running;
    }
    qDebug() << "[OTG] ===== onOtgInputRequested completed =====";
}