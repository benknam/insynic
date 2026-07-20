#ifndef INSYNIC_SIDE_PANEL_H
#define INSYNIC_SIDE_PANEL_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>

#include "insynic_scrcpy.h"

class InsynicSidePanel : public QWidget
{
    Q_OBJECT

public:
    explicit InsynicSidePanel(QWidget *parent = nullptr);
    ~InsynicSidePanel();

    void setScrcpy(struct insynic_scrcpy *scrcpy);
    void setConnected(bool connected, bool isNetworkConnection = false);
    void retranslateUi();

signals:
    void otgInputRequested();
    void disconnectRequested();

private slots:
    void onVolumeUpClicked();
    void onVolumeDownClicked();
    void onNotificationClicked();
    void onRotateClicked();
    void onScreenToggleClicked();
    void onOtgInputRequested();
    void onDisconnectClicked();

private:
    void setupUi();
    QPushButton *createIconButton(const QString &icon, const char *tooltip);

    struct insynic_scrcpy *m_scrcpy;
    bool m_connected;
    bool m_isNetworkConnection;

    QPushButton *m_volUpBtn;
    QPushButton *m_volDownBtn;
    QPushButton *m_notifBtn;
    QPushButton *m_rotateBtn;
    QPushButton *m_screenToggleBtn;
    QPushButton *m_otgBtn;
    QPushButton *m_disconnectBtn;
};

#endif
