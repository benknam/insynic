#include "insynic_sidepanel.h"

InsynicSidePanel::InsynicSidePanel(QWidget *parent)
    : QWidget(parent)
    , m_scrcpy(nullptr)
    , m_connected(false)
{
    setupUi();
}

InsynicSidePanel::~InsynicSidePanel()
{
}

void
InsynicSidePanel::setupUi()
{
    setFixedWidth(48);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(4);
    mainLayout->setContentsMargins(4, 4, 4, 4);

    QString iconStyle =
        "QPushButton { background-color: #4a4a4a; color: white; "
        "border-radius: 6px; font-size: 18px; min-width: 36px; min-height: 36px; }"
        "QPushButton:hover { background-color: #5a5a5a; }"
        "QPushButton:disabled { background-color: #2a2a2a; color: #666; }";

    QString orangeStyle =
        "QPushButton { background-color: #FF9800; color: white; "
        "border-radius: 6px; font-size: 18px; min-width: 36px; min-height: 36px; }"
        "QPushButton:hover { background-color: #F57C00; }"
        "QPushButton:disabled { background-color: #2a2a2a; color: #666; }";

    QString redStyle =
        "QPushButton { background-color: #f44336; color: white; "
        "border-radius: 6px; font-size: 18px; min-width: 36px; min-height: 36px; }"
        "QPushButton:hover { background-color: #d32f2f; }";

    m_volUpBtn = createIconButton("▲", "Volume Up");
    m_volUpBtn->setStyleSheet(iconStyle);
    connect(m_volUpBtn, &QPushButton::clicked, this, &InsynicSidePanel::onVolumeUpClicked);
    mainLayout->addWidget(m_volUpBtn);

    m_volDownBtn = createIconButton("▼", "Volume Down");
    m_volDownBtn->setStyleSheet(iconStyle);
    connect(m_volDownBtn, &QPushButton::clicked, this, &InsynicSidePanel::onVolumeDownClicked);
    mainLayout->addWidget(m_volDownBtn);

    m_notifBtn = createIconButton("○", "Notifications");
    m_notifBtn->setStyleSheet(iconStyle);
    connect(m_notifBtn, &QPushButton::clicked, this, &InsynicSidePanel::onNotificationClicked);
    mainLayout->addWidget(m_notifBtn);

    m_rotateBtn = createIconButton("↻", "Rotate");
    m_rotateBtn->setStyleSheet(iconStyle);
    connect(m_rotateBtn, &QPushButton::clicked, this, &InsynicSidePanel::onRotateClicked);
    mainLayout->addWidget(m_rotateBtn);

    m_screenToggleBtn = createIconButton("□", "Screen On/Off");
    m_screenToggleBtn->setStyleSheet(iconStyle);
    connect(m_screenToggleBtn, &QPushButton::clicked, this, &InsynicSidePanel::onScreenToggleClicked);
    mainLayout->addWidget(m_screenToggleBtn);

    mainLayout->addStretch();

    m_otgBtn = createIconButton("🔌", "OTG Input");
    m_otgBtn->setEnabled(false);
    m_otgBtn->setStyleSheet(orangeStyle);
    connect(m_otgBtn, &QPushButton::clicked, this, &InsynicSidePanel::onOtgInputRequested);
    mainLayout->addWidget(m_otgBtn);

    mainLayout->addSpacing(8);

    m_disconnectBtn = createIconButton("✕", "Disconnect");
    m_disconnectBtn->setEnabled(false);
    m_disconnectBtn->setStyleSheet(redStyle);
    connect(m_disconnectBtn, &QPushButton::clicked, this, &InsynicSidePanel::onDisconnectClicked);
    mainLayout->addWidget(m_disconnectBtn);

    setStyleSheet("QWidget { background-color: #333333; border: 1px solid #555555; }");
}

QPushButton *
InsynicSidePanel::createIconButton(const QString &icon, const char *tooltip)
{
    QPushButton *btn = new QPushButton(icon);
    btn->setToolTip(tr(tooltip));
    return btn;
}

void
InsynicSidePanel::setScrcpy(struct insynic_scrcpy *scrcpy)
{
    m_scrcpy = scrcpy;
}

void
InsynicSidePanel::setConnected(bool connected, bool isNetworkConnection)
{
    m_connected = connected;
    m_isNetworkConnection = isNetworkConnection;

    if (isNetworkConnection) {
        m_otgBtn->hide();
    } else {
        m_otgBtn->show();
        m_otgBtn->setEnabled(connected);
    }

    m_disconnectBtn->setEnabled(connected);
}

void
InsynicSidePanel::onVolumeUpClicked()
{
    if (m_scrcpy) {
        insynic_scrcpy_inject_volume_up(m_scrcpy);
    }
}

void
InsynicSidePanel::onVolumeDownClicked()
{
    if (m_scrcpy) {
        insynic_scrcpy_inject_volume_down(m_scrcpy);
    }
}

void
InsynicSidePanel::onNotificationClicked()
{
    if (m_scrcpy) {
        insynic_scrcpy_expand_notification_panel(m_scrcpy);
    }
}

void
InsynicSidePanel::onRotateClicked()
{
    if (m_scrcpy) {
        insynic_scrcpy_rotate_device(m_scrcpy);
    }
}

void
InsynicSidePanel::onScreenToggleClicked()
{
    if (m_scrcpy) {
        insynic_scrcpy_toggle_display(m_scrcpy);
    }
}

void
InsynicSidePanel::onOtgInputRequested()
{
    emit otgInputRequested();
}

void
InsynicSidePanel::onDisconnectClicked()
{
    qDebug() << "[SidePanel] onDisconnectClicked called, emitting disconnectRequested";
    emit disconnectRequested();
}

void
InsynicSidePanel::retranslateUi()
{
    m_volUpBtn->setToolTip(tr("Volume Up"));
    m_volDownBtn->setToolTip(tr("Volume Down"));
    m_notifBtn->setToolTip(tr("Notifications"));
    m_rotateBtn->setToolTip(tr("Rotate"));
    m_screenToggleBtn->setToolTip(tr("Screen On/Off"));
    m_otgBtn->setToolTip(tr("OTG Input"));
    m_disconnectBtn->setToolTip(tr("Close Connection"));
}
