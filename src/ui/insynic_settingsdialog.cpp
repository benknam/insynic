#include "insynic_settingsdialog.h"

#include <QDialogButtonBox>

InsynicSettingsDialog::InsynicSettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi();
}

InsynicSettingsDialog::~InsynicSettingsDialog()
{
}

void InsynicSettingsDialog::setupUi()
{
    setWindowTitle(tr("Streaming Settings"));
    setMinimumWidth(420);
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QGroupBox *videoGroup = new QGroupBox(tr("Video Settings"), this);
    QFormLayout *videoLayout = new QFormLayout(videoGroup);

    QHBoxLayout *maxSizeLayout = new QHBoxLayout();
    m_maxSizeSpinBox = new QSpinBox(this);
    m_maxSizeSpinBox->setRange(240, 4096);
    m_maxSizeSpinBox->setSingleStep(64);
    m_maxSizeSpinBox->setValue(1024);
    m_maxSizeComboBox = new QComboBox(this);
    m_maxSizeComboBox->addItem(tr("240p"), 240);
    m_maxSizeComboBox->addItem(tr("360p"), 360);
    m_maxSizeComboBox->addItem(tr("480p"), 480);
    m_maxSizeComboBox->addItem(tr("720p"), 720);
    m_maxSizeComboBox->addItem(tr("1080p"), 1080);
    m_maxSizeComboBox->addItem(tr("1440p"), 1440);
    m_maxSizeComboBox->addItem(tr("2160p"), 2160);
    m_maxSizeComboBox->addItem(tr("Custom"), 0);
    maxSizeLayout->addWidget(m_maxSizeSpinBox);
    maxSizeLayout->addWidget(m_maxSizeComboBox);
    videoLayout->addRow(tr("Max Resolution:"), maxSizeLayout);

    connect(m_maxSizeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this](int index) {
                int value = m_maxSizeComboBox->itemData(index).toInt();
                if (value > 0) {
                    m_maxSizeSpinBox->setValue(value);
                    m_maxSizeSpinBox->setEnabled(false);
                } else {
                    m_maxSizeSpinBox->setEnabled(true);
                }
            });

    QHBoxLayout *maxFpsLayout = new QHBoxLayout();
    m_maxFpsSpinBox = new QSpinBox(this);
    m_maxFpsSpinBox->setRange(5, 120);
    m_maxFpsSpinBox->setSingleStep(5);
    m_maxFpsSpinBox->setValue(60);
    m_maxFpsComboBox = new QComboBox(this);
    m_maxFpsComboBox->addItem(tr("15 FPS"), 15);
    m_maxFpsComboBox->addItem(tr("30 FPS"), 30);
    m_maxFpsComboBox->addItem(tr("60 FPS"), 60);
    m_maxFpsComboBox->addItem(tr("120 FPS"), 120);
    m_maxFpsComboBox->addItem(tr("Custom"), 0);
    maxFpsLayout->addWidget(m_maxFpsSpinBox);
    maxFpsLayout->addWidget(m_maxFpsComboBox);
    videoLayout->addRow(tr("Max FPS:"), maxFpsLayout);

    connect(m_maxFpsComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this](int index) {
                int value = m_maxFpsComboBox->itemData(index).toInt();
                if (value > 0) {
                    m_maxFpsSpinBox->setValue(value);
                    m_maxFpsSpinBox->setEnabled(false);
                } else {
                    m_maxFpsSpinBox->setEnabled(true);
                }
            });

    QHBoxLayout *bitRateLayout = new QHBoxLayout();
    m_videoBitRateSpinBox = new QSpinBox(this);
    m_videoBitRateSpinBox->setRange(1, 50);
    m_videoBitRateSpinBox->setSingleStep(1);
    m_videoBitRateSpinBox->setValue(8);
    m_videoBitRateComboBox = new QComboBox(this);
    m_videoBitRateComboBox->addItem(tr("2 Mbps"), 2);
    m_videoBitRateComboBox->addItem(tr("4 Mbps"), 4);
    m_videoBitRateComboBox->addItem(tr("8 Mbps"), 8);
    m_videoBitRateComboBox->addItem(tr("16 Mbps"), 16);
    m_videoBitRateComboBox->addItem(tr("Custom"), 0);
    bitRateLayout->addWidget(m_videoBitRateSpinBox);
    bitRateLayout->addWidget(m_videoBitRateComboBox);
    videoLayout->addRow(tr("Bit Rate (Mbps):"), bitRateLayout);

    connect(m_videoBitRateComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this](int index) {
                int value = m_videoBitRateComboBox->itemData(index).toInt();
                if (value > 0) {
                    m_videoBitRateSpinBox->setValue(value);
                    m_videoBitRateSpinBox->setEnabled(false);
                } else {
                    m_videoBitRateSpinBox->setEnabled(true);
                }
            });

    mainLayout->addWidget(videoGroup);

    QGroupBox *scrcpyGroup = new QGroupBox(tr("scrcpy Options"), this);
    QVBoxLayout *scrcpyLayout = new QVBoxLayout(scrcpyGroup);

    m_turnScreenOffCheck = new QCheckBox(tr("Turn screen off during mirroring"), this);
    m_turnScreenOffCheck->setToolTip(tr("Turn the device screen off when mirroring starts."));
    scrcpyLayout->addWidget(m_turnScreenOffCheck);

    m_stayAwakeCheck = new QCheckBox(tr("Keep device awake"), this);
    m_stayAwakeCheck->setToolTip(tr("Prevent the device from sleeping while connected."));
    scrcpyLayout->addWidget(m_stayAwakeCheck);

    m_powerOnCheck = new QCheckBox(tr("Power on on connect"), this);
    m_powerOnCheck->setToolTip(tr("Power on the device screen when a connection is established."));
    scrcpyLayout->addWidget(m_powerOnCheck);

    m_disableScreensaverCheck = new QCheckBox(tr("Disable screensaver"), this);
    m_disableScreensaverCheck->setToolTip(tr("Disable the computer screensaver while a device is connected."));
    scrcpyLayout->addWidget(m_disableScreensaverCheck);

    m_controlEnabledCheck = new QCheckBox(tr("Enable control (mouse/keyboard)"), this);
    m_controlEnabledCheck->setToolTip(tr("Allow controlling the device with mouse and keyboard."));
    m_controlEnabledCheck->setChecked(true);
    scrcpyLayout->addWidget(m_controlEnabledCheck);

    mainLayout->addWidget(scrcpyGroup);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Apply | QDialogButtonBox::Reset | QDialogButtonBox::Cancel,
        this);
    mainLayout->addWidget(buttonBox);

    connect(buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked,
            this, &InsynicSettingsDialog::onApplyClicked);
    connect(buttonBox->button(QDialogButtonBox::Reset), &QPushButton::clicked,
            this, &InsynicSettingsDialog::onResetClicked);
    connect(buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked,
            this, &QDialog::reject);
}

void InsynicSettingsDialog::setMaxSize(int size)
{
    m_maxSizeSpinBox->setValue(size);
    m_originalMaxSize = size;

    int index = m_maxSizeComboBox->findData(size);
    if (index >= 0) {
        m_maxSizeComboBox->setCurrentIndex(index);
        m_maxSizeSpinBox->setEnabled(false);
    } else {
        m_maxSizeComboBox->setCurrentIndex(m_maxSizeComboBox->count() - 1);
        m_maxSizeSpinBox->setEnabled(true);
    }
}

int InsynicSettingsDialog::maxSize() const
{
    return m_maxSizeSpinBox->value();
}

void InsynicSettingsDialog::setMaxFps(int fps)
{
    m_maxFpsSpinBox->setValue(fps);
    m_originalMaxFps = fps;

    int index = m_maxFpsComboBox->findData(fps);
    if (index >= 0) {
        m_maxFpsComboBox->setCurrentIndex(index);
        m_maxFpsSpinBox->setEnabled(false);
    } else {
        m_maxFpsComboBox->setCurrentIndex(m_maxFpsComboBox->count() - 1);
        m_maxFpsSpinBox->setEnabled(true);
    }
}

int InsynicSettingsDialog::maxFps() const
{
    return m_maxFpsSpinBox->value();
}

void InsynicSettingsDialog::setVideoBitRate(int bitrate)
{
    m_videoBitRateSpinBox->setValue(bitrate);
    m_originalBitRate = bitrate;

    int index = m_videoBitRateComboBox->findData(bitrate);
    if (index >= 0) {
        m_videoBitRateComboBox->setCurrentIndex(index);
        m_videoBitRateSpinBox->setEnabled(false);
    } else {
        m_videoBitRateComboBox->setCurrentIndex(m_videoBitRateComboBox->count() - 1);
        m_videoBitRateSpinBox->setEnabled(true);
    }
}

int InsynicSettingsDialog::videoBitRate() const
{
    return m_videoBitRateSpinBox->value();
}

void InsynicSettingsDialog::setTurnScreenOff(bool on)
{
    m_turnScreenOffCheck->setChecked(on);
}

bool InsynicSettingsDialog::turnScreenOff() const
{
    return m_turnScreenOffCheck->isChecked();
}

void InsynicSettingsDialog::setStayAwake(bool on)
{
    m_stayAwakeCheck->setChecked(on);
}

bool InsynicSettingsDialog::stayAwake() const
{
    return m_stayAwakeCheck->isChecked();
}

void InsynicSettingsDialog::setPowerOn(bool on)
{
    m_powerOnCheck->setChecked(on);
}

bool InsynicSettingsDialog::powerOn() const
{
    return m_powerOnCheck->isChecked();
}

void InsynicSettingsDialog::setDisableScreensaver(bool on)
{
    m_disableScreensaverCheck->setChecked(on);
}

bool InsynicSettingsDialog::disableScreensaver() const
{
    return m_disableScreensaverCheck->isChecked();
}

void InsynicSettingsDialog::setControlEnabled(bool on)
{
    m_controlEnabledCheck->setChecked(on);
}

bool InsynicSettingsDialog::controlEnabled() const
{
    return m_controlEnabledCheck->isChecked();
}

void InsynicSettingsDialog::onApplyClicked()
{
    emit settingsChanged();
    accept();
}

void InsynicSettingsDialog::onResetClicked()
{
    setMaxSize(m_originalMaxSize);
    setMaxFps(m_originalMaxFps);
    setVideoBitRate(m_originalBitRate);
}
