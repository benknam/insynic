#ifndef INSYNIC_CONTROL_PANEL_H
#define INSYNIC_CONTROL_PANEL_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QMap>

#include "insynic_scrcpy.h"
#include "insynic_virtualkey.h"
#include "insynic_profilemanager.h"

class InsynicControlPanel : public QWidget
{
    Q_OBJECT

public:
    explicit InsynicControlPanel(QWidget *parent = nullptr);
    ~InsynicControlPanel();

    void setScrcpy(struct insynic_scrcpy *scrcpy);
    void setConnected(bool connected);
    bool isConnected() const { return m_connected; }

signals:
    void connectRequested();
    void disconnectRequested();
    void fileManagerRequested();
    void deviceSelected(const QString &serial);
    void otgInputRequested();
    void networkConnectOptionSelected();

public slots:
    void updateDeviceList(const QStringList &devices);
    void setSerial(const QString &serial);
    void setOtgMode(bool enabled);
    void setNetworkConnected(bool connected);

public slots:
    void updateProfileCombo();

private slots:
    void onBackClicked();
    void onHomeClicked();
    void onRecentClicked();
    void onMenuClicked();
    void onVolumeUpClicked();
    void onVolumeDownClicked();
    void onNotificationClicked();
    void onSettingsPanelClicked();
    void onRotateClicked();
    void onScreenToggleClicked();
    void onOtgInputClicked();
    void onAddKeyClicked();
    void onSaveProfileClicked();
    void onApplyProfileClicked();
    void onDeleteProfileClicked();

private:
    void setupUi();
    QPushButton *createButton(const QString &text, const char *slot);

    struct insynic_scrcpy *m_scrcpy;
    QString m_serial;

    QComboBox *m_deviceCombo;
    QPushButton *m_connectBtn;
    QPushButton *m_fileManagerBtn;

    QPushButton *m_backBtn;
    QPushButton *m_homeBtn;
    QPushButton *m_recentBtn;
    QPushButton *m_menuBtn;
    QPushButton *m_notifBtn;
    QPushButton *m_settingsBtn;
    QPushButton *m_rotateBtn;
    QPushButton *m_screenToggleBtn;
    QPushButton *m_volUpBtn;
    QPushButton *m_volDownBtn;
    QPushButton *m_otgBtn;

    bool m_connected;
    bool m_networkConnected;
    
    QPushButton *m_addKeyBtn;
    QPushButton *m_saveProfileBtn;
    QComboBox *m_profileCombo;
    QPushButton *m_applyProfileBtn;
    QPushButton *m_deleteProfileBtn;
    
    InsynicProfileManager *m_profileManager;
    
signals:
    void addKeyRequested();
    void profileSelected(const QString &name);
    void saveProfileRequested();
};

#endif
