#ifndef INSYNIC_DEVICE_WINDOW_H
#define INSYNIC_DEVICE_WINDOW_H

#include <QWidget>
#include <QTimer>
#include <QMap>
#include <QProcess>
#include <SDL3/SDL.h>
#include "insynic_scrcpy.h"
#include "insynic_controlbar.h"
#include "insynic_sidepanel.h"
#include "insynic_virtualkey.h"
#include "insynic_draggablekey.h"

class InsynicDeviceWindow : public QWidget
{
    Q_OBJECT

public:
    explicit InsynicDeviceWindow(const QString &serial, const QString &adbPath,
                                  const QString &serverPath, int maxSize, int maxFps,
                                  int videoBitRate,
                                  bool turnScreenOff = false,
                                  bool stayAwake = false,
                                  bool powerOn = false,
                                  bool disableScreensaver = false,
                                  bool controlEnabled = true,
                                  bool audioEnabled = false,
                                  int audioBitRate = 128,
                                  int audioCodec = 0,
                                  int audioSource = 0,
                                  const QString &recordFilePath = QString(),
                                  int recordFormat = 0,
                                  bool recordVideo = true,
                                  bool recordAudio = false,
                                  QWidget *parent = nullptr);
    ~InsynicDeviceWindow();

    QString serial() const { return m_serial; }
    bool isConnected() const { return m_connected; }
    struct insynic_scrcpy *scrcpy() const { return m_scrcpy; }

    void addVirtualKey(const VirtualKey &key);
    void removeAllVirtualKeys();
    void applyProfile(const QString &profileName);
    void processSdlEvent(const SDL_Event *event);
    SDL_Window* sdlWindow() const;
    void retranslateUi();

signals:
    void disconnected(const QString &serial);
    void connectionMessage(const QString &serial, const QString &message);

private slots:
    void checkState();
    void handleStateChange(enum insynic_scrcpy_state state);
    void onAddKeyRequested();
    void onProfileSelected(const QString &name);
    void onSaveProfileRequested();
    void onCloseRequested();
    void updateKeyContainerPosition();
    void onOtgInputRequested();
    void processSdlEvents();
    void checkCloseProgress();

private:
    static void stateCallback(enum insynic_scrcpy_state state, void *userdata);
    void finishClose();

    QString m_serial;
    QString m_adbPath;
    QString m_serverPath;
    int m_maxSize;
    int m_maxFps;
    int m_videoBitRate;
    bool m_turnScreenOff;
    bool m_stayAwake;
    bool m_powerOn;
    bool m_disableScreensaver;
    bool m_controlEnabled;
    bool m_audioEnabled;
    int m_audioBitRate;
    int m_audioCodec;
    int m_audioSource;
    QString m_recordFilePath;
    int m_recordFormat;
    bool m_recordVideo;
    bool m_recordAudio;
    QByteArray m_recordFileData;
    
    struct insynic_scrcpy *m_scrcpy;
    bool m_connected;
    volatile bool m_isClosing;
    int m_closePollCount;
    QTimer *m_stateTimer;
    QTimer *m_keyContainerTimer;
    QTimer *m_sdlEventTimer;
    volatile bool m_stateChangePending;
    volatile int m_pendingState;
    
    InsynicControlBar *m_controlBar;
    InsynicSidePanel *m_sidePanel;
    
    QList<InsynicDraggableKey*> m_virtualKeys;
    QWidget *m_keyContainer;
    bool m_isDraggingKey;
    QMap<int, bool> m_toggleStates;
    
    int m_scrcpyWindowX;
    int m_scrcpyWindowY;
    int m_scrcpyWindowWidth;
    int m_scrcpyWindowHeight;
    
    QProcess *m_otgProcess;
    bool m_otgMode;
    QProcess *m_scrcpyProcess;
    bool m_isNetworkConnection;

protected:
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
};

#endif