#include "insynic_profilemanager.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>

InsynicProfileManager::InsynicProfileManager(QObject *parent)
    : QObject(parent)
{
}

InsynicProfileManager::~InsynicProfileManager()
{
}

QString InsynicProfileManager::getProfilesDir() const
{
    QString dirPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dirPath);
    if (!dir.exists()) {
        dir.mkpath(dirPath);
    }
    return dirPath;
}

QString InsynicProfileManager::getProfilePath(const QString &name) const
{
    return QString("%1/%2.json").arg(getProfilesDir()).arg(name);
}

QStringList InsynicProfileManager::getProfileNames() const
{
    QStringList names;
    QDir dir(getProfilesDir());
    QStringList files = dir.entryList(QStringList() << "*.json", QDir::Files);
    
    foreach (const QString &file, files) {
        names.append(file.left(file.length() - 5));
    }
    
    return names;
}

bool InsynicProfileManager::saveProfile(const Profile &profile)
{
    QJsonObject obj;
    obj["name"] = profile.name;
    obj["windowX"] = profile.windowX;
    obj["windowY"] = profile.windowY;
    obj["windowWidth"] = profile.windowWidth;
    obj["windowHeight"] = profile.windowHeight;
    
    QJsonArray keysArray;
    foreach (const VirtualKey &key, profile.keys) {
        QJsonObject keyObj;
        keyObj["keyCode"] = key.keyCode;
        keyObj["keyName"] = key.keyName;
        keyObj["relX"] = key.relX;
        keyObj["relY"] = key.relY;
        keyObj["size"] = key.size;
        keysArray.append(keyObj);
    }
    obj["keys"] = keysArray;
    
    QJsonDocument doc(obj);
    QFile file(getProfilePath(profile.name));
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

bool InsynicProfileManager::loadProfile(const QString &name, Profile &profile)
{
    QFile file(getProfilePath(name));
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return false;
    }
    
    QJsonObject obj = doc.object();
    profile.name = obj["name"].toString();
    profile.windowX = obj["windowX"].toInt();
    profile.windowY = obj["windowY"].toInt();
    profile.windowWidth = obj["windowWidth"].toInt();
    profile.windowHeight = obj["windowHeight"].toInt();
    
    profile.keys.clear();
    QJsonArray keysArray = obj["keys"].toArray();
    foreach (const QJsonValue &val, keysArray) {
        QJsonObject keyObj = val.toObject();
        VirtualKey key;
        key.keyCode = keyObj["keyCode"].toInt();
        key.keyName = keyObj["keyName"].toString();
        key.relX = keyObj["relX"].toDouble();
        key.relY = keyObj["relY"].toDouble();
        key.size = keyObj["size"].toInt();
        profile.keys.append(key);
    }
    
    return true;
}

bool InsynicProfileManager::deleteProfile(const QString &name)
{
    QFile file(getProfilePath(name));
    return file.remove();
}