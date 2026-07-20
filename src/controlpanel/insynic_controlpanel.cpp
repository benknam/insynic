#include "insynic_controlpanel.h"

#include <QStyle>
#include <QIcon>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>

InsynicControlPanel::InsynicControlPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

InsynicControlPanel::~InsynicControlPanel()
{
}

void
InsynicControlPanel::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);

    QLabel *titleLabel = new QLabel(tr("insynic"), this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    QGroupBox *devicesGroup = new QGroupBox(tr("Devices"), this);
    QVBoxLayout *devicesLayout = new QVBoxLayout(devicesGroup);
    devicesLayout->setContentsMargins(6, 6, 6, 6);
    devicesLayout->setSpacing(4);

    m_deviceList = new QListWidget(devicesGroup);
    m_deviceList->setMinimumHeight(100);
    m_deviceList->setMaximumHeight(180);
    m_deviceList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_deviceList->setStyleSheet(
        "QListWidget { background-color: #2a2a2a; color: #e0e0e0; "
        "border-radius: 4px; border: 1px solid #444; }"
        "QListWidget::item { padding: 4px; }"
        "QListWidget::item:selected { background-color: #4a4a4a; }"
    );
    connect(m_deviceList, &QListWidget::itemDoubleClicked,
            this, &InsynicControlPanel::onDeviceDoubleClicked);
    connect(m_deviceList, &QListWidget::currentItemChanged,
            this, &InsynicControlPanel::onDeviceSelectionChanged);
    connect(m_deviceList, &QListWidget::customContextMenuRequested,
            this, &InsynicControlPanel::onDeviceListContextMenu);
    devicesLayout->addWidget(m_deviceList);

    QHBoxLayout *btnRow1 = new QHBoxLayout();
    btnRow1->setSpacing(4);

    m_connectBtn = new QPushButton(tr("Connect"), devicesGroup);
    m_connectBtn->setMinimumHeight(32);
    m_connectBtn->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; "
        "border-radius: 4px; font-weight: bold; padding: 4px 8px; }"
        "QPushButton:hover { background-color: #45a049; }"
        "QPushButton:disabled { background-color: #333; color: #666; }"
    );
    connect(m_connectBtn, &QPushButton::clicked, this, &InsynicControlPanel::onConnectClicked);
    btnRow1->addWidget(m_connectBtn);

    m_refreshBtn = new QPushButton(tr("Refresh"), devicesGroup);
    m_refreshBtn->setMinimumHeight(32);
    m_refreshBtn->setStyleSheet(
        "QPushButton { background-color: #2196F3; color: white; "
        "border-radius: 4px; padding: 4px 8px; }"
        "QPushButton:hover { background-color: #1976D2; }"
    );
    connect(m_refreshBtn, &QPushButton::clicked, this, &InsynicControlPanel::onRefreshClicked);
    btnRow1->addWidget(m_refreshBtn);

    devicesLayout->addLayout(btnRow1);

    QHBoxLayout *btnRow2 = new QHBoxLayout();
    btnRow2->setSpacing(4);

    m_networkBtn = new QPushButton(tr("Network"), devicesGroup);
    m_networkBtn->setMinimumHeight(32);
    m_networkBtn->setStyleSheet(
        "QPushButton { background-color: #9C27B0; color: white; "
        "border-radius: 4px; padding: 4px 8px; }"
        "QPushButton:hover { background-color: #7B1FA2; }"
    );
    connect(m_networkBtn, &QPushButton::clicked, this, &InsynicControlPanel::onNetworkConnectClicked);
    btnRow2->addWidget(m_networkBtn);

    m_disconnectAllBtn = new QPushButton(tr("Disconnect All"), devicesGroup);
    m_disconnectAllBtn->setMinimumHeight(32);
    m_disconnectAllBtn->setEnabled(false);
    m_disconnectAllBtn->setStyleSheet(
        "QPushButton { background-color: #f44336; color: white; "
        "border-radius: 4px; padding: 4px 8px; }"
        "QPushButton:hover { background-color: #d32f2f; }"
        "QPushButton:disabled { background-color: #333; color: #666; }"
    );
    connect(m_disconnectAllBtn, &QPushButton::clicked, this, &InsynicControlPanel::onDisconnectAllClicked);
    btnRow2->addWidget(m_disconnectAllBtn);

    devicesLayout->addLayout(btnRow2);

    m_fileManagerBtn = new QPushButton(tr("File Manager"), devicesGroup);
    m_fileManagerBtn->setMinimumHeight(32);
    m_fileManagerBtn->setStyleSheet(
        "QPushButton { background-color: #FF9800; color: white; "
        "border-radius: 4px; padding: 4px 8px; }"
        "QPushButton:hover { background-color: #F57C00; }"
    );
    connect(m_fileManagerBtn, &QPushButton::clicked, this,
            &InsynicControlPanel::fileManagerRequested);
    devicesLayout->addWidget(m_fileManagerBtn);

    mainLayout->addWidget(devicesGroup);

    QGroupBox *connectedGroup = new QGroupBox(tr("Connected Devices"), this);
    QVBoxLayout *connectedLayout = new QVBoxLayout(connectedGroup);
    connectedLayout->setContentsMargins(6, 6, 6, 6);

    m_connectedList = new QListWidget(connectedGroup);
    m_connectedList->setMinimumHeight(60);
    m_connectedList->setMaximumHeight(180);
    m_connectedList->setWordWrap(true);
    m_connectedList->setTextElideMode(Qt::ElideNone);
    m_connectedList->setUniformItemSizes(false);
    m_connectedList->setStyleSheet(
        "QListWidget { background-color: #2a2a2a; color: #e0e0e0; "
        "border-radius: 4px; border: 1px solid #444; }"
        "QListWidget::item { padding: 3px; }"
        "QListWidget::item:selected { background-color: #4a4a4a; }"
    );
    connectedLayout->addWidget(m_connectedList);

    mainLayout->addWidget(connectedGroup);

    setFixedWidth(260);
    setStyleSheet(
        "QGroupBox { color: #e0e0e0; border: 1px solid #555; "
        "border-radius: 4px; margin-top: 6px; padding-top: 6px; }"
        "QGroupBox::title { left: 6px; padding: 0 3px; font-size: 12px; }"
        "QLabel { color: #e0e0e0; }"
        "QWidget { background-color: #1e1e1e; }"
    );
}

QString
InsynicControlPanel::currentDevice() const
{
    QListWidgetItem *item = m_deviceList->currentItem();
    if (item) {
        QString serial = item->data(Qt::UserRole).toString();
        if (!serial.isEmpty()) {
            return serial;
        }
        return item->text();
    }
    return QString();
}

void
InsynicControlPanel::updateConnectButtonState()
{
    QString serial = currentDevice();
    bool isConnected = m_connectedDevices.contains(serial);

    if (isConnected) {
        m_connectBtn->setText(tr("Disconnect"));
        m_connectBtn->setStyleSheet(
            "QPushButton { background-color: #f44336; color: white; "
            "border-radius: 4px; font-weight: bold; padding: 4px 8px; }"
            "QPushButton:hover { background-color: #d32f2f; }"
            "QPushButton:disabled { background-color: #333; color: #666; }"
        );
    } else {
        m_connectBtn->setText(tr("Connect"));
        m_connectBtn->setStyleSheet(
            "QPushButton { background-color: #4CAF50; color: white; "
            "border-radius: 4px; font-weight: bold; padding: 4px 8px; }"
            "QPushButton:hover { background-color: #45a049; }"
            "QPushButton:disabled { background-color: #333; color: #666; }"
        );
    }

    m_connectBtn->setEnabled(!serial.isEmpty());
}

void
InsynicControlPanel::onConnectClicked()
{
    QString serial = currentDevice();
    qDebug() << "[ControlPanel] onConnectClicked, current device:" << serial;

    if (serial.isEmpty() && m_deviceList->count() > 0) {
        QListWidgetItem *firstItem = m_deviceList->item(0);
        serial = firstItem->data(Qt::UserRole).toString();
        if (serial.isEmpty()) {
            serial = firstItem->text();
        }
        qDebug() << "[ControlPanel] No selection, using first device:" << serial;
    }
    if (!serial.isEmpty()) {
        if (m_connectedDevices.contains(serial)) {
            qDebug() << "[ControlPanel] Device already connected, emitting disconnectRequested";
            emit disconnectRequested(serial);
        } else {
            qDebug() << "[ControlPanel] Device not connected, emitting connectRequested";
            emit connectRequested(serial);
        }
    } else {
        qWarning() << "[ControlPanel] No device selected!";
    }
}

void
InsynicControlPanel::onDisconnectAllClicked()
{
    qDebug() << "[ControlPanel] onDisconnectAllClicked, emitting disconnectAllRequested";
    emit disconnectAllRequested();
}

void
InsynicControlPanel::onRefreshClicked()
{
    qDebug() << "[ControlPanel] onRefreshClicked, emitting refreshRequested";
    emit refreshRequested();
}

void
InsynicControlPanel::onNetworkConnectClicked()
{
    qDebug() << "[ControlPanel] onNetworkConnectClicked, emitting networkConnectOptionSelected";
    emit networkConnectOptionSelected();
}

void
InsynicControlPanel::onDeviceDoubleClicked(QListWidgetItem *item)
{
    if (item) {
        QString serial = item->data(Qt::UserRole).toString();
        if (serial.isEmpty()) {
            serial = item->text();
        }
        qDebug() << "[ControlPanel] onDeviceDoubleClicked, serial:" << serial;
        if (m_connectedDevices.contains(serial)) {
            qDebug() << "[ControlPanel] Device connected, emitting disconnectRequested";
            emit disconnectRequested(serial);
        } else {
            qDebug() << "[ControlPanel] Device not connected, emitting connectRequested";
            emit connectRequested(serial);
        }
    }
}

void
InsynicControlPanel::onDeviceSelectionChanged()
{
    QString serial = currentDevice();
    qDebug() << "[ControlPanel] onDeviceSelectionChanged, current:" << serial;
    updateConnectButtonState();
}

void
InsynicControlPanel::updateDeviceList(const QStringList &devices, const QMap<QString, QString> &deviceNames)
{
    QString current = currentDevice();
    m_deviceNames = deviceNames;
    m_deviceList->clear();
    for (const QString &dev : devices) {
        QString displayName;
        if (deviceNames.contains(dev) && !deviceNames[dev].isEmpty()) {
            displayName = deviceNames[dev] + "【" + dev + "】";
        } else {
            displayName = dev;
        }
        QListWidgetItem *item = new QListWidgetItem(displayName);
        item->setData(Qt::UserRole, dev);
        if (m_connectedDevices.contains(dev)) {
            item->setForeground(QColor("#4CAF50"));
        }
        m_deviceList->addItem(item);
    }
    if (!current.isEmpty()) {
        for (int i = 0; i < m_deviceList->count(); i++) {
            if (m_deviceList->item(i)->data(Qt::UserRole).toString() == current) {
                m_deviceList->setCurrentRow(i);
                break;
            }
        }
    } else if (m_deviceList->count() > 0) {
        m_deviceList->setCurrentRow(0);
    }
    updateConnectButtonState();
}

void
InsynicControlPanel::updateConnectionStatus(const QString &serial, bool connected)
{
    qDebug() << "[ControlPanel] updateConnectionStatus:" << serial << "- connected:" << connected;

    if (connected) {
        if (!m_connectedDevices.contains(serial)) {
            m_connectedDevices.append(serial);
        }
        QString deviceName = m_deviceNames.value(serial);
        QString displayName = deviceName.isEmpty() ? serial : deviceName + "【" + serial + "】";
        QString status = displayName + " - " + tr("Connected");
        bool found = false;
        for (int i = 0; i < m_connectedList->count(); i++) {
            QListWidgetItem *item = m_connectedList->item(i);
            if (item->data(Qt::UserRole).toString() == serial) {
                item->setText(status);
                item->setForeground(QColor("#4CAF50"));
                found = true;
                break;
            }
        }
        if (!found) {
            QListWidgetItem *item = new QListWidgetItem(status);
            item->setData(Qt::UserRole, serial);
            item->setForeground(QColor("#4CAF50"));
            m_connectedList->addItem(item);
        }
        qDebug() << "[ControlPanel] Added to connected list, total:" << m_connectedDevices.size();
    } else {
        m_connectedDevices.removeAll(serial);
        qDebug() << "[ControlPanel] Removed from connected list, remaining:" << m_connectedDevices.size();
        for (int i = 0; i < m_connectedList->count(); i++) {
            QListWidgetItem *item = m_connectedList->item(i);
            if (item->data(Qt::UserRole).toString() == serial) {
                delete m_connectedList->takeItem(i);
                break;
            }
        }
        for (int i = 0; i < m_deviceList->count(); i++) {
            QListWidgetItem *item = m_deviceList->item(i);
            if (item->data(Qt::UserRole).toString() == serial) {
                item->setForeground(QColor("#e0e0e0"));
                break;
            }
        }
    }

    m_disconnectAllBtn->setEnabled(m_connectedList->count() > 0);
    updateConnectButtonState();
}

void
InsynicControlPanel::updateConnectionMessage(const QString &serial, const QString &message)
{
    qDebug() << "[ControlPanel] updateConnectionMessage:" << serial << "-" << message;

    QString deviceName = m_deviceNames.value(serial);
    QString displayName = deviceName.isEmpty() ? serial : deviceName + "【" + serial + "】";
    QString text = displayName + " - " + message;

    QColor color;
    if (message == QLatin1String("Connected") || message == tr("Connected")) {
        color = QColor("#4CAF50");
    } else if (message.contains("failed", Qt::CaseInsensitive) ||
               message.contains(tr("failed"), Qt::CaseInsensitive)) {
        color = QColor("#f44336");
    } else {
        color = QColor("#FFA500");
    }

    for (int i = 0; i < m_connectedList->count(); i++) {
        QListWidgetItem *item = m_connectedList->item(i);
        if (item->data(Qt::UserRole).toString() == serial) {
            item->setText(text);
            item->setForeground(color);
            return;
        }
    }

    QListWidgetItem *item = new QListWidgetItem(text);
    item->setData(Qt::UserRole, serial);
    item->setForeground(color);
    m_connectedList->addItem(item);
    m_disconnectAllBtn->setEnabled(m_connectedList->count() > 0);
}

void
InsynicControlPanel::onDeviceListContextMenu(const QPoint &pos)
{
    QPoint globalPos = m_deviceList->mapToGlobal(pos);
    QListWidgetItem *item = m_deviceList->itemAt(pos);

    QMenu menu;
    if (item) {
        QString serial = item->data(Qt::UserRole).toString();
        if (serial.isEmpty()) {
            serial = item->text();
        }
        QAction *settingsAction = menu.addAction(tr("Streaming Settings..."));
        connect(settingsAction, &QAction::triggered, this, [this, serial]() {
            emit deviceSettingsRequested(serial);
        });
        menu.addSeparator();
    }

    QAction *connectAllAction = menu.addAction(tr("Connect All Devices"));
    connect(connectAllAction, &QAction::triggered, this, [this]() {
        emit connectAllRequested();
    });

    menu.exec(globalPos);
}

void
InsynicControlPanel::retranslateUi()
{
    m_connectBtn->setText(m_connectedDevices.contains(currentDevice()) ? tr("Disconnect") : tr("Connect"));
    m_refreshBtn->setText(tr("Refresh"));
    m_networkBtn->setText(tr("Network"));
    m_disconnectAllBtn->setText(tr("Disconnect All"));
    m_fileManagerBtn->setText(tr("File Manager"));

    QList<QGroupBox *> groupBoxes = findChildren<QGroupBox *>();
    for (QGroupBox *gb : groupBoxes) {
        QString originalText = gb->title();
        if (originalText.contains("Devices")) {
            gb->setTitle(tr("Devices"));
        } else if (originalText.contains("Connected")) {
            gb->setTitle(tr("Connected Devices"));
        }
    }
}
