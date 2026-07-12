#ifndef INSYNIC_PROFILE_MANAGER_H
#define INSYNIC_PROFILE_MANAGER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QStringList>
#include "insynic_virtualkey.h"

class InsynicProfileManager : public QObject
{
    Q_OBJECT

public:
    explicit InsynicProfileManager(QObject *parent = nullptr);
    ~InsynicProfileManager();
    
    QStringList getProfileNames() const;
    bool saveProfile(const Profile &profile);
    bool loadProfile(const QString &name, Profile &profile);
    bool deleteProfile(const QString &name);
    
private:
    QString getProfilesDir() const;
    QString getProfilePath(const QString &name) const;
};

#endif