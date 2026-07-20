#ifndef INSYNIC_FILE_MANAGER_H
#define INSYNIC_FILE_MANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QProcess>

struct AdbFileInfo {
    QString name;
    QString path;
    QString permissions;
    QString owner;
    QString group;
    qint64 size;
    QString date;
    QString time;
    bool isDir;
    bool isLink;
};

class InsynicFileManager : public QObject
{
    Q_OBJECT

public:
    explicit InsynicFileManager(QObject *parent = nullptr);
    ~InsynicFileManager();

    void setAdbPath(const QString &adbPath);
    void setSerial(const QString &serial);

    QVector<AdbFileInfo> listFiles(const QString &remotePath, bool *ok = nullptr);
    bool pushFile(const QString &localPath, const QString &remotePath);
    bool pullFile(const QString &remotePath, const QString &localPath);
    bool deleteFile(const QString &remotePath);
    bool mkdir(const QString &remotePath);
    bool rename(const QString &remoteOldPath, const QString &remoteNewPath);

    QStringList listDevices();
    bool connectDevice(const QString &ip, int port);
    QString getDeviceName(const QString &serial);

signals:
    void operationFinished(bool success, const QString &message);
    void progress(int percent);

private:
    QString runAdb(const QStringList &args, bool *ok = nullptr,
                   int timeoutMs = 30000);

    QString m_adbPath;
    QString m_serial;
};

#endif
