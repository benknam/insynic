#include "insynic_recorddialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QStandardPaths>

InsynicRecordDialog::InsynicRecordDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi();
}

void
InsynicRecordDialog::setupUi()
{
    setWindowTitle(tr("Record Settings"));
    setMinimumWidth(480);
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);

    // Record enable checkbox
    m_recordEnabledCheck = new QCheckBox(tr("Enable recording on connect"), this);
    m_recordEnabledCheck->setToolTip(tr("If checked, recording will start automatically when the device is connected."));
    m_recordEnabledCheck->setChecked(false);
    connect(m_recordEnabledCheck, &QCheckBox::toggled,
            this, &InsynicRecordDialog::onRecordEnabledToggled);
    mainLayout->addWidget(m_recordEnabledCheck);

    // Output file group
    m_fileGroup = new QGroupBox(tr("Output File"), this);
    QFormLayout *fileLayout = new QFormLayout(m_fileGroup);

    QHBoxLayout *pathLayout = new QHBoxLayout();
    m_filePathEdit = new QLineEdit(m_fileGroup);
    m_filePathEdit->setText(defaultFileName());
    pathLayout->addWidget(m_filePathEdit);

    QPushButton *browseBtn = new QPushButton(tr("Browse..."), m_fileGroup);
    connect(browseBtn, &QPushButton::clicked, this, &InsynicRecordDialog::onBrowseClicked);
    pathLayout->addWidget(browseBtn);
    fileLayout->addRow(tr("File Path:"), pathLayout);

    m_formatCombo = new QComboBox(m_fileGroup);
    m_formatCombo->addItem(tr("Auto (by extension)"), 0);
    m_formatCombo->addItem(tr("MP4 (video)"), 1);
    m_formatCombo->addItem(tr("MKV (video)"), 2);
    m_formatCombo->addItem(tr("M4A (audio only)"), 3);
    m_formatCombo->addItem(tr("MKA (audio only)"), 4);
    m_formatCombo->addItem(tr("OPUS (audio only)"), 5);
    m_formatCombo->addItem(tr("AAC (audio only)"), 6);
    m_formatCombo->addItem(tr("FLAC (audio only)"), 7);
    m_formatCombo->addItem(tr("WAV (audio only)"), 8);
    fileLayout->addRow(tr("Format:"), m_formatCombo);
    connect(m_formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &InsynicRecordDialog::onFormatChanged);

    mainLayout->addWidget(m_fileGroup);

    // Stream selection group
    m_streamGroup = new QGroupBox(tr("Stream Selection"), this);
    QVBoxLayout *streamLayout = new QVBoxLayout(m_streamGroup);

    m_videoCheck = new QCheckBox(tr("Record video stream"), m_streamGroup);
    m_videoCheck->setToolTip(tr("Save video stream to the output file."));
    m_videoCheck->setChecked(true);
    streamLayout->addWidget(m_videoCheck);

    m_audioCheck = new QCheckBox(tr("Record audio stream"), m_streamGroup);
    m_audioCheck->setToolTip(tr("Save audio stream to the output file (requires audio streaming enabled in Streaming Settings)."));
    m_audioCheck->setChecked(false);
    streamLayout->addWidget(m_audioCheck);

    mainLayout->addWidget(m_streamGroup);

    // Hint label - always visible, dimmed when recording is disabled
    m_hintLabel = new QLabel(
        tr("Note: Recording starts automatically when the device is connected, "
           "and stops when disconnected."), this);
    m_hintLabel->setWordWrap(true);
    m_hintLabel->setStyleSheet("color: #FF9800; font-size: 11px;");
    mainLayout->addWidget(m_hintLabel);

    // Buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);

    updateControlsEnabled();
}

QString
InsynicRecordDialog::defaultFileName() const
{
    QString desktop = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    return desktop + "/insynic_" + ts + ".mp4";
}

void
InsynicRecordDialog::onBrowseClicked()
{
    QString path = QFileDialog::getSaveFileName(this, tr("Select Output File"),
                                                 m_filePathEdit->text(),
                                                 tr("Video Files (*.mp4 *.mkv);;Audio Files (*.m4a *.mka *.opus *.aac *.flac *.wav);;All Files (*)"));
    if (!path.isEmpty()) {
        m_filePathEdit->setText(path);
        if (path.endsWith(".mkv", Qt::CaseInsensitive)) {
            m_formatCombo->setCurrentIndex(2);
        } else if (path.endsWith(".m4a", Qt::CaseInsensitive)) {
            m_formatCombo->setCurrentIndex(3);
        } else if (path.endsWith(".mka", Qt::CaseInsensitive)) {
            m_formatCombo->setCurrentIndex(4);
        } else if (path.endsWith(".opus", Qt::CaseInsensitive)) {
            m_formatCombo->setCurrentIndex(5);
        } else if (path.endsWith(".aac", Qt::CaseInsensitive)) {
            m_formatCombo->setCurrentIndex(6);
        } else if (path.endsWith(".flac", Qt::CaseInsensitive)) {
            m_formatCombo->setCurrentIndex(7);
        } else if (path.endsWith(".wav", Qt::CaseInsensitive)) {
            m_formatCombo->setCurrentIndex(8);
        } else if (path.endsWith(".mp4", Qt::CaseInsensitive)) {
            m_formatCombo->setCurrentIndex(1);
        } else {
            m_formatCombo->setCurrentIndex(0);
        }
    }
}

void
InsynicRecordDialog::onFormatChanged(int index)
{
    Q_UNUSED(index);
    int data = m_formatCombo->currentData().toInt();
    bool audioOnly = (data >= 3 && data <= 8);
    if (audioOnly) {
        m_videoCheck->setEnabled(false);
        m_videoCheck->setChecked(false);
        m_audioCheck->setEnabled(true);
        m_audioCheck->setChecked(true);
    } else {
        m_videoCheck->setEnabled(true);
        m_audioCheck->setEnabled(true);
    }
}

void
InsynicRecordDialog::onRecordEnabledToggled(bool on)
{
    Q_UNUSED(on);
    updateControlsEnabled();
}

void
InsynicRecordDialog::updateControlsEnabled()
{
    bool enabled = m_recordEnabledCheck->isChecked();
    // Disable entire group boxes to keep layout stable and consistent visual style
    m_fileGroup->setEnabled(enabled);
    m_streamGroup->setEnabled(enabled);
    // Hint label always visible, just dim it when disabled to preserve layout
    if (enabled) {
        m_hintLabel->setStyleSheet("color: #FF9800; font-size: 11px;");
    } else {
        m_hintLabel->setStyleSheet("color: #888; font-size: 11px;");
    }
}

bool
InsynicRecordDialog::recordEnabled() const
{
    return m_recordEnabledCheck->isChecked();
}

QString
InsynicRecordDialog::filePath() const
{
    return m_filePathEdit->text();
}

int
InsynicRecordDialog::format() const
{
    return m_formatCombo->currentData().toInt();
}

bool
InsynicRecordDialog::recordVideo() const
{
    return m_videoCheck->isChecked();
}

bool
InsynicRecordDialog::recordAudio() const
{
    return m_audioCheck->isChecked();
}

void
InsynicRecordDialog::setRecordEnabled(bool on)
{
    m_recordEnabledCheck->setChecked(on);
    updateControlsEnabled();
}

void
InsynicRecordDialog::setFilePath(const QString &path)
{
    m_filePathEdit->setText(path);
}

void
InsynicRecordDialog::setFormat(int format)
{
    int idx = m_formatCombo->findData(format);
    if (idx >= 0) {
        m_formatCombo->setCurrentIndex(idx);
    }
}

void
InsynicRecordDialog::setRecordVideo(bool on)
{
    m_videoCheck->setChecked(on);
}

void
InsynicRecordDialog::setRecordAudio(bool on)
{
    m_audioCheck->setChecked(on);
}
