#include "insynic_keyconfigdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QKeyEvent>
#include <QColorDialog>
#include <QCheckBox>

InsynicKeyConfigDialog::InsynicKeyConfigDialog(const VirtualKey &key, QWidget *parent)
    : QDialog(parent)
    , m_key(key)
    , m_recordedKeyCode(key.keyCode)
    , m_recordedKeyName(key.keyName)
    , m_isRecording(false)
    , m_selectedColor(key.color)
{
    setupUi();
}

InsynicKeyConfigDialog::~InsynicKeyConfigDialog()
{
}

VirtualKey InsynicKeyConfigDialog::getKey() const
{
    return m_key;
}

void InsynicKeyConfigDialog::setupUi()
{
    setWindowTitle(tr("Configure Key"));
    setFixedSize(320, 320);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QHBoxLayout *keyLayout = new QHBoxLayout();
    QLabel *keyLabel = new QLabel(tr("Key:"), this);
    m_keyBtn = new QPushButton(this);
    m_keyBtn->setMinimumHeight(32);
    connect(m_keyBtn, &QPushButton::clicked, this, &InsynicKeyConfigDialog::onRecordKeyClicked);
    keyLayout->addWidget(keyLabel);
    keyLayout->addWidget(m_keyBtn);
    mainLayout->addLayout(keyLayout);
    
    QHBoxLayout *sizeLayout = new QHBoxLayout();
    QLabel *sizeLabel = new QLabel(tr("Size (px):"), this);
    m_sizeSpin = new QSpinBox(this);
    m_sizeSpin->setRange(20, 100);
    m_sizeSpin->setValue(m_key.size);
    sizeLayout->addWidget(sizeLabel);
    sizeLayout->addWidget(m_sizeSpin);
    mainLayout->addLayout(sizeLayout);
    
    QHBoxLayout *opacityLayout = new QHBoxLayout();
    QLabel *opacityLabel = new QLabel(tr("Opacity:"), this);
    m_opacitySlider = new QSlider(Qt::Horizontal, this);
    m_opacitySlider->setRange(1, 100);
    m_opacitySlider->setValue(m_key.opacity);
    m_opacityValueLabel = new QLabel(QString("%1%").arg(m_key.opacity), this);
    m_opacityValueLabel->setMinimumWidth(40);
    m_opacityValueLabel->setAlignment(Qt::AlignRight);
    connect(m_opacitySlider, &QSlider::valueChanged, this, [this](int value) {
        m_opacityValueLabel->setText(QString("%1%").arg(value));
    });
    opacityLayout->addWidget(opacityLabel);
    opacityLayout->addWidget(m_opacitySlider);
    opacityLayout->addWidget(m_opacityValueLabel);
    mainLayout->addLayout(opacityLayout);
    
    QHBoxLayout *colorLayout = new QHBoxLayout();
    QLabel *colorLabel = new QLabel(tr("Color:"), this);
    m_colorBtn = new QPushButton(this);
    m_colorBtn->setMinimumHeight(32);
    connect(m_colorBtn, &QPushButton::clicked, this, &InsynicKeyConfigDialog::onColorClicked);
    colorLayout->addWidget(colorLabel);
    colorLayout->addWidget(m_colorBtn);
    mainLayout->addLayout(colorLayout);
    
    QHBoxLayout *toggleLayout = new QHBoxLayout();
    m_toggleCheckBox = new QCheckBox(tr("Toggle"), this);
    m_toggleCheckBox->setChecked(m_key.toggle);
    toggleLayout->addWidget(m_toggleCheckBox);
    mainLayout->addLayout(toggleLayout);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_okBtn = new QPushButton(tr("OK"), this);
    m_cancelBtn = new QPushButton(tr("Cancel"), this);
    m_deleteBtn = new QPushButton(tr("Delete"), this);
    
    buttonLayout->addWidget(m_okBtn);
    buttonLayout->addWidget(m_cancelBtn);
    buttonLayout->addWidget(m_deleteBtn);
    mainLayout->addLayout(buttonLayout);
    
    connect(m_okBtn, &QPushButton::clicked, this, &InsynicKeyConfigDialog::onOkClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &InsynicKeyConfigDialog::onCancelClicked);
    connect(m_deleteBtn, &QPushButton::clicked, this, &InsynicKeyConfigDialog::onDeleteClicked);
    
    updateKeyButton();
    updateColorButton();
}

void InsynicKeyConfigDialog::updateKeyButton()
{
    if (m_isRecording) {
        m_keyBtn->setText(tr("Press a key (Esc to cancel)..."));
        m_keyBtn->setStyleSheet("background-color: #ffe0e0;");
    } else if (m_recordedKeyCode != 0) {
        m_keyBtn->setText(m_recordedKeyName);
        m_keyBtn->setStyleSheet("");
    } else {
        m_keyBtn->setText(tr("Click to set key"));
        m_keyBtn->setStyleSheet("");
    }
}

void InsynicKeyConfigDialog::updateColorButton()
{
    QString colorName = m_selectedColor.name();
    m_colorBtn->setText(colorName);
    m_colorBtn->setStyleSheet(QString(
        "background-color: %1;"
        "color: %2;"
    ).arg(colorName).arg(
        m_selectedColor.lightness() > 128 ? "#000000" : "#ffffff"
    ));
}

void InsynicKeyConfigDialog::onRecordKeyClicked()
{
    m_isRecording = true;
    updateKeyButton();
    setFocus();
}

void InsynicKeyConfigDialog::onColorClicked()
{
    QColor color = QColorDialog::getColor(m_selectedColor, this, tr("Select Key Color"));
    if (color.isValid()) {
        m_selectedColor = color;
        updateColorButton();
    }
}

void InsynicKeyConfigDialog::keyPressEvent(QKeyEvent *event)
{
    if (m_isRecording) {
        if (event->key() == Qt::Key_Escape) {
            m_isRecording = false;
            updateKeyButton();
            event->accept();
            return;
        }
        
        m_recordedKeyCode = event->key();
        m_recordedKeyName = QKeySequence(event->key()).toString();
        if (m_recordedKeyName.isEmpty()) {
            m_recordedKeyName = QString("Key %1").arg(event->key());
        }
        
        m_isRecording = false;
        updateKeyButton();
        event->accept();
        return;
    }
    
    QDialog::keyPressEvent(event);
}

void InsynicKeyConfigDialog::onOkClicked()
{
    m_key.keyCode = m_recordedKeyCode;
    m_key.keyName = m_recordedKeyName;
    m_key.size = m_sizeSpin->value();
    m_key.opacity = m_opacitySlider->value();
    m_key.color = m_selectedColor;
    m_key.toggle = m_toggleCheckBox->isChecked();
    accept();
}

void InsynicKeyConfigDialog::onCancelClicked()
{
    reject();
}

void InsynicKeyConfigDialog::onDeleteClicked()
{
    emit keyDeleted();
    accept();
}
