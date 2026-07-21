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
    bool audioEnabled = false;
    int audioBitRate = 128;      // kbps
    int audioCodec = 0;          // 0=OPUS, 1=AAC
    int audioSource = 0;         // 0=OUTPUT, 1=MIC, 2=PLAYBACK
    // Recording
    bool recordEnabled = false;       // enable recording on connect
    QString recordFilePath;            // output file path
    int recordFormat = 0;             // 0=auto,1=mp4,2=mkv,3=m4a,4=mka,5=opus,6=aac,7=flac,8=wav
    bool recordVideo = true;           // record video stream
    bool recordAudio = false;          // record audio stream
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
