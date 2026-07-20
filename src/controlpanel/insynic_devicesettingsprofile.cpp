#include "insynic_devicesettingsprofile.h"

InsynicDeviceSettingsProfile &InsynicDeviceSettingsProfile::instance()
{
    static InsynicDeviceSettingsProfile instance;
    return instance;
}

QString InsynicDeviceSettingsProfile::key(const QString &serial, const QString &field) const
{
    QString safeSerial = serial;
    safeSerial.replace('.', '_').replace(':', '_').replace('/', '_');
    return QString("deviceSettings/%1/%2").arg(safeSerial, field);
}

DeviceStreamingSettings InsynicDeviceSettingsProfile::loadSettings(const QString &serial) const
{
    DeviceStreamingSettings s;
    QSettings settings;
    settings.beginGroup("deviceSettings");
    QString safeSerial = serial;
    safeSerial.replace('.', '_').replace(':', '_').replace('/', '_');
    settings.beginGroup(safeSerial);
    s.maxSize = settings.value("maxSize", s.maxSize).toInt();
    s.maxFps = settings.value("maxFps", s.maxFps).toInt();
    s.videoBitRate = settings.value("videoBitRate", s.videoBitRate).toInt();
    s.turnScreenOff = settings.value("turnScreenOff", s.turnScreenOff).toBool();
    s.stayAwake = settings.value("stayAwake", s.stayAwake).toBool();
    s.powerOn = settings.value("powerOn", s.powerOn).toBool();
    s.disableScreensaver = settings.value("disableScreensaver", s.disableScreensaver).toBool();
    s.controlEnabled = settings.value("controlEnabled", s.controlEnabled).toBool();
    settings.endGroup();
    settings.endGroup();
    return s;
}

void InsynicDeviceSettingsProfile::saveSettings(const QString &serial, const DeviceStreamingSettings &s)
{
    QSettings settings;
    settings.beginGroup("deviceSettings");
    QString safeSerial = serial;
    safeSerial.replace('.', '_').replace(':', '_').replace('/', '_');
    settings.beginGroup(safeSerial);
    settings.setValue("maxSize", s.maxSize);
    settings.setValue("maxFps", s.maxFps);
    settings.setValue("videoBitRate", s.videoBitRate);
    settings.setValue("turnScreenOff", s.turnScreenOff);
    settings.setValue("stayAwake", s.stayAwake);
    settings.setValue("powerOn", s.powerOn);
    settings.setValue("disableScreensaver", s.disableScreensaver);
    settings.setValue("controlEnabled", s.controlEnabled);
    settings.endGroup();
    settings.endGroup();
}
