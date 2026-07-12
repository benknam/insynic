#ifndef INSYNIC_NETWORK_DIALOG_H
#define INSYNIC_NETWORK_DIALOG_H

#include <QDialog>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>

class InsynicNetworkDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InsynicNetworkDialog(QWidget *parent = nullptr);
    ~InsynicNetworkDialog();

    QString ipAddress() const;
    int port() const;

private:
    void setupUi();

    QLineEdit *m_ipInput;
    QLineEdit *m_portInput;
};

#endif