#include "insynic_filemanager.h"

#include <QProcess>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QDebug>

InsynicFileManager::InsynicFileManager(QObject *parent)
    : QObject(parent)
    , m_adbPath("adb")
{
}

InsynicFileManager::~InsynicFileManager()
{
}

void
InsynicFileManager::setAdbPath(const QString &adbPath)
{
    m_adbPath = adbPath;
}

void
InsynicFileManager::setSerial(const QString &serial)
{
    m_serial = serial;
}

QString
InsynicFileManager::runAdb(const QStringList &args, bool *ok, int timeoutMs)
{
    QStringList fullArgs;
    if (!m_serial.isEmpty()) {
        fullArgs << "-s" << m_serial;
    }
    fullArgs << args;

    QProcess process;
    process.start(m_adbPath, fullArgs);

    if (!process.waitForStarted(timeoutMs)) {
        if (ok) *ok = false;
        return QString();
    }

    if (!process.waitForFinished(timeoutMs)) {
        process.kill();
        process.waitForFinished();
        if (ok) *ok = false;
        return QString();
    }

    if (ok) {
        *ok = (process.exitCode() == 0);
    }

    QString output = QString::fromUtf8(process.readAllStandardOutput());
    QString error = QString::fromUtf8(process.readAllStandardError());
    if (!error.isEmpty()) {
        qWarning() << "adb error:" << error;
    }

    return output;
}

QStringList
InsynicFileManager::listDevices()
{
    bool ok;
    QString output = runAdb(QStringList() << "devices", &ok);
    if (!ok) {
        return QStringList();
    }

    QStringList devices;
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    for (int i = 1; i < lines.size(); ++i) {
        QString line = lines[i].trimmed();
        if (line.endsWith("device") || line.endsWith("unauthorized")
            || line.endsWith("offline")) {
            QString serial = line.section('\t', 0, 0).trimmed();
            if (!serial.isEmpty()) {
                devices << serial;
            }
        }
    }
    return devices;
}

bool
InsynicFileManager::connectDevice(const QString &ip, int port)
{
    QString address = QString("%1:%2").arg(ip).arg(port);
    bool ok;
    QString output = runAdb(QStringList() << "connect" << address, &ok);
    if (!ok) {
        return false;
    }
    return output.contains("connected") || output.contains("already connected");
}

static QString
normalizeRemotePath(const QString &path)
{
    QString p = path;
    if (p.isEmpty()) {
        return "/";
    }
    if (!p.startsWith('/')) {
        p.prepend('/');
    }
    while (p.endsWith('/') && p.length() > 1) {
        p.chop(1);
    }
    return p;
}

QVector<AdbFileInfo>
InsynicFileManager::listFiles(const QString &remotePath, bool *ok)
{
    QVector<AdbFileInfo> result;
    QString path = normalizeRemotePath(remotePath);

    bool readLinkOk;
    QString realPathOutput = runAdb(
        QStringList() << "shell" << "readlink" << "-f" << path, &readLinkOk);
    QString resolvedPath = readLinkOk && !realPathOutput.trimmed().isEmpty() 
        ? realPathOutput.trimmed() 
        : path;

    QString output = runAdb(
        QStringList() << "shell" << "ls" << "-la" << resolvedPath, ok);
    if (!ok || output.isEmpty()) {
        return result;
    }

    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        if (line.startsWith("total ")) {
            continue;
        }
        if (line.trimmed().isEmpty()) {
            continue;
        }

        QRegularExpression re(
            "^([dlcb-][rwx-]+)\\s+\\d*\\s*"
            "([^\\s]+)\\s+([^\\s]+)\\s+"
            "([0-9]+)?\\s*"
            "([0-9]{4}-[0-9]{2}-[0-9]{2}\\s+[0-9]{2}:[0-9]{2})\\s+"
            "(.+)$"
        );
        QRegularExpressionMatch match = re.match(line.trimmed());
        if (!match.hasMatch()) {
            continue;
        }

        AdbFileInfo info;
        info.permissions = match.captured(1);
        info.owner = match.captured(2);
        info.group = match.captured(3);
        info.size = match.captured(4).toLongLong();
        info.date = match.captured(5);
        info.name = match.captured(6);

        info.isLink = info.permissions.startsWith('l');
        
        QString targetPath = path + "/" + info.name;
        if (info.isLink) {
            int arrowIdx = info.name.indexOf(" -> ");
            if (arrowIdx > 0) {
                info.name = info.name.left(arrowIdx);
                targetPath = path + "/" + info.name;
            }
            
            bool isDirResult;
            QString testCmd = QString("test -d %1 && echo 1 || echo 0").arg(targetPath);
            QString dirCheck = runAdb(QStringList() << "shell" << testCmd, &isDirResult);
            info.isDir = isDirResult && dirCheck.trimmed() == "1";
        } else {
            info.isDir = info.permissions.startsWith('d');
        }

        if (path == "/") {
            info.path = "/" + info.name;
        } else {
            info.path = path + "/" + info.name;
        }

        if (info.name == "." || info.name == "..") {
            continue;
        }

        result.append(info);
    }

    std::sort(result.begin(), result.end(),
              [](const AdbFileInfo &a, const AdbFileInfo &b) {
        if (a.isDir != b.isDir) {
            return a.isDir;
        }
        return a.name < b.name;
    });

    return result;
}

bool
InsynicFileManager::pushFile(const QString &localPath,
                             const QString &remotePath)
{
    bool ok;
    runAdb(QStringList() << "push" << localPath << remotePath, &ok, 300000);
    emit operationFinished(ok,
        ok ? "Push completed" : "Push failed");
    return ok;
}

bool
InsynicFileManager::pullFile(const QString &remotePath,
                             const QString &localPath)
{
    bool ok;
    runAdb(QStringList() << "pull" << remotePath << localPath, &ok, 300000);
    emit operationFinished(ok,
        ok ? "Pull completed" : "Pull failed");
    return ok;
}

bool
InsynicFileManager::deleteFile(const QString &remotePath)
{
    bool ok;
    runAdb(QStringList() << "shell" << "rm" << "-rf" << remotePath, &ok);
    return ok;
}

bool
InsynicFileManager::mkdir(const QString &remotePath)
{
    bool ok;
    runAdb(QStringList() << "shell" << "mkdir" << "-p" << remotePath, &ok);
    return ok;
}

bool
InsynicFileManager::rename(const QString &remoteOldPath,
                           const QString &remoteNewPath)
{
    bool ok;
    runAdb(QStringList() << "shell" << "mv" << remoteOldPath << remoteNewPath,
           &ok);
    return ok;
}
