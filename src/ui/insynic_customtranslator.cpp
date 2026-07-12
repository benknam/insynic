#include "insynic_customtranslator.h"

#include <QFile>
#include <QXmlStreamReader>

InsynicCustomTranslator::InsynicCustomTranslator(QObject *parent)
    : QTranslator(parent)
{
}

InsynicCustomTranslator::~InsynicCustomTranslator()
{
}

bool InsynicCustomTranslator::load(const QString &filename)
{
    m_translations.clear();
    
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QXmlStreamReader reader(&file);
    QString currentContext;
    QString currentSource;
    QString currentTranslation;
    
    while (!reader.atEnd()) {
        QXmlStreamReader::TokenType token = reader.readNext();
        
        if (token == QXmlStreamReader::StartElement) {
            if (reader.name() == "context") {
                currentContext.clear();
            } else if (reader.name() == "name") {
                if (currentContext.isEmpty()) {
                    currentContext = reader.readElementText();
                }
            } else if (reader.name() == "source") {
                currentSource = reader.readElementText();
            } else if (reader.name() == "translation") {
                currentTranslation = reader.readElementText();
                if (!currentSource.isEmpty() && !currentTranslation.isEmpty()) {
                    QString key = currentContext + "\x00" + currentSource;
                    m_translations[key] = currentTranslation;
                }
                currentSource.clear();
                currentTranslation.clear();
            }
        }
    }
    
    file.close();
    
    return !m_translations.isEmpty();
}

QString InsynicCustomTranslator::translate(const char *context, const char *sourceText, const char *disambiguation, int n) const
{
    Q_UNUSED(disambiguation);
    Q_UNUSED(n);
    
    QString key = QString::fromUtf8(context) + "\x00" + QString::fromUtf8(sourceText);
    if (m_translations.contains(key)) {
        return m_translations[key];
    }
    
    return QString();
}