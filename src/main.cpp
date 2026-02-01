#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include "ui/MainWindow.h"

int main(int argc, char* argv[]) {
    // Enable high DPI scaling
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    
    QApplication app(argc, argv);
    
    // Set application info
    app.setApplicationName("AccessibilityGamingAssistant");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("AccessibilityGaming");
    app.setOrganizationDomain("accessibilitygaming.app");
    
    // Ensure app data directory exists
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(appDataPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    // Create and show main window
    MainWindow window;
    window.show();
    
    return app.exec();
}
