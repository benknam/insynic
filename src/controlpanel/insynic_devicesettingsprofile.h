#ifndef INSYNIC_DEVICE_SETTINGS_PROFILE_H
#define INSYNIC_DEVICE_SETTINGS_PROFILE_H

#include <QString>
#include <QSettings>
#include <QMap>

struct DeviceStreamingSettings {
    int maxSize = 1024;
    int maxFps = 60;
    int videoBitRate = 8;
    bool turnScreenOff = false;
    bool stayAwake = false;
    bool powerOn = false;
    bool disableScreensaver = false;
    bool controlEnabled = true;
};

class InsynicDeviceSettingsProfile
{
public:
    static InsynicDeviceSettingsProfile &instance();

    DeviceStreamingSettings loadSettings(const QString &serial) const;
    void saveSettings(const QString &serial, const DeviceStreamingSettings &settings);

private:
    InsynicDeviceSettingsProfile() = default;
    QString key(const QString &serial, const QString &field) const;
};

#endif
