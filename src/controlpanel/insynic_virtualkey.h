#ifndef INSYNIC_VIRTUALKEY_H
#define INSYNIC_VIRTUALKEY_H

#include <QString>
#include <QPoint>
#include <QList>
#include <QKeySequence>

struct VirtualKey {
    int keyCode;
    QString keyName;
    double relX;
    double relY;
    int size;
    
    VirtualKey() : keyCode(0), relX(0.5), relY(0.5), size(40) {}
    VirtualKey(int code, const QString &name, double rx, double ry, int s) 
        : keyCode(code), keyName(name), relX(rx), relY(ry), size(s) {}
    
    bool isValid() const { return keyCode != 0 && relX >= 0 && relY >= 0; }
    
    int getScreenX(int screenWidth) const { return static_cast<int>(relX * screenWidth); }
    int getScreenY(int screenHeight) const { return static_cast<int>(relY * screenHeight); }
};

struct Profile {
    QString name;
    QList<VirtualKey> keys;
    int windowX;
    int windowY;
    int windowWidth;
    int windowHeight;
    
    Profile() : windowX(0), windowY(0), windowWidth(0), windowHeight(0) {}
    Profile(const QString &n) : name(n), windowX(0), windowY(0), windowWidth(0), windowHeight(0) {}
};

#endif