#include "utils/TranslationManager.h"
#include <QJsonDocument>
#include <QJsonObject>

TranslationManager::TranslationManager(QObject* parent)
    : QObject(parent)
    , m_currentLanguage("en")
{
    initLanguageNames();
    m_rtlLanguages << "ar" << "he" << "fa" << "ur";
    loadTranslations();
}

TranslationManager::~TranslationManager() {
}

void TranslationManager::initLanguageNames() {
    m_languageNames["en"] = "English";
    m_languageNames["ar"] = "العربية";
}

void TranslationManager::setLanguage(const QString& languageCode) {
    if (m_currentLanguage != languageCode) {
        m_currentLanguage = languageCode;
        loadTranslations();
        emit languageChanged(languageCode);
    }
}

QString TranslationManager::getCurrentLanguage() const {
    return m_currentLanguage;
}

QStringList TranslationManager::getAvailableLanguages() const {
    return m_languageNames.keys();
}

QString TranslationManager::getLanguageName(const QString& code) const {
    return m_languageNames.value(code, code);
}

void TranslationManager::loadTranslations() {
    if (m_currentLanguage == "en") {
        m_translations = getEnglishTranslations();
    } else if (m_currentLanguage == "ar") {
        m_translations = getArabicTranslations();
    } else {
        m_translations = getEnglishTranslations();
    }
}

QString TranslationManager::translate(const QString& key) const {
    if (m_translations.contains(key)) {
        return m_translations[key].toString();
    }
    return key;
}

QString TranslationManager::tr(const QString& key) const {
    return translate(key);
}

bool TranslationManager::isRTL() const {
    return m_rtlLanguages.contains(m_currentLanguage);
}

QJsonObject TranslationManager::getEnglishTranslations() const {
    QJsonObject translations;
    
    // App
    translations["app_title"] = "Accessibility Gaming Assistant";
    translations["app_version"] = "Version 1.0.0";
    
    // Main tab
    translations["tab_main"] = "Main";
    translations["tab_detection"] = "Detection";
    translations["tab_visual"] = "Visual";
    translations["tab_settings"] = "Settings";
    
    // Controls
    translations["control_group"] = "Control";
    translations["btn_start"] = "Start";
    translations["btn_stop"] = "Stop";
    translations["status_stopped"] = "Status: Stopped";
    translations["status_running"] = "Status: Running";
    
    // Quick settings
    translations["quick_settings"] = "Quick Settings";
    translations["aim_assist"] = "Aim Assist";
    translations["response_speed"] = "Response Speed";
    
    // Color detection
    translations["color_detection"] = "Color Detection";
    translations["target_color"] = "Target Color";
    translations["select_color"] = "Select Color";
    translations["tolerance"] = "Tolerance";
    
    // FOV
    translations["fov_settings"] = "FOV Settings";
    translations["fov_radius"] = "FOV Radius";
    
    // Overlay
    translations["overlay"] = "Overlay";
    translations["enable_overlay"] = "Enable Overlay";
    translations["show_fov_circle"] = "Show FOV Circle";
    translations["show_crosshair"] = "Show Crosshair";
    
    // Monitor
    translations["monitor"] = "Monitor";
    translations["active_monitor"] = "Active Monitor";
    translations["primary"] = "Primary";
    
    // Language
    translations["language"] = "Language";
    
    // Stats
    translations["statistics"] = "Statistics";
    translations["targets"] = "Targets";
    translations["assists"] = "Assists";
    translations["time"] = "Time";
    translations["fps"] = "FPS";
    
    // About
    translations["about"] = "About";
    translations["about_desc"] = "Helping gamers with disabilities play better.";
    
    // Color picker
    translations["color_picker"] = "Color Picker";
    translations["pick_from_screen"] = "Pick from Screen";
    translations["current"] = "Current";
    translations["original"] = "Original";
    translations["history"] = "History";
    
    // System tray
    translations["show_hide"] = "Show/Hide";
    translations["quit"] = "Quit";
    
    return translations;
}

QJsonObject TranslationManager::getArabicTranslations() const {
    QJsonObject translations;
    
    // App
    translations["app_title"] = "مساعد الألعاب للوصول";
    translations["app_version"] = "الإصدار 1.0.0";
    
    // Main tab
    translations["tab_main"] = "الرئيسية";
    translations["tab_detection"] = "الكشف";
    translations["tab_visual"] = "المرئيات";
    translations["tab_settings"] = "الإعدادات";
    
    // Controls
    translations["control_group"] = "التحكم";
    translations["btn_start"] = "تشغيل";
    translations["btn_stop"] = "إيقاف";
    translations["status_stopped"] = "الحالة: متوقف";
    translations["status_running"] = "الحالة: يعمل";
    
    // Quick settings
    translations["quick_settings"] = "إعدادات سريعة";
    translations["aim_assist"] = "مساعد التصويب";
    translations["response_speed"] = "سرعة الاستجابة";
    
    // Color detection
    translations["color_detection"] = "كشف الألوان";
    translations["target_color"] = "لون الهدف";
    translations["select_color"] = "اختر لون";
    translations["tolerance"] = "التسامح";
    
    // FOV
    translations["fov_settings"] = "إعدادات مجال الرؤية";
    translations["fov_radius"] = "نصف قطر مجال الرؤية";
    
    // Overlay
    translations["overlay"] = "الطبقة العلوية";
    translations["enable_overlay"] = "تفعيل الطبقة العلوية";
    translations["show_fov_circle"] = "إظهار دائرة مجال الرؤية";
    translations["show_crosshair"] = "إظهار التقاطع";
    
    // Monitor
    translations["monitor"] = "الشاشة";
    translations["active_monitor"] = "الشاشة النشطة";
    translations["primary"] = "الرئيسية";
    
    // Language
    translations["language"] = "اللغة";
    
    // Stats
    translations["statistics"] = "الإحصائيات";
    translations["targets"] = "الأهداف";
    translations["assists"] = "المساعدات";
    translations["time"] = "الوقت";
    translations["fps"] = "الإطارات/ثانية";
    
    // About
    translations["about"] = "حول";
    translations["about_desc"] = "مساعدة اللاعبين ذوي الإعاقات على اللعب بشكل أفضل.";
    
    // Color picker
    translations["color_picker"] = "منتقي الألوان";
    translations["pick_from_screen"] = "اختر من الشاشة";
    translations["current"] = "الحالي";
    translations["original"] = "الأصلي";
    translations["history"] = "السجل";
    
    // System tray
    translations["show_hide"] = "إظهار/إخفاء";
    translations["quit"] = "خروج";
    
    return translations;
}
