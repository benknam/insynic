#ifndef INSYNIC_KEY_CONFIG_DIALOG_H
#define INSYNIC_KEY_CONFIG_DIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QSpinBox>
#include <QLabel>
#include "insynic_virtualkey.h"

class InsynicKeyConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InsynicKeyConfigDialog(const VirtualKey &key, QWidget *parent = nullptr);
    ~InsynicKeyConfigDialog();
    
    VirtualKey getKey() const;

private slots:
    void onRecordKeyClicked();
    void onOkClicked();
    void onCancelClicked();
    void onDeleteClicked();

signals:
    void keyDeleted();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void setupUi();
    void updateKeyButton();
    
    VirtualKey m_key;
    int m_recordedKeyCode;
    QString m_recordedKeyName;
    bool m_isRecording;
    
    QPushButton *m_keyBtn;
    QSpinBox *m_sizeSpin;
    QPushButton *m_okBtn;
    QPushButton *m_cancelBtn;
    QPushButton *m_deleteBtn;
};

#endif
