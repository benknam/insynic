#ifndef INSYNIC_SAVE_PROFILE_DIALOG_H
#define INSYNIC_SAVE_PROFILE_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QStringList>

class InsynicSaveProfileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InsynicSaveProfileDialog(const QStringList &existingNames, QWidget *parent = nullptr);
    ~InsynicSaveProfileDialog();
    
    QString getProfileName() const;
    bool shouldOverwrite() const;

private slots:
    void onOkClicked();
    void onCancelClicked();

private:
    void setupUi();
    
    QStringList m_existingNames;
    QString m_profileName;
    bool m_overwrite;
    QLineEdit *m_nameEdit;
    QPushButton *m_okBtn;
    QPushButton *m_cancelBtn;
};

#endif