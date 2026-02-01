#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QCloseEvent>
#include <QColor>
#include <QString>
#include <memory>

#include "core/Tracker.h"
#include "core/Overlay.h"
#include "utils/ConfigManager.h"
#include "utils/TranslationManager.h"
#include "utils/StatsTracker.h"

class ColorPicker;
class AdvancedColorPicker;
class SettingsPanel;
class StatsPanel;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;
    void changeEvent(QEvent* event) override;

private slots:
    // Control slots
    void onStartStopClicked();
    void onToggleOverlay();

    // Settings slots
    void onAimAssistChanged(int value);
    void onResponseSpeedChanged(int value);
    void onFOVChanged(int value);
    void onColorSelected(const QColor& color);
    void onToleranceChanged(int value);

    // Monitor slots
    void onMonitorChanged(int index);

    // Language slots
    void onLanguageChanged(int index);

    // Stats slots
    void onStatsUpdated(double fps, int targets, int assists);

    // Tray slots
    void onTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void onShowHideAction();
    void onQuitAction();

    // Hotkey slots
    void onHotkeyPressed();

private:
    // Core components
    std::unique_ptr<Tracker> m_tracker;
    std::unique_ptr<Overlay> m_overlay;
    std::unique_ptr<ConfigManager> m_configManager;
    std::unique_ptr<TranslationManager> m_translationManager;
    std::unique_ptr<StatsTracker> m_statsTracker;

    // UI Components
    QWidget* m_centralWidget;
    QTabWidget* m_tabWidget;

    // Main tab
    QPushButton* m_startStopButton;
    QLabel* m_statusLabel;
    QLabel* m_fpsLabel;

    // Detection tab
    QSlider* m_aimAssistSlider;
    QLabel* m_aimAssistLabel;
    QSlider* m_responseSpeedSlider;
    QLabel* m_responseSpeedLabel;
    QSlider* m_fovSlider;
    QLabel* m_fovLabel;
    QPushButton* m_colorPickerButton;
    QLabel* m_selectedColorLabel;
    QSlider* m_toleranceSlider;
    QLabel* m_toleranceLabel;

    // Visual tab
    QCheckBox* m_overlayCheckbox;
    QCheckBox* m_fovCircleCheckbox;
    QCheckBox* m_crosshairCheckbox;
    QComboBox* m_monitorCombo;

    // Settings tab
    QComboBox* m_languageCombo;

    // Stats display
    QLabel* m_targetsLabel;
    QLabel* m_assistsLabel;
    QLabel* m_runTimeLabel;

    // System tray
    QSystemTrayIcon* m_trayIcon;
    QMenu* m_trayMenu;

    // Color picker dialogs
    ColorPicker* m_colorPicker;
    AdvancedColorPicker* m_advancedColorPicker;

    // State
    bool m_isRunning;
    QColor m_selectedColor;

    // Setup methods
    void setupUI();
    void setupMainTab();
    void setupDetectionTab();
    void setupVisualTab();
    void setupSettingsTab();
    void setupStatsDisplay();
    void setupTrayIcon();
    void setupConnections();
    void setupHotkeys();

    // Helper methods
    void applyDarkTheme();
    void loadSettings();
    void saveSettings();
    void updateUILanguage();
    void updateStatus(const QString& status);
    QGroupBox* createGroupBox(const QString& title);
};

#endif // MAINWINDOW_H
