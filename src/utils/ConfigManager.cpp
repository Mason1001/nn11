#include "utils/ConfigManager.h"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QCryptographicHash>

ConfigManager::ConfigManager(QObject* parent)
    : QObject(parent)
    , m_encryptionEnabled(false)
    , m_encryptionKey("AGA_DEFAULT_KEY_2024")
{
    m_configPath = getConfigFilePath();
    setDefaults();
}

ConfigManager::~ConfigManager() {
}

QString ConfigManager::getConfigFilePath() const {
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(appDataPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return appDataPath + "/config.json";
}

void ConfigManager::setDefaults() {
    m_config["aimAssistStrength"] = 30;
    m_config["responseSpeed"] = 50;
    m_config["fovRadius"] = 150;
    m_config["targetColor"] = "#FF0000";
    m_config["colorTolerance"] = 30;
    m_config["overlayEnabled"] = true;
    m_config["fovCircleVisible"] = true;
    m_config["crosshairVisible"] = false;
    m_config["language"] = "en";
    m_config["activeMonitor"] = 0;
    m_config["minimizeToTray"] = true;
    m_config["startMinimized"] = false;
    m_config["toggleHotkey"] = "F6";
}

bool ConfigManager::load() {
    QFile file(m_configPath);
    if (!file.exists()) {
        return false;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    if (m_encryptionEnabled) {
        data = decrypt(QString::fromUtf8(data)).toUtf8();
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        return false;
    }
    
    m_config = doc.object();
    emit configLoaded();
    return true;
}

bool ConfigManager::save() {
    QJsonDocument doc(m_config);
    QByteArray data = doc.toJson(QJsonDocument::Indented);
    
    if (m_encryptionEnabled) {
        data = encrypt(QString::fromUtf8(data)).toUtf8();
    }
    
    QFile file(m_configPath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    file.write(data);
    file.close();
    
    emit configSaved();
    return true;
}

void ConfigManager::reset() {
    setDefaults();
    save();
}

QVariant ConfigManager::getValue(const QString& key, const QVariant& defaultValue) const {
    if (m_config.contains(key)) {
        return m_config[key].toVariant();
    }
    return defaultValue;
}

void ConfigManager::setValue(const QString& key, const QVariant& value) {
    m_config[key] = QJsonValue::fromVariant(value);
    emit settingChanged(key, value);
}

int ConfigManager::getAimAssistStrength() const {
    return m_config["aimAssistStrength"].toInt(30);
}

void ConfigManager::setAimAssistStrength(int value) {
    setValue("aimAssistStrength", value);
}

int ConfigManager::getResponseSpeed() const {
    return m_config["responseSpeed"].toInt(50);
}

void ConfigManager::setResponseSpeed(int value) {
    setValue("responseSpeed", value);
}

int ConfigManager::getFOVRadius() const {
    return m_config["fovRadius"].toInt(150);
}

void ConfigManager::setFOVRadius(int value) {
    setValue("fovRadius", value);
}

QColor ConfigManager::getTargetColor() const {
    QString colorStr = m_config["targetColor"].toString("#FF0000");
    return QColor(colorStr);
}

void ConfigManager::setTargetColor(const QColor& color) {
    setValue("targetColor", color.name());
}

int ConfigManager::getColorTolerance() const {
    return m_config["colorTolerance"].toInt(30);
}

void ConfigManager::setColorTolerance(int value) {
    setValue("colorTolerance", value);
}

bool ConfigManager::isOverlayEnabled() const {
    return m_config["overlayEnabled"].toBool(true);
}

void ConfigManager::setOverlayEnabled(bool enabled) {
    setValue("overlayEnabled", enabled);
}

bool ConfigManager::isFOVCircleVisible() const {
    return m_config["fovCircleVisible"].toBool(true);
}

void ConfigManager::setFOVCircleVisible(bool visible) {
    setValue("fovCircleVisible", visible);
}

bool ConfigManager::isCrosshairVisible() const {
    return m_config["crosshairVisible"].toBool(false);
}

void ConfigManager::setCrosshairVisible(bool visible) {
    setValue("crosshairVisible", visible);
}

QString ConfigManager::getLanguage() const {
    return m_config["language"].toString("en");
}

void ConfigManager::setLanguage(const QString& language) {
    setValue("language", language);
}

int ConfigManager::getActiveMonitor() const {
    return m_config["activeMonitor"].toInt(0);
}

void ConfigManager::setActiveMonitor(int monitor) {
    setValue("activeMonitor", monitor);
}

bool ConfigManager::isMinimizeToTray() const {
    return m_config["minimizeToTray"].toBool(true);
}

void ConfigManager::setMinimizeToTray(bool minimize) {
    setValue("minimizeToTray", minimize);
}

bool ConfigManager::isStartMinimized() const {
    return m_config["startMinimized"].toBool(false);
}

void ConfigManager::setStartMinimized(bool minimized) {
    setValue("startMinimized", minimized);
}

QString ConfigManager::getToggleHotkey() const {
    return m_config["toggleHotkey"].toString("F6");
}

void ConfigManager::setToggleHotkey(const QString& hotkey) {
    setValue("toggleHotkey", hotkey);
}

void ConfigManager::setEncryptionEnabled(bool enabled) {
    m_encryptionEnabled = enabled;
}

bool ConfigManager::isEncryptionEnabled() const {
    return m_encryptionEnabled;
}

QString ConfigManager::encrypt(const QString& data) const {
    // Simple XOR encryption (for demo - use proper encryption in production)
    QByteArray key = m_encryptionKey.toUtf8();
    QByteArray input = data.toUtf8();
    QByteArray output;
    
    for (int i = 0; i < input.size(); ++i) {
        output.append(input[i] ^ key[i % key.size()]);
    }
    
    return output.toBase64();
}

QString ConfigManager::decrypt(const QString& data) const {
    QByteArray input = QByteArray::fromBase64(data.toUtf8());
    QByteArray key = m_encryptionKey.toUtf8();
    QByteArray output;
    
    for (int i = 0; i < input.size(); ++i) {
        output.append(input[i] ^ key[i % key.size()]);
    }
    
    return QString::fromUtf8(output);
}
