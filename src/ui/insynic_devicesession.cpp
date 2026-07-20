#include "insynic_devicesession.h"
#include <QMessageBox>
#include <QtConcurrent>
#include <QFutureWatcher>

InsynicDeviceSession::InsynicDeviceSession(QObject *parent)
    : QObject(parent)
    , m_scrcpy(nullptr)
    , m_connected(false)
{
}

InsynicDeviceSession::~InsynicDeviceSession()
{
    stop();
}

bool
InsynicDeviceSession::start(const QString &serial, const QString &adbPath, const QString &serverPath,
                            int maxSize, int maxFps, int videoBitRate)
{
    if (m_scrcpy) {
        return false;
    }

    m_serial = serial;

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
    config.audio_enabled = false;
    config.control_enabled = true;
    config.turn_screen_off = false;

    config.window_width = 0;
    config.window_height = 0;
    config.window_x = 0;
    config.window_y = 0;

    m_scrcpy = insynic_scrcpy_create(&config);
    if (!m_scrcpy) {
        return false;
    }

    insynic_scrcpy_set_state_callback(m_scrcpy, stateCallback, this);

    if (!insynic_scrcpy_start(m_scrcpy)) {
        insynic_scrcpy_destroy(m_scrcpy);
        m_scrcpy = nullptr;
        return false;
    }

    return true;
}

void
InsynicDeviceSession::stop()
{
    if (!m_scrcpy) {
        return;
    }

    struct insynic_scrcpy *scrcpy = m_scrcpy;
    m_scrcpy = nullptr;

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
InsynicDeviceSession::stateCallback(enum insynic_scrcpy_state state, void *userdata)
{
    InsynicDeviceSession *self = static_cast<InsynicDeviceSession*>(userdata);
    QMetaObject::invokeMethod(self, "handleStateChange",
                              Qt::QueuedConnection,
                              Q_ARG(enum insynic_scrcpy_state, state));
}

void
InsynicDeviceSession::handleStateChange(enum insynic_scrcpy_state state)
{
    switch (state) {
    case INSYNIC_SCRCPY_STATE_CONNECTED:
        m_connected = true;
        emit connected();
        break;
    case INSYNIC_SCRCPY_STATE_DISCONNECTED:
        m_connected = false;
        emit disconnected();
        break;
    case INSYNIC_SCRCPY_STATE_ERROR:
        m_connected = false;
        emit error("Connection error");
        break;
    case INSYNIC_SCRCPY_STATE_CONNECTING:
    case INSYNIC_SCRCPY_STATE_IDLE:
    default:
        break;
    }
}