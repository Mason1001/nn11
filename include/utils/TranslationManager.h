#ifndef TRANSLATIONMANAGER_H
#define TRANSLATIONMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QJsonObject>

class TranslationManager : public QObject {
    Q_OBJECT

public:
    explicit TranslationManager(QObject* parent = nullptr);
    ~TranslationManager();

    // Language management
    void setLanguage(const QString& languageCode);
    QString getCurrentLanguage() const;
    QStringList getAvailableLanguages() const;
    QString getLanguageName(const QString& code) const;

    // Translation
    QString translate(const QString& key) const;
    QString tr(const QString& key) const; // Alias for translate

    // Check if RTL
    bool isRTL() const;

signals:
    void languageChanged(const QString& languageCode);

private:
    QString m_currentLanguage;
    QJsonObject m_translations;
    QMap<QString, QString> m_languageNames;
    QStringList m_rtlLanguages;

    void loadTranslations();
    void initLanguageNames();
    QJsonObject getEnglishTranslations() const;
    QJsonObject getArabicTranslations() const;
};

#endif // TRANSLATIONMANAGER_H
