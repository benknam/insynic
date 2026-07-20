#ifndef INSYNIC_SETTINGS_DIALOG_H
#define INSYNIC_SETTINGS_DIALOG_H

#include <QDialog>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>
#include <QCheckBox>

class InsynicSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InsynicSettingsDialog(QWidget *parent = nullptr);
    ~InsynicSettingsDialog();

    void setMaxSize(int size);
    int maxSize() const;

    void setMaxFps(int fps);
    int maxFps() const;

    void setVideoBitRate(int bitrate);
    int videoBitRate() const;

    void setTurnScreenOff(bool on);
    bool turnScreenOff() const;

    void setStayAwake(bool on);
    bool stayAwake() const;

    void setPowerOn(bool on);
    bool powerOn() const;

    void setDisableScreensaver(bool on);
    bool disableScreensaver() const;

    void setControlEnabled(bool on);
    bool controlEnabled() const;

signals:
    void settingsChanged();

private slots:
    void onApplyClicked();
    void onResetClicked();

private:
    void setupUi();

    QSpinBox *m_maxSizeSpinBox;
    QComboBox *m_maxSizeComboBox;
    QSpinBox *m_maxFpsSpinBox;
    QComboBox *m_maxFpsComboBox;
    QSpinBox *m_videoBitRateSpinBox;
    QComboBox *m_videoBitRateComboBox;

    QCheckBox *m_turnScreenOffCheck;
    QCheckBox *m_stayAwakeCheck;
    QCheckBox *m_powerOnCheck;
    QCheckBox *m_disableScreensaverCheck;
    QCheckBox *m_controlEnabledCheck;

    int m_originalMaxSize;
    int m_originalMaxFps;
    int m_originalBitRate;
};

#endif