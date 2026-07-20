#ifndef INSYNIC_DEVICE_CONTROL_H
#define INSYNIC_DEVICE_CONTROL_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QComboBox>

#include "insynic_scrcpy.h"
#include "insynic_virtualkey.h"
#include "insynic_profilemanager.h"

class InsynicDeviceControl : public QWidget
{
    Q_OBJECT

public:
    explicit InsynicDeviceControl(QWidget *parent = nullptr);
    ~InsynicDeviceControl();

    void setScrcpy(struct insynic_scrcpy *scrcpy);
    void setConnected(bool connected);

signals:
    void addKeyRequested();
    void profileSelected(const QString &name);
    void saveProfileRequested();
    void closeRequested();
    void otgInputRequested();

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
    void onRotateClicked();
    void onScreenToggleClicked();
    void onAddKeyClicked();
    void onSaveProfileClicked();
    void onApplyProfileClicked();
    void onDeleteProfileClicked();
    void onCloseClicked();
    void onOtgInputClicked();

private:
    void setupUi();
    QPushButton *createButton(const QString &text, const char *slot);

    struct insynic_scrcpy *m_scrcpy;

    QPushButton *m_backBtn;
    QPushButton *m_homeBtn;
    QPushButton *m_recentBtn;
    QPushButton *m_menuBtn;
    QPushButton *m_notifBtn;
    QPushButton *m_rotateBtn;
    QPushButton *m_screenToggleBtn;
    QPushButton *m_volUpBtn;
    QPushButton *m_volDownBtn;

    bool m_connected;

    QPushButton *m_addKeyBtn;
    QPushButton *m_saveProfileBtn;
    QComboBox *m_profileCombo;
    QPushButton *m_applyProfileBtn;
    QPushButton *m_deleteProfileBtn;
    QPushButton *m_closeBtn;
    QPushButton *m_otgBtn;

    InsynicProfileManager *m_profileManager;
};

#endif