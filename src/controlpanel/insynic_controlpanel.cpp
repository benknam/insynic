#include "insynic_controlpanel.h"

#include <QStyle>
#include <QIcon>
#include <QProcess>
#include <QCoreApplication>
#include <QDir>
#include "insynic_saveprofiledialog.h"
#include <QMessageBox>

InsynicControlPanel::InsynicControlPanel(QWidget *parent)
    : QWidget(parent)
    , m_scrcpy(nullptr)
    , m_connected(false)
    , m_networkConnected(false)
    , m_profileManager(new InsynicProfileManager(this))
{
    setupUi();
}

InsynicControlPanel::~InsynicControlPanel()
{
}

void
InsynicControlPanel::setScrcpy(struct insynic_scrcpy *scrcpy)
{
    m_scrcpy = scrcpy;
}

void
InsynicControlPanel::setConnected(bool connected)
{
    m_connected = connected;
    m_connectBtn->setText(connected ? tr("Disconnect") : tr("Connect"));

    m_backBtn->setEnabled(connected);
    m_homeBtn->setEnabled(connected);
    m_recentBtn->setEnabled(connected);
    m_menuBtn->setEnabled(connected);
    m_notifBtn->setEnabled(connected);
    m_settingsBtn->setEnabled(connected);
    m_rotateBtn->setEnabled(connected);
    m_screenToggleBtn->setEnabled(connected);
    m_volUpBtn->setEnabled(connected);
    m_volDownBtn->setEnabled(connected);
    m_otgBtn->setEnabled(connected && !m_networkConnected);
    m_addKeyBtn->setEnabled(connected);
    m_saveProfileBtn->setEnabled(connected);
    m_applyProfileBtn->setEnabled(connected);
    m_deleteProfileBtn->setEnabled(connected);
}

void
InsynicControlPanel::updateDeviceList(const QStringList &devices)
{
    QString current = m_deviceCombo->currentData().toString();
    m_deviceCombo->clear();
    m_deviceCombo->addItem(tr("Auto-select"), "");
    m_deviceCombo->addItem(tr("Network Connection"), "__network__");
    for (const QString &dev : devices) {
        m_deviceCombo->addItem(dev, dev);
    }
    if (current != "__network__") {
        int idx = m_deviceCombo->findData(current);
        if (idx >= 0) {
            m_deviceCombo->setCurrentIndex(idx);
        }
    }
}

void
InsynicControlPanel::setSerial(const QString &serial)
{
    m_serial = serial;
}

QPushButton *
InsynicControlPanel::createButton(const QString &text, const char *slot)
{
    QPushButton *btn = new QPushButton(text, this);
    btn->setMinimumHeight(36);
    connect(btn, SIGNAL(clicked()), this, slot);
    return btn;
}

void
InsynicControlPanel::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(10);

    QLabel *titleLabel = new QLabel(tr("insynic"), this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    QGroupBox *connGroup = new QGroupBox(tr("Connection"), this);
    QVBoxLayout *connLayout = new QVBoxLayout(connGroup);
    connLayout->setContentsMargins(0, 0, 0, 0);
    connLayout->setSpacing(6);

    m_deviceCombo = new QComboBox(connGroup);
    m_deviceCombo->addItem(tr("Auto-select"), "");
    m_deviceCombo->addItem(tr("Network Connection"), "__network__");
    connect(m_deviceCombo,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx) {
        QString data = m_deviceCombo->itemData(idx).toString();
        if (data == "__network__") {
            emit networkConnectOptionSelected();
        } else {
            emit deviceSelected(data);
        }
    });
    connLayout->addWidget(m_deviceCombo);

    m_connectBtn = new QPushButton(tr("Connect"), connGroup);
    m_connectBtn->setMinimumHeight(40);
    m_connectBtn->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; "
        "border-radius: 6px; font-weight: bold; }"
        "QPushButton:hover { background-color: #45a049; }"
        "QPushButton:disabled { background-color: #cccccc; }"
    );
    connect(m_connectBtn, &QPushButton::clicked, this, [this]() {
        if (m_connected) {
            emit disconnectRequested();
        } else {
            emit connectRequested();
        }
    });
    connLayout->addWidget(m_connectBtn);

    m_fileManagerBtn = new QPushButton(tr("File Manager"), connGroup);
    m_fileManagerBtn->setMinimumHeight(36);
    connect(m_fileManagerBtn, &QPushButton::clicked, this,
            &InsynicControlPanel::fileManagerRequested);
    connLayout->addWidget(m_fileManagerBtn);

    mainLayout->addWidget(connGroup);

    QGroupBox *keysGroup = new QGroupBox(tr("Android Keys"), this);
    QHBoxLayout *keysLayout = new QHBoxLayout(keysGroup);
    keysLayout->setSpacing(6);

    m_backBtn = createButton(tr("Back"), SLOT(onBackClicked()));
    m_homeBtn = createButton(tr("Home"), SLOT(onHomeClicked()));
    m_recentBtn = createButton(tr("Task"), SLOT(onRecentClicked()));
    m_menuBtn = createButton(tr("Menu"), SLOT(onMenuClicked()));

    keysLayout->addWidget(m_backBtn);
    keysLayout->addWidget(m_homeBtn);
    keysLayout->addWidget(m_recentBtn);
    keysLayout->addWidget(m_menuBtn);

    mainLayout->addWidget(keysGroup);

    QGroupBox *systemGroup = new QGroupBox(tr("System"), this);
    QGridLayout *systemLayout = new QGridLayout(systemGroup);
    systemLayout->setSpacing(6);

    m_notifBtn = createButton(tr("Notifications"), SLOT(onNotificationClicked()));
    m_settingsBtn = createButton(tr("Quick Settings"),
                                 SLOT(onSettingsPanelClicked()));
    m_rotateBtn = createButton(tr("Rotate"), SLOT(onRotateClicked()));
    m_screenToggleBtn = createButton(tr("Screen On/Off"), SLOT(onScreenToggleClicked()));
    m_volUpBtn = createButton(tr("Volume Up"), SLOT(onVolumeUpClicked()));
    m_volDownBtn = createButton(tr("Volume Down"), SLOT(onVolumeDownClicked()));

    systemLayout->addWidget(m_notifBtn, 0, 0);
    systemLayout->addWidget(m_settingsBtn, 0, 1);
    systemLayout->addWidget(m_rotateBtn, 1, 0);
    systemLayout->addWidget(m_screenToggleBtn, 1, 1);
    systemLayout->addWidget(m_volUpBtn, 2, 0);
    systemLayout->addWidget(m_volDownBtn, 2, 1);

    mainLayout->addWidget(systemGroup);

    QGroupBox *utilitiesGroup = new QGroupBox(tr("Utilities"), this);
    QVBoxLayout *utilitiesLayout = new QVBoxLayout(utilitiesGroup);
    utilitiesLayout->setSpacing(6);

    m_addKeyBtn = new QPushButton(tr("Add Key"), utilitiesGroup);
    m_addKeyBtn->setMinimumHeight(36);
    connect(m_addKeyBtn, &QPushButton::clicked, this, &InsynicControlPanel::onAddKeyClicked);
    utilitiesLayout->addWidget(m_addKeyBtn);

    m_saveProfileBtn = new QPushButton(tr("Save Profile"), utilitiesGroup);
    m_saveProfileBtn->setMinimumHeight(36);
    connect(m_saveProfileBtn, &QPushButton::clicked, this, &InsynicControlPanel::onSaveProfileClicked);
    utilitiesLayout->addWidget(m_saveProfileBtn);

    m_profileCombo = new QComboBox(utilitiesGroup);
    updateProfileCombo();
    utilitiesLayout->addWidget(m_profileCombo);

    QHBoxLayout *profileBtnLayout = new QHBoxLayout();
    m_applyProfileBtn = new QPushButton(tr("Apply"), utilitiesGroup);
    m_applyProfileBtn->setMinimumHeight(30);
    m_deleteProfileBtn = new QPushButton(tr("Delete"), utilitiesGroup);
    m_deleteProfileBtn->setMinimumHeight(30);
    
    profileBtnLayout->addWidget(m_applyProfileBtn);
    profileBtnLayout->addWidget(m_deleteProfileBtn);
    utilitiesLayout->addLayout(profileBtnLayout);

    connect(m_applyProfileBtn, &QPushButton::clicked, this, &InsynicControlPanel::onApplyProfileClicked);
    connect(m_deleteProfileBtn, &QPushButton::clicked, this, &InsynicControlPanel::onDeleteProfileClicked);

    mainLayout->addWidget(utilitiesGroup);

    m_otgBtn = new QPushButton(tr("OTG Input"), this);
    m_otgBtn->setMinimumHeight(40);
    m_otgBtn->setStyleSheet(
        "QPushButton { background-color: #2196F3; color: white; "
        "border-radius: 6px; font-weight: bold; }"
        "QPushButton:hover { background-color: #1976D2; }"
        "QPushButton:disabled { background-color: #cccccc; }"
    );
    connect(m_otgBtn, &QPushButton::clicked, this,
            &InsynicControlPanel::onOtgInputClicked);
    mainLayout->addWidget(m_otgBtn);

    mainLayout->addStretch();

    setConnected(false);
    setFixedWidth(260);
}

void
InsynicControlPanel::onBackClicked()
{
    if (m_scrcpy) {
        insynic_scrcpy_inject_back(m_scrcpy);
    }
}

void
InsynicControlPanel::onHomeClicked()
{
    if (m_scrcpy) {
        insynic_scrcpy_inject_home(m_scrcpy);
    }
}

void
InsynicControlPanel::onRecentClicked()
{
    if (m_scrcpy) {
        insynic_scrcpy_inject_recent(m_scrcpy);
    }
}



void
InsynicControlPanel::onVolumeUpClicked()
{
    if (m_scrcpy) {
        insynic_scrcpy_inject_volume_up(m_scrcpy);
    }
}

void
InsynicControlPanel::onVolumeDownClicked()
{
    if (m_scrcpy) {
        insynic_scrcpy_inject_volume_down(m_scrcpy);
    }
}

void
InsynicControlPanel::onNotificationClicked()
{
    if (m_scrcpy) {
        insynic_scrcpy_expand_notification_panel(m_scrcpy);
    }
}

void
InsynicControlPanel::onSettingsPanelClicked()
{
    if (m_scrcpy) {
        insynic_scrcpy_expand_settings_panel(m_scrcpy);
    }
}

void
InsynicControlPanel::onRotateClicked()
{
    if (m_scrcpy) {
        insynic_scrcpy_rotate_device(m_scrcpy);
    }
}

void
InsynicControlPanel::onScreenToggleClicked()
{
    if (m_scrcpy) {
        insynic_scrcpy_toggle_display(m_scrcpy);
    }
}

void
InsynicControlPanel::onOtgInputClicked()
{
    emit otgInputRequested();
}

void
InsynicControlPanel::setOtgMode(bool enabled)
{
    if (enabled) {
        m_otgBtn->setText(tr("OTG Input ON\n(Cmd+Q to exit)"));
        m_otgBtn->setStyleSheet(
            "QPushButton { background-color: #f44336; color: white; "
            "border-radius: 6px; font-weight: bold; }"
            "QPushButton:hover { background-color: #d32f2f; }"
            "QPushButton:disabled { background-color: #cccccc; }"
        );
    } else {
        m_otgBtn->setText(tr("OTG Input"));
        m_otgBtn->setStyleSheet(
            "QPushButton { background-color: #2196F3; color: white; "
            "border-radius: 6px; font-weight: bold; }"
            "QPushButton:hover { background-color: #1976D2; }"
            "QPushButton:disabled { background-color: #cccccc; }"
        );
    }
}

void
InsynicControlPanel::setNetworkConnected(bool connected)
{
    m_networkConnected = connected;
    if (m_connected) {
        m_otgBtn->setEnabled(!connected);
    }
}

void
InsynicControlPanel::onMenuClicked()
{
    if (m_scrcpy) {
        insynic_scrcpy_inject_menu(m_scrcpy);
    }
}

void
InsynicControlPanel::onAddKeyClicked()
{
    emit addKeyRequested();
}

void
InsynicControlPanel::onSaveProfileClicked()
{
    emit saveProfileRequested();
}

void
InsynicControlPanel::onApplyProfileClicked()
{
    QString name = m_profileCombo->currentText();
    if (!name.isEmpty()) {
        emit profileSelected(name);
    }
}

void
InsynicControlPanel::onDeleteProfileClicked()
{
    QString name = m_profileCombo->currentText();
    if (!name.isEmpty()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, tr("Confirm Delete"), 
            tr("Are you sure you want to delete profile \"%1\"?").arg(name),
            QMessageBox::Yes | QMessageBox::No);
        
        if (reply == QMessageBox::Yes) {
            if (m_profileManager->deleteProfile(name)) {
                updateProfileCombo();
                QMessageBox::information(this, tr("Success"), tr("Profile deleted successfully."));
            } else {
                QMessageBox::warning(this, tr("Error"), tr("Failed to delete profile."));
            }
        }
    }
}

void
InsynicControlPanel::updateProfileCombo()
{
    QString current = m_profileCombo->currentText();
    m_profileCombo->clear();
    QStringList names = m_profileManager->getProfileNames();
    foreach (const QString &name, names) {
        m_profileCombo->addItem(name);
    }
    if (!current.isEmpty()) {
        int idx = m_profileCombo->findText(current);
        if (idx >= 0) {
            m_profileCombo->setCurrentIndex(idx);
        }
    }
}
