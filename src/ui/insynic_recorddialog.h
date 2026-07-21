#ifndef INSYNIC_RECORD_DIALOG_H
#define INSYNIC_RECORD_DIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class InsynicRecordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InsynicRecordDialog(QWidget *parent = nullptr);

    bool recordEnabled() const;
    QString filePath() const;
    int format() const;       // 0=auto,1=mp4,2=mkv,3=m4a,4=mka,5=opus,6=aac,7=flac,8=wav
    bool recordVideo() const;
    bool recordAudio() const;

    void setRecordEnabled(bool on);
    void setFilePath(const QString &path);
    void setFormat(int format);
    void setRecordVideo(bool on);
    void setRecordAudio(bool on);

private slots:
    void onBrowseClicked();
    void onFormatChanged(int index);
    void onRecordEnabledToggled(bool on);

private:
    void setupUi();
    QString defaultFileName() const;
    void updateControlsEnabled();

    QCheckBox *m_recordEnabledCheck;
    QGroupBox *m_fileGroup;
    QLineEdit *m_filePathEdit;
    QComboBox *m_formatCombo;
    QGroupBox *m_streamGroup;
    QCheckBox *m_videoCheck;
    QCheckBox *m_audioCheck;
    QLabel *m_hintLabel;
};

#endif
