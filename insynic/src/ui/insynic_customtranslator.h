#ifndef INSYNIC_CUSTOMTRANSLATOR_H
#define INSYNIC_CUSTOMTRANSLATOR_H

#include <QTranslator>
#include <QMap>
#include <QString>

class InsynicCustomTranslator : public QTranslator
{
    Q_OBJECT

public:
    InsynicCustomTranslator(QObject *parent = nullptr);
    ~InsynicCustomTranslator();

    bool load(const QString &filename);
    QString translate(const char *context, const char *sourceText, const char *disambiguation = nullptr, int n = -1) const override;

private:
    QMap<QString, QString> m_translations;
};

#endif