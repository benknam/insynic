#ifndef INSYNIC_DEVICE_SESSION_H
#define INSYNIC_DEVICE_SESSION_H

#include <QObject>
#include <QString>
#include "insynic_scrcpy.h"

class InsynicDeviceSession : public QObject
{
    Q_OBJECT

public:
    explicit InsynicDeviceSession(QObject *parent = nullptr);
    ~InsynicDeviceSession();

    bool start(const QString &serial, const QString &adbPath, const QString &serverPath,
               int maxSize, int maxFps, int videoBitRate);
    void stop();
    
    QString serial() const { return m_serial; }
    bool isConnected() const { return m_connected; }
    struct insynic_scrcpy *scrcpy() const { return m_scrcpy; }

signals:
    void connected();
    void disconnected();
    void error(const QString &message);

private:
    static void stateCallback(enum insynic_scrcpy_state state, void *userdata);
    void handleStateChange(enum insynic_scrcpy_state state);

    QString m_serial;
    struct insynic_scrcpy *m_scrcpy;
    bool m_connected;
};

#endif