#include "insynic_saveprofiledialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

InsynicSaveProfileDialog::InsynicSaveProfileDialog(const QStringList &existingNames, QWidget *parent)
    : QDialog(parent)
    , m_existingNames(existingNames)
    , m_overwrite(false)
{
    setupUi();
}

InsynicSaveProfileDialog::~InsynicSaveProfileDialog()
{
}

QString InsynicSaveProfileDialog::getProfileName() const
{
    return m_profileName;
}

bool InsynicSaveProfileDialog::shouldOverwrite() const
{
    return m_overwrite;
}

void InsynicSaveProfileDialog::setupUi()
{
    setWindowTitle(tr("Save Profile"));
    setFixedSize(300, 120);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QHBoxLayout *nameLayout = new QHBoxLayout();
    QLabel *nameLabel = new QLabel(tr("Profile Name:"), this);
    m_nameEdit = new QLineEdit(this);
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(m_nameEdit);
    mainLayout->addLayout(nameLayout);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_okBtn = new QPushButton(tr("Save"), this);
    m_cancelBtn = new QPushButton(tr("Cancel"), this);
    
    buttonLayout->addWidget(m_okBtn);
    buttonLayout->addWidget(m_cancelBtn);
    mainLayout->addLayout(buttonLayout);
    
    connect(m_okBtn, SIGNAL(clicked()), this, SLOT(onOkClicked()));
    connect(m_cancelBtn, SIGNAL(clicked()), this, SLOT(onCancelClicked()));
}

void InsynicSaveProfileDialog::onOkClicked()
{
    QString name = m_nameEdit->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Profile name cannot be empty."));
        return;
    }
    
    if (m_existingNames.contains(name)) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, tr("Confirm Overwrite"), 
            tr("Profile \"%1\" already exists. Do you want to overwrite it?").arg(name),
            QMessageBox::Yes | QMessageBox::No);
        
        if (reply == QMessageBox::No) {
            return;
        }
        m_overwrite = true;
    }
    
    m_profileName = name;
    accept();
}

void InsynicSaveProfileDialog::onCancelClicked()
{
    reject();
}