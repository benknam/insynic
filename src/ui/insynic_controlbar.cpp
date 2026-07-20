#include "insynic_controlbar.h"
#include <QMessageBox>
#include "insynic_profilemanager.h"

InsynicControlBar::InsynicControlBar(QWidget *parent)
    : QWidget(parent)
    , m_scrcpy(nullptr)
    , m_connected(false)
    , m_expanded(false)
{
    m_profileManager = new InsynicProfileManager(this);
    setupUi();
}

InsynicControlBar::~InsynicControlBar()
{
}

void
InsynicControlBar::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(4);
    mainLayout->setContentsMargins(4, 4, 4, 4);

    QHBoxLayout *basicKeysLayout = new QHBoxLayout();
    basicKeysLayout->setSpacing(4);
    basicKeysLayout->addStretch();

    QString btnStyle =
        "QPushButton { background-color: #4a4a4a; color: white; "
        "border-radius: 4px; font-size: 14px; }"
        "QPushButton:hover { background-color: #5a5a5a; }";

    m_backBtn = new QPushButton(tr("←"));
    m_backBtn->setMinimumSize(48, 32);
    m_backBtn->setStyleSheet(btnStyle);
    m_backBtn->setToolTip(tr("Back"));
    connect(m_backBtn, &QPushButton::clicked, this, &InsynicControlBar::onBackClicked);

    m_homeBtn = new QPushButton(tr("◉"));
    m_homeBtn->setMinimumSize(48, 32);
    m_homeBtn->setStyleSheet(btnStyle);
    m_homeBtn->setToolTip(tr("Home"));
    connect(m_homeBtn, &QPushButton::clicked, this, &InsynicControlBar::onHomeClicked);

    m_recentBtn = new QPushButton(tr("⊞"));
    m_recentBtn->setMinimumSize(48, 32);
    m_recentBtn->setStyleSheet(btnStyle);
    m_recentBtn->setToolTip(tr("Recent Apps"));
    connect(m_recentBtn, &QPushButton::clicked, this, &InsynicControlBar::onRecentClicked);

    m_menuBtn = new QPushButton(tr("≡"));
    m_menuBtn->setMinimumSize(48, 32);
    m_menuBtn->setStyleSheet(btnStyle);
    m_menuBtn->setToolTip(tr("Menu"));
    connect(m_menuBtn, &QPushButton::clicked, this, &InsynicControlBar::onMenuClicked);

    basicKeysLayout->addWidget(m_backBtn);
    basicKeysLayout->addWidget(m_homeBtn);
    basicKeysLayout->addWidget(m_recentBtn);
    basicKeysLayout->addWidget(m_menuBtn);
    basicKeysLayout->addStretch();

    m_expandBtn = new QPushButton(tr("▼"));
    m_expandBtn->setMinimumSize(32, 32);
    m_expandBtn->setStyleSheet(
        "QPushButton { background-color: #3a3a3a; color: white; "
        "border-radius: 4px; font-size: 12px; }"
        "QPushButton:hover { background-color: #4a4a4a; }"
    );
    connect(m_expandBtn, &QPushButton::clicked, this, &InsynicControlBar::onExpandClicked);
    basicKeysLayout->addWidget(m_expandBtn);

    mainLayout->addLayout(basicKeysLayout);

    m_expandedPanel = new QWidget(this);
    m_expandedPanel->setVisible(false);
    QHBoxLayout *expandedLayout = new QHBoxLayout(m_expandedPanel);
    expandedLayout->setSpacing(4);

    m_addKeyBtn = new QPushButton(tr("Add Key"));
    m_addKeyBtn->setEnabled(false);
    m_addKeyBtn->setStyleSheet(
        "QPushButton { background-color: #2196F3; color: white; "
        "border-radius: 4px; padding: 4px 8px; }"
        "QPushButton:hover { background-color: #1976D2; }"
        "QPushButton:disabled { background-color: #2a2a2a; color: #666; }"
    );
    connect(m_addKeyBtn, &QPushButton::clicked, this, &InsynicControlBar::addKeyRequested);

    m_saveProfileBtn = new QPushButton(tr("Save Profile"));
    m_saveProfileBtn->setEnabled(false);
    m_saveProfileBtn->setStyleSheet(
        "QPushButton { background-color: #2196F3; color: white; "
        "border-radius: 4px; padding: 4px 8px; }"
        "QPushButton:hover { background-color: #1976D2; }"
        "QPushButton:disabled { background-color: #2a2a2a; color: #666; }"
    );
    connect(m_saveProfileBtn, &QPushButton::clicked, this, &InsynicControlBar::saveProfileRequested);

    m_profileCombo = new QComboBox();
    m_profileCombo->setMinimumWidth(100);
    updateProfileCombo();

    m_applyProfileBtn = new QPushButton(tr("Apply"));
    m_applyProfileBtn->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; "
        "border-radius: 4px; padding: 4px 8px; }"
        "QPushButton:hover { background-color: #45a049; }"
        "QPushButton:disabled { background-color: #2a2a2a; color: #666; }"
    );
    connect(m_applyProfileBtn, &QPushButton::clicked, this, &InsynicControlBar::onApplyProfileClicked);

    m_deleteProfileBtn = new QPushButton(tr("Delete"));
    m_deleteProfileBtn->setStyleSheet(
        "QPushButton { background-color: #f44336; color: white; "
        "border-radius: 4px; padding: 4px 8px; }"
        "QPushButton:hover { background-color: #d32f2f; }"
        "QPushButton:disabled { background-color: #2a2a2a; color: #666; }"
    );
    connect(m_deleteProfileBtn, &QPushButton::clicked, this, &InsynicControlBar::onDeleteProfileClicked);

    expandedLayout->addStretch();
    expandedLayout->addWidget(m_addKeyBtn);
    expandedLayout->addWidget(m_saveProfileBtn);
    expandedLayout->addWidget(m_profileCombo);
    expandedLayout->addWidget(m_applyProfileBtn);
    expandedLayout->addWidget(m_deleteProfileBtn);
    expandedLayout->addStretch();

    mainLayout->addWidget(m_expandedPanel);

    setStyleSheet("QWidget { background-color: #333333; border: 1px solid #555555; }");
}

void
InsynicControlBar::setScrcpy(struct insynic_scrcpy *scrcpy)
{
    m_scrcpy = scrcpy;
}

void
InsynicControlBar::setConnected(bool connected)
{
    m_connected = connected;
    m_addKeyBtn->setEnabled(connected);
    m_saveProfileBtn->setEnabled(connected);
}

void
InsynicControlBar::updateProfileCombo()
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
InsynicControlBar::retranslateUi()
{
    m_backBtn->setToolTip(tr("Back"));
    m_homeBtn->setToolTip(tr("Home"));
    m_recentBtn->setToolTip(tr("Recent Apps"));
    m_menuBtn->setToolTip(tr("Menu"));
    m_addKeyBtn->setText(tr("Add Key"));
    m_saveProfileBtn->setText(tr("Save Profile"));
    m_applyProfileBtn->setText(tr("Apply"));
    m_deleteProfileBtn->setText(tr("Delete"));
    m_expandBtn->setText(m_expanded ? tr("▲") : tr("▼"));
}

void
InsynicControlBar::onExpandClicked()
{
    m_expanded = !m_expanded;
    m_expandedPanel->setVisible(m_expanded);
    m_expandBtn->setText(m_expanded ? tr("▲") : tr("▼"));
}

void
InsynicControlBar::onBackClicked()
{
    if (m_scrcpy) {
        insynic_scrcpy_inject_back(m_scrcpy);
    }
}

void
InsynicControlBar::onHomeClicked()
{
    if (m_scrcpy) {
        insynic_scrcpy_inject_home(m_scrcpy);
    }
}

void
InsynicControlBar::onRecentClicked()
{
    if (m_scrcpy) {
        insynic_scrcpy_inject_recent(m_scrcpy);
    }
}

void
InsynicControlBar::onMenuClicked()
{
    if (m_scrcpy) {
        insynic_scrcpy_inject_menu(m_scrcpy);
    }
}

void
InsynicControlBar::onApplyProfileClicked()
{
    QString name = m_profileCombo->currentText();
    if (!name.isEmpty()) {
        emit profileSelected(name);
    }
}

void
InsynicControlBar::onDeleteProfileClicked()
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
