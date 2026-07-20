#ifndef INSYNIC_CONTROL_BAR_H
#define INSYNIC_CONTROL_BAR_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QComboBox>

#include "insynic_scrcpy.h"
#include "insynic_profilemanager.h"

class InsynicControlBar : public QWidget
{
    Q_OBJECT

public:
    explicit InsynicControlBar(QWidget *parent = nullptr);
    ~InsynicControlBar();

    void setScrcpy(struct insynic_scrcpy *scrcpy);
    void setConnected(bool connected);
    void updateProfileCombo();
    void retranslateUi();

signals:
    void addKeyRequested();
    void profileSelected(const QString &name);
    void saveProfileRequested();

private slots:
    void onBackClicked();
    void onHomeClicked();
    void onRecentClicked();
    void onMenuClicked();
    void onExpandClicked();
    void onApplyProfileClicked();
    void onDeleteProfileClicked();

private:
    void setupUi();

    struct insynic_scrcpy *m_scrcpy;
    bool m_connected;
    bool m_expanded;

    QPushButton *m_backBtn;
    QPushButton *m_homeBtn;
    QPushButton *m_recentBtn;
    QPushButton *m_menuBtn;
    QPushButton *m_expandBtn;

    QWidget *m_expandedPanel;
    QPushButton *m_addKeyBtn;
    QPushButton *m_saveProfileBtn;
    QComboBox *m_profileCombo;
    QPushButton *m_applyProfileBtn;
    QPushButton *m_deleteProfileBtn;

    InsynicProfileManager *m_profileManager;
};

#endif
