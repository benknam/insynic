#ifndef INSYNIC_CONTROL_PANEL_H
#define INSYNIC_CONTROL_PANEL_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMap>
#include <QMenu>

#include "insynic_scrcpy.h"

class InsynicControlPanel : public QWidget
{
    Q_OBJECT

public:
    explicit InsynicControlPanel(QWidget *parent = nullptr);
    ~InsynicControlPanel();

    QString currentDevice() const;
    void retranslateUi();

signals:
    void connectRequested(const QString &serial);
    void disconnectRequested(const QString &serial);
    void disconnectAllRequested();
    void connectAllRequested();
    void fileManagerRequested();
    void networkConnectOptionSelected();
    void refreshRequested();
    void deviceSettingsRequested(const QString &serial);
    void recordSettingsRequested(const QString &serial);
    void disconnectNetworkDeviceRequested(const QString &serial);

public slots:
    void updateDeviceList(const QStringList &devices, const QMap<QString, QString> &deviceNames = QMap<QString, QString>());
    void updateConnectionStatus(const QString &serial, bool connected);
    void updateConnectionMessage(const QString &serial, const QString &message);

private slots:
    void onConnectClicked();
    void onDisconnectAllClicked();
    void onRefreshClicked();
    void onNetworkConnectClicked();
    void onDeviceDoubleClicked(QListWidgetItem *item);
    void onDeviceSelectionChanged();
    void onDeviceListContextMenu(const QPoint &pos);

private:
    void setupUi();
    void updateConnectButtonState();

    QListWidget *m_deviceList;
    QPushButton *m_connectBtn;
    QPushButton *m_disconnectAllBtn;
    QPushButton *m_refreshBtn;
    QPushButton *m_networkBtn;
    QPushButton *m_fileManagerBtn;
    QListWidget *m_connectedList;
    QStringList m_connectedDevices;
    QMap<QString, QString> m_deviceNames;
};

#endif
