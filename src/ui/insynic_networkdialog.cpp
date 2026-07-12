#include "insynic_networkdialog.h"

InsynicNetworkDialog::InsynicNetworkDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi();
}

InsynicNetworkDialog::~InsynicNetworkDialog()
{
}

void InsynicNetworkDialog::setupUi()
{
    setWindowTitle(tr("Network Connection"));
    setMinimumWidth(320);
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    QHBoxLayout *ipLayout = new QHBoxLayout();
    QLabel *ipLabel = new QLabel(tr("Device IP:"), this);
    ipLabel->setMinimumWidth(70);
    m_ipInput = new QLineEdit(this);
    m_ipInput->setPlaceholderText("192.168.x.x");
    ipLayout->addWidget(ipLabel);
    ipLayout->addWidget(m_ipInput);
    mainLayout->addLayout(ipLayout);

    QHBoxLayout *portLayout = new QHBoxLayout();
    QLabel *portLabel = new QLabel(tr("Port:"), this);
    portLabel->setMinimumWidth(70);
    m_portInput = new QLineEdit(this);
    m_portInput->setText("5555");
    portLayout->addWidget(portLabel);
    portLayout->addWidget(m_portInput);
    mainLayout->addLayout(portLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        this);
    mainLayout->addWidget(buttonBox);

    connect(buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked,
            this, &InsynicNetworkDialog::accept);
    connect(buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked,
            this, &InsynicNetworkDialog::reject);
}

QString InsynicNetworkDialog::ipAddress() const
{
    return m_ipInput->text().trimmed();
}

int InsynicNetworkDialog::port() const
{
    bool ok;
    int port = m_portInput->text().toInt(&ok);
    if (!ok || port <= 0) {
        return 5555;
    }
    return port;
}