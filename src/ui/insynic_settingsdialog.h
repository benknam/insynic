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

signals:
    void settingsChanged(int maxSize, int maxFps, int videoBitRate);

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

    int m_originalMaxSize;
    int m_originalMaxFps;
    int m_originalBitRate;
};

#endif