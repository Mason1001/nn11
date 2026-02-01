#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QString>
#include <QColor>
#include <QVariant>
#include <QJsonObject>
#include <QByteArray>
#include <map>

class ConfigManager : public QObject {
    Q_OBJECT

public:
    explicit ConfigManager(QObject* parent = nullptr);
    ~ConfigManager();

    // Load/Save
    bool load();
    bool save();
    void reset();

    // Generic getters/setters
    QVariant getValue(const QString& key, const QVariant& defaultValue = QVariant()) const;
    void setValue(const QString& key, const QVariant& value);

    // Detection settings
    int getAimAssistStrength() const;
    void setAimAssistStrength(int value);

    int getResponseSpeed() const;
    void setResponseSpeed(int value);

    int getFOVRadius() const;
    void setFOVRadius(int value);

    QColor getTargetColor() const;
    void setTargetColor(const QColor& color);

    int getColorTolerance() const;
    void setColorTolerance(int value);

    // Visual settings
    bool isOverlayEnabled() const;
    void setOverlayEnabled(bool enabled);

    bool isFOVCircleVisible() const;
    void setFOVCircleVisible(bool visible);

    bool isCrosshairVisible() const;
    void setCrosshairVisible(bool visible);

    // General settings
    QString getLanguage() const;
    void setLanguage(const QString& language);

    int getActiveMonitor() const;
    void setActiveMonitor(int monitor);

    bool isMinimizeToTray() const;
    void setMinimizeToTray(bool minimize);

    bool isStartMinimized() const;
    void setStartMinimized(bool minimized);

    // Hotkeys
    QString getToggleHotkey() const;
    void setToggleHotkey(const QString& hotkey);

    // Encryption
    void setEncryptionEnabled(bool enabled);
    bool isEncryptionEnabled() const;

signals:
    void configLoaded();
    void configSaved();
    void settingChanged(const QString& key, const QVariant& value);

private:
    QString m_configPath;
    QJsonObject m_config;
    bool m_encryptionEnabled;
    QString m_encryptionKey;

    QString getConfigFilePath() const;
    QString encrypt(const QString& data) const;
    QString decrypt(const QString& data) const;
    void setDefaults();
};

#endif // CONFIGMANAGER_H
