#include "insynic_devicecontrol.h"
#include <QMessageBox>

InsynicDeviceControl::InsynicDeviceControl(QWidget *parent)
    : QWidget(parent)
    , m_scrcpy(nullptr)
    , m_connected(false)
{
    m_profileManager = new InsynicProfileManager(this);
    setupUi();
}

InsynicDeviceControl::~InsynicDeviceControl()
{
}

void
InsynicDeviceControl::setupUi()
{
    setMinimumWidth(180);
    setMaximumWidth(200);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);

    QGroupBox *androidKeysGroup = new QGroupBox(tr("Android Keys"));
    QGridLayout *androidKeysLayout = new QGridLayout(androidKeysGroup);

    m_backBtn = createButton(tr("Back"), SLOT(onBackClicked()));
    m_homeBtn = createButton(tr("Home"), SLOT(onHomeClicked()));
    m_recentBtn = createButton(tr("Task"), SLOT(onRecentClicked()));
    m_menuBtn = createButton(tr("Menu"), SLOT(onMenuClicked()));

    androidKeysLayout->addWidget(m_backBtn, 0, 0);
    androidKeysLayout->addWidget(m_homeBtn, 0, 1);
    androidKeysLayout->addWidget(m_recentBtn, 1, 0);
    androidKeysLayout->addWidget(m_menuBtn, 1, 1);

    mainLayout->addWidget(androidKeysGroup);

    QGroupBox *systemGroup = new QGroupBox(tr("System"));
    QGridLayout *systemLayout = new QGridLayout(systemGroup);

    m_volUpBtn = createButton(tr("Vol+"), SLOT(onVolumeUpClicked()));
    m_volDownBtn = createButton(tr("Vol-"), SLOT(onVolumeDownClicked()));
    m_notifBtn = createButton(tr("Notif"), SLOT(onNotificationClicked()));
    m_rotateBtn = createButton(tr("Rotate"), SLOT(onRotateClicked()));
    m_screenToggleBtn = createButton(tr("Screen"), SLOT(onScreenToggleClicked()));

    systemLayout->addWidget(m_volUpBtn, 0, 0);
    systemLayout->addWidget(m_volDownBtn, 0, 1);
    systemLayout->addWidget(m_notifBtn, 1, 0);
    systemLayout->addWidget(m_rotateBtn, 1, 1);
    systemLayout->addWidget(m_screenToggleBtn, 2, 0, 1, 2);

    mainLayout->addWidget(systemGroup);

    QGroupBox *utilitiesGroup = new QGroupBox(tr("Utilities"));
    QVBoxLayout *utilitiesLayout = new QVBoxLayout(utilitiesGroup);

    m_addKeyBtn = new QPushButton(tr("Add Key"));
    m_addKeyBtn->setEnabled(false);
    connect(m_addKeyBtn, &QPushButton::clicked, this, &InsynicDeviceControl::addKeyRequested);
    utilitiesLayout->addWidget(m_addKeyBtn);

    m_saveProfileBtn = new QPushButton(tr("Save Profile"));
    m_saveProfileBtn->setEnabled(false);
    connect(m_saveProfileBtn, &QPushButton::clicked, this, &InsynicDeviceControl::saveProfileRequested);
    utilitiesLayout->addWidget(m_saveProfileBtn);

    m_profileCombo = new QComboBox();
    updateProfileCombo();
    utilitiesLayout->addWidget(m_profileCombo);

    QHBoxLayout *profileButtonsLayout = new QHBoxLayout();
    m_applyProfileBtn = new QPushButton(tr("Apply"));
    m_deleteProfileBtn = new QPushButton(tr("Delete"));
    profileButtonsLayout->addWidget(m_applyProfileBtn);
    profileButtonsLayout->addWidget(m_deleteProfileBtn);
    utilitiesLayout->addLayout(profileButtonsLayout);

    connect(m_profileCombo, &QComboBox::currentTextChanged,
            this, &InsynicDeviceControl::profileSelected);
    connect(m_applyProfileBtn, &QPushButton::clicked, this, &InsynicDeviceControl::onApplyProfileClicked);
    connect(m_deleteProfileBtn, &QPushButton::clicked, this, &InsynicDeviceControl::onDeleteProfileClicked);

    mainLayout->addWidget(utilitiesGroup);

    m_closeBtn = new QPushButton(tr("Close Connection"));
    connect(m_closeBtn, &QPushButton::clicked, this, &InsynicDeviceControl::closeRequested);
    mainLayout->addWidget(m_closeBtn);

    m_otgBtn = new QPushButton(tr("OTG Input"));
    m_otgBtn->setStyleSheet(
        "QPushButton { background-color: #2196F3; color: white; "
        "border-radius: 6px; font-weight: bold; }"
        "QPushButton:hover { background-color: #1976D2; }"
        "QPushButton:disabled { background-color: #cccccc; }"
    );
    m_otgBtn->setEnabled(false);
    connect(m_otgBtn, &QPushButton::clicked, this, &InsynicDeviceControl::onOtgInputClicked);
    mainLayout->addWidget(m_otgBtn);

    mainLayout->addStretch();
}

QPushButton *
InsynicDeviceControl::createButton(const QString &text, const char *slot)
{
    QPushButton *btn = new QPushButton(text);
    connect(btn, SIGNAL(clicked()), this, slot);
    return btn;
}

void
InsynicDeviceControl::setScrcpy(struct insynic_scrcpy *scrcpy)
{
    m_scrcpy = scrcpy;
}

void
InsynicDeviceControl::setConnected(bool connected)
{
    m_connected = connected;
    m_addKeyBtn->setEnabled(connected);
    m_saveProfileBtn->setEnabled(connected);
    m_otgBtn->setEnabled(connected);
}

void
InsynicDeviceControl::updateProfileCombo()
{
    QString current = m_profileCombo->currentText();
    m_profileCombo->clear();
    QStringList profiles = m_profileManager->getProfileNames();
    m_profileCombo->addItems(profiles);
    int idx = m_profileCombo->findText(current);
    if (idx >= 0) {
        m_profileCombo->setCurrentIndex(idx);
    }
}

void
InsynicDeviceControl::onBackClicked()
{
    if (m_scrcpy) {
        insynic_scrcpy_inject_back(m_scrcpy);
    }
}

void
InsynicDeviceControl::onHomeClicked()
{
    if (m_scrcpy) {
        insynic_scrcpy_inject_home(m_scrcpy);
    }
}

void
InsynicDeviceControl::onRecentClicked()
{
    if (m_scrcpy) {
        insynic_scrcpy_inject_recent(m_scrcpy);
    }
}

void
InsynicDeviceControl::onMenuClicked()
{
    if (m_scrcpy) {
        insynic_scrcpy_inject_menu(m_scrcpy);
    }
}

void
InsynicDeviceControl::onVolumeUpClicked()
{
    if (m_scrcpy) {
        insynic_scrcpy_inject_volume_up(m_scrcpy);
    }
}

void
InsynicDeviceControl::onVolumeDownClicked()
{
    if (m_scrcpy) {
        insynic_scrcpy_inject_volume_down(m_scrcpy);
    }
}

void
InsynicDeviceControl::onNotificationClicked()
{
    if (m_scrcpy) {
        insynic_scrcpy_expand_notification_panel(m_scrcpy);
    }
}

void
InsynicDeviceControl::onRotateClicked()
{
    if (m_scrcpy) {
        insynic_scrcpy_rotate_device(m_scrcpy);
    }
}

void
InsynicDeviceControl::onScreenToggleClicked()
{
    if (m_scrcpy) {
        insynic_scrcpy_toggle_display(m_scrcpy);
    }
}

void
InsynicDeviceControl::onAddKeyClicked()
{
    emit addKeyRequested();
}

void
InsynicDeviceControl::onSaveProfileClicked()
{
    emit saveProfileRequested();
}

void
InsynicDeviceControl::onApplyProfileClicked()
{
    QString name = m_profileCombo->currentText();
    if (!name.isEmpty()) {
        emit profileSelected(name);
    }
}

void
InsynicDeviceControl::onDeleteProfileClicked()
{
    QString name = m_profileCombo->currentText();
    if (!name.isEmpty()) {
        if (QMessageBox::question(this, tr("Delete Profile"),
            tr("Are you sure you want to delete profile \"%1\"?").arg(name)) == QMessageBox::Yes) {
            m_profileManager->deleteProfile(name);
            updateProfileCombo();
        }
    }
}

void
InsynicDeviceControl::onCloseClicked()
{
    emit closeRequested();
}

void
InsynicDeviceControl::onOtgInputClicked()
{
    emit otgInputRequested();
}