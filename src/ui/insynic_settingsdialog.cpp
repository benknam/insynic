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
    setMinimumWidth(380);
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

void InsynicSettingsDialog::onApplyClicked()
{
    emit settingsChanged(maxSize(), maxFps(), videoBitRate());
    accept();
}

void InsynicSettingsDialog::onResetClicked()
{
    setMaxSize(m_originalMaxSize);
    setMaxFps(m_originalMaxFps);
    setVideoBitRate(m_originalBitRate);
}