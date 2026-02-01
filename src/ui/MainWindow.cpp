#include "ui/MainWindow.h"
#include "ui/ColorPicker.h"
#include "ui/AdvancedColorPicker.h"
#include <QApplication>
#include <QMessageBox>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_tracker(std::make_unique<Tracker>())
    , m_overlay(std::make_unique<Overlay>())
    , m_configManager(std::make_unique<ConfigManager>())
    , m_translationManager(std::make_unique<TranslationManager>())
    , m_statsTracker(std::make_unique<StatsTracker>())
    , m_isRunning(false)
    , m_selectedColor(Qt::red)
{
    setWindowTitle("Accessibility Gaming Assistant");
    setMinimumSize(500, 600);
    resize(550, 700);
    
    applyDarkTheme();
    setupUI();
    setupTrayIcon();
    setupConnections();
    setupHotkeys();
    
    loadSettings();
    updateUILanguage();
}

MainWindow::~MainWindow() {
    saveSettings();
}

void MainWindow::applyDarkTheme() {
    QString style = R"(
        QMainWindow, QWidget {
            background-color: #1e1e1e;
            color: #ffffff;
            font-family: 'Segoe UI', sans-serif;
        }
        
        QGroupBox {
            border: 1px solid #3e3e42;
            border-radius: 5px;
            margin-top: 10px;
            padding-top: 10px;
            font-weight: bold;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px;
        }
        
        QPushButton {
            background-color: #0e639c;
            border: none;
            border-radius: 4px;
            padding: 8px 16px;
            color: white;
            font-weight: bold;
        }
        
        QPushButton:hover {
            background-color: #1177bb;
        }
        
        QPushButton:pressed {
            background-color: #094771;
        }
        
        QPushButton:disabled {
            background-color: #3e3e42;
            color: #808080;
        }
        
        QSlider::groove:horizontal {
            border: 1px solid #3e3e42;
            height: 6px;
            background: #2d2d30;
            border-radius: 3px;
        }
        
        QSlider::handle:horizontal {
            background: #007acc;
            border: none;
            width: 16px;
            margin: -5px 0;
            border-radius: 8px;
        }
        
        QSlider::handle:horizontal:hover {
            background: #1e90ff;
        }
        
        QComboBox {
            background-color: #2d2d30;
            border: 1px solid #3e3e42;
            border-radius: 4px;
            padding: 5px 10px;
            color: white;
        }
        
        QComboBox::drop-down {
            border: none;
            width: 20px;
        }
        
        QComboBox QAbstractItemView {
            background-color: #2d2d30;
            border: 1px solid #3e3e42;
            selection-background-color: #007acc;
        }
        
        QCheckBox {
            spacing: 8px;
        }
        
        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border-radius: 3px;
            border: 1px solid #3e3e42;
            background-color: #2d2d30;
        }
        
        QCheckBox::indicator:checked {
            background-color: #007acc;
            border-color: #007acc;
        }
        
        QTabWidget::pane {
            border: 1px solid #3e3e42;
            border-radius: 4px;
        }
        
        QTabBar::tab {
            background-color: #2d2d30;
            border: 1px solid #3e3e42;
            padding: 8px 16px;
            margin-right: 2px;
        }
        
        QTabBar::tab:selected {
            background-color: #007acc;
        }
        
        QLabel {
            color: #cccccc;
        }
    )";
    
    qApp->setStyleSheet(style);
}

void MainWindow::setupUI() {
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(m_centralWidget);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Title
    QLabel* titleLabel = new QLabel("🎮 Accessibility Gaming Assistant");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #007acc;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    // Tab widget
    m_tabWidget = new QTabWidget();
    mainLayout->addWidget(m_tabWidget);
    
    setupMainTab();
    setupDetectionTab();
    setupVisualTab();
    setupSettingsTab();
    
    // Stats display at bottom
    setupStatsDisplay();
}

void MainWindow::setupMainTab() {
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);
    layout->setSpacing(15);
    
    // Control group
    QGroupBox* controlGroup = createGroupBox("⚡ Control");
    QVBoxLayout* controlLayout = new QVBoxLayout(controlGroup);
    
    m_startStopButton = new QPushButton("▶️ Start");
    m_startStopButton->setMinimumHeight(50);
    m_startStopButton->setStyleSheet("font-size: 16px;");
    controlLayout->addWidget(m_startStopButton);
    
    m_statusLabel = new QLabel("Status: Stopped");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    controlLayout->addWidget(m_statusLabel);
    
    m_fpsLabel = new QLabel("FPS: 0");
    m_fpsLabel->setAlignment(Qt::AlignCenter);
    m_fpsLabel->setStyleSheet("font-size: 14px; color: #00ff00;");
    controlLayout->addWidget(m_fpsLabel);
    
    layout->addWidget(controlGroup);
    
    // Quick settings
    QGroupBox* quickGroup = createGroupBox("⚙️ Quick Settings");
    QGridLayout* quickLayout = new QGridLayout(quickGroup);
    
    quickLayout->addWidget(new QLabel("Aim Assist:"), 0, 0);
    m_aimAssistSlider = new QSlider(Qt::Horizontal);
    m_aimAssistSlider->setRange(0, 100);
    m_aimAssistSlider->setValue(30);
    quickLayout->addWidget(m_aimAssistSlider, 0, 1);
    m_aimAssistLabel = new QLabel("30%");
    m_aimAssistLabel->setMinimumWidth(50);
    quickLayout->addWidget(m_aimAssistLabel, 0, 2);
    
    quickLayout->addWidget(new QLabel("Response:"), 1, 0);
    m_responseSpeedSlider = new QSlider(Qt::Horizontal);
    m_responseSpeedSlider->setRange(0, 100);
    m_responseSpeedSlider->setValue(50);
    quickLayout->addWidget(m_responseSpeedSlider, 1, 1);
    m_responseSpeedLabel = new QLabel("50%");
    m_responseSpeedLabel->setMinimumWidth(50);
    quickLayout->addWidget(m_responseSpeedLabel, 1, 2);
    
    layout->addWidget(quickGroup);
    layout->addStretch();
    
    m_tabWidget->addTab(tab, "🏠 Main");
}

void MainWindow::setupDetectionTab() {
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);
    layout->setSpacing(15);
    
    // Color settings
    QGroupBox* colorGroup = createGroupBox("🎨 Color Detection");
    QVBoxLayout* colorLayout = new QVBoxLayout(colorGroup);
    
    QHBoxLayout* colorSelectLayout = new QHBoxLayout();
    colorSelectLayout->addWidget(new QLabel("Target Color:"));
    m_colorPickerButton = new QPushButton("Select Color");
    m_colorPickerButton->setMinimumWidth(120);
    colorSelectLayout->addWidget(m_colorPickerButton);
    m_selectedColorLabel = new QLabel();
    m_selectedColorLabel->setFixedSize(30, 30);
    m_selectedColorLabel->setStyleSheet("background-color: red; border: 2px solid #3e3e42; border-radius: 4px;");
    colorSelectLayout->addWidget(m_selectedColorLabel);
    colorSelectLayout->addStretch();
    colorLayout->addLayout(colorSelectLayout);
    
    QHBoxLayout* toleranceLayout = new QHBoxLayout();
    toleranceLayout->addWidget(new QLabel("Tolerance:"));
    m_toleranceSlider = new QSlider(Qt::Horizontal);
    m_toleranceSlider->setRange(0, 100);
    m_toleranceSlider->setValue(30);
    toleranceLayout->addWidget(m_toleranceSlider);
    m_toleranceLabel = new QLabel("30");
    m_toleranceLabel->setMinimumWidth(40);
    toleranceLayout->addWidget(m_toleranceLabel);
    colorLayout->addLayout(toleranceLayout);
    
    layout->addWidget(colorGroup);
    
    // FOV settings
    QGroupBox* fovGroup = createGroupBox("🎯 FOV Settings");
    QVBoxLayout* fovLayout = new QVBoxLayout(fovGroup);
    
    QHBoxLayout* fovSizeLayout = new QHBoxLayout();
    fovSizeLayout->addWidget(new QLabel("FOV Radius:"));
    m_fovSlider = new QSlider(Qt::Horizontal);
    m_fovSlider->setRange(50, 500);
    m_fovSlider->setValue(150);
    fovSizeLayout->addWidget(m_fovSlider);
    m_fovLabel = new QLabel("150px");
    m_fovLabel->setMinimumWidth(60);
    fovSizeLayout->addWidget(m_fovLabel);
    fovLayout->addLayout(fovSizeLayout);
    
    layout->addWidget(fovGroup);
    layout->addStretch();
    
    m_tabWidget->addTab(tab, "🔍 Detection");
}

void MainWindow::setupVisualTab() {
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);
    layout->setSpacing(15);
    
    // Overlay settings
    QGroupBox* overlayGroup = createGroupBox("🖼️ Overlay");
    QVBoxLayout* overlayLayout = new QVBoxLayout(overlayGroup);
    
    m_overlayCheckbox = new QCheckBox("Enable Overlay");
    m_overlayCheckbox->setChecked(true);
    overlayLayout->addWidget(m_overlayCheckbox);
    
    m_fovCircleCheckbox = new QCheckBox("Show FOV Circle");
    m_fovCircleCheckbox->setChecked(true);
    overlayLayout->addWidget(m_fovCircleCheckbox);
    
    m_crosshairCheckbox = new QCheckBox("Show Crosshair");
    m_crosshairCheckbox->setChecked(false);
    overlayLayout->addWidget(m_crosshairCheckbox);
    
    layout->addWidget(overlayGroup);
    
    // Monitor selection
    QGroupBox* monitorGroup = createGroupBox("🖥️ Monitor");
    QVBoxLayout* monitorLayout = new QVBoxLayout(monitorGroup);
    
    QHBoxLayout* monitorSelectLayout = new QHBoxLayout();
    monitorSelectLayout->addWidget(new QLabel("Active Monitor:"));
    m_monitorCombo = new QComboBox();
    
    auto monitors = m_tracker->screenCapture()->getMonitors();
    for (const auto& monitor : monitors) {
        QString text = QString("%1 (%2x%3)%4")
            .arg(monitor.name)
            .arg(monitor.geometry.width())
            .arg(monitor.geometry.height())
            .arg(monitor.isPrimary ? " [Primary]" : "");
        m_monitorCombo->addItem(text, monitor.index);
    }
    
    monitorSelectLayout->addWidget(m_monitorCombo);
    monitorLayout->addLayout(monitorSelectLayout);
    
    layout->addWidget(monitorGroup);
    layout->addStretch();
    
    m_tabWidget->addTab(tab, "👁️ Visual");
}

void MainWindow::setupSettingsTab() {
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);
    layout->setSpacing(15);
    
    // Language settings
    QGroupBox* langGroup = createGroupBox("🌍 Language");
    QVBoxLayout* langLayout = new QVBoxLayout(langGroup);
    
    QHBoxLayout* langSelectLayout = new QHBoxLayout();
    langSelectLayout->addWidget(new QLabel("Language:"));
    m_languageCombo = new QComboBox();
    m_languageCombo->addItem("English", "en");
    m_languageCombo->addItem("العربية", "ar");
    langSelectLayout->addWidget(m_languageCombo);
    langSelectLayout->addStretch();
    langLayout->addLayout(langSelectLayout);
    
    layout->addWidget(langGroup);
    
    // About
    QGroupBox* aboutGroup = createGroupBox("ℹ️ About");
    QVBoxLayout* aboutLayout = new QVBoxLayout(aboutGroup);
    
    QLabel* versionLabel = new QLabel("Version 1.0.0");
    versionLabel->setAlignment(Qt::AlignCenter);
    aboutLayout->addWidget(versionLabel);
    
    QLabel* descLabel = new QLabel("Accessibility Gaming Assistant\nHelping gamers with disabilities play better.");
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setWordWrap(true);
    aboutLayout->addWidget(descLabel);
    
    layout->addWidget(aboutGroup);
    layout->addStretch();
    
    m_tabWidget->addTab(tab, "⚙️ Settings");
}

void MainWindow::setupStatsDisplay() {
    QGroupBox* statsGroup = createGroupBox("📊 Statistics");
    QHBoxLayout* statsLayout = new QHBoxLayout(statsGroup);
    
    m_targetsLabel = new QLabel("Targets: 0");
    statsLayout->addWidget(m_targetsLabel);
    
    m_assistsLabel = new QLabel("Assists: 0");
    statsLayout->addWidget(m_assistsLabel);
    
    m_runTimeLabel = new QLabel("Time: 00:00:00");
    statsLayout->addWidget(m_runTimeLabel);
    
    qobject_cast<QVBoxLayout*>(m_centralWidget->layout())->addWidget(statsGroup);
}

void MainWindow::setupTrayIcon() {
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setToolTip("Accessibility Gaming Assistant");
    
    m_trayMenu = new QMenu(this);
    
    QAction* showAction = m_trayMenu->addAction("Show/Hide");
    connect(showAction, &QAction::triggered, this, &MainWindow::onShowHideAction);
    
    m_trayMenu->addSeparator();
    
    QAction* quitAction = m_trayMenu->addAction("Quit");
    connect(quitAction, &QAction::triggered, this, &MainWindow::onQuitAction);
    
    m_trayIcon->setContextMenu(m_trayMenu);
    
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onTrayActivated);
    
    m_trayIcon->show();
}

void MainWindow::setupConnections() {
    // Start/Stop button
    connect(m_startStopButton, &QPushButton::clicked, this, &MainWindow::onStartStopClicked);
    
    // Sliders
    connect(m_aimAssistSlider, &QSlider::valueChanged, this, &MainWindow::onAimAssistChanged);
    connect(m_responseSpeedSlider, &QSlider::valueChanged, this, &MainWindow::onResponseSpeedChanged);
    connect(m_fovSlider, &QSlider::valueChanged, this, &MainWindow::onFOVChanged);
    connect(m_toleranceSlider, &QSlider::valueChanged, this, &MainWindow::onToleranceChanged);
    
    // Color picker
    connect(m_colorPickerButton, &QPushButton::clicked, [this]() {
        AdvancedColorPicker picker(this);
        picker.setColor(m_selectedColor);
        if (picker.exec() == QDialog::Accepted) {
            onColorSelected(picker.getSelectedColor());
        }
    });
    
    // Checkboxes
    connect(m_overlayCheckbox, &QCheckBox::toggled, [this](bool checked) {
        m_overlay->setOverlayEnabled(checked);
    });
    
    connect(m_fovCircleCheckbox, &QCheckBox::toggled, [this](bool checked) {
        m_overlay->setFOVVisible(checked);
    });
    
    connect(m_crosshairCheckbox, &QCheckBox::toggled, [this](bool checked) {
        m_overlay->setCrosshairVisible(checked);
    });
    
    // Combos
    connect(m_monitorCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::onMonitorChanged);
    connect(m_languageCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::onLanguageChanged);
    
    // Tracker signals
    connect(m_tracker.get(), &Tracker::statsUpdated, this, &MainWindow::onStatsUpdated);
}

void MainWindow::setupHotkeys() {
    // Platform-specific hotkey setup would go here
    // For simplicity, we're not implementing global hotkeys in this version
}

void MainWindow::onStartStopClicked() {
    if (m_isRunning) {
        m_tracker->stop();
        m_statsTracker->endSession();
        m_isRunning = false;
        m_startStopButton->setText("▶️ Start");
        m_statusLabel->setText("Status: Stopped");
        m_statusLabel->setStyleSheet("color: #ff6666;");
    } else {
        m_tracker->start();
        m_statsTracker->startSession();
        m_isRunning = true;
        m_startStopButton->setText("⏹️ Stop");
        m_statusLabel->setText("Status: Running");
        m_statusLabel->setStyleSheet("color: #66ff66;");
        
        if (m_overlayCheckbox->isChecked()) {
            m_overlay->show();
        }
    }
}

void MainWindow::onToggleOverlay() {
    bool visible = m_overlay->isVisible();
    m_overlay->setVisible(!visible);
    m_overlayCheckbox->setChecked(!visible);
}

void MainWindow::onAimAssistChanged(int value) {
    m_aimAssistLabel->setText(QString("%1%").arg(value));
    m_tracker->mouseController()->setAimAssistStrength(value);
}

void MainWindow::onResponseSpeedChanged(int value) {
    m_responseSpeedLabel->setText(QString("%1%").arg(value));
    m_tracker->mouseController()->setResponseSpeed(value);
}

void MainWindow::onFOVChanged(int value) {
    m_fovLabel->setText(QString("%1px").arg(value));
    m_tracker->colorDetection()->setFOVRadius(value);
    m_overlay->setFOVRadius(value);
}

void MainWindow::onColorSelected(const QColor& color) {
    m_selectedColor = color;
    m_selectedColorLabel->setStyleSheet(
        QString("background-color: %1; border: 2px solid #3e3e42; border-radius: 4px;")
            .arg(color.name()));
    m_tracker->colorDetection()->setTargetColor(color);
}

void MainWindow::onToleranceChanged(int value) {
    m_toleranceLabel->setText(QString::number(value));
    m_tracker->colorDetection()->setColorTolerance(value);
}

void MainWindow::onMonitorChanged(int index) {
    int monitorIndex = m_monitorCombo->itemData(index).toInt();
    m_tracker->screenCapture()->setActiveMonitor(monitorIndex);
}

void MainWindow::onLanguageChanged(int index) {
    QString langCode = m_languageCombo->itemData(index).toString();
    m_translationManager->setLanguage(langCode);
    updateUILanguage();
    
    // Handle RTL for Arabic
    if (m_translationManager->isRTL()) {
        setLayoutDirection(Qt::RightToLeft);
    } else {
        setLayoutDirection(Qt::LeftToRight);
    }
}

void MainWindow::onStatsUpdated(double fps, int targets, int assists) {
    m_fpsLabel->setText(QString("FPS: %1").arg(static_cast<int>(fps)));
    m_targetsLabel->setText(QString("Targets: %1").arg(targets));
    m_assistsLabel->setText(QString("Assists: %1").arg(assists));
    
    qint64 runTime = m_tracker->getRunningTimeMs();
    int hours = runTime / 3600000;
    int minutes = (runTime % 3600000) / 60000;
    int seconds = (runTime % 60000) / 1000;
    m_runTimeLabel->setText(QString("Time: %1:%2:%3")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0')));
}

void MainWindow::onTrayActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::DoubleClick) {
        onShowHideAction();
    }
}

void MainWindow::onShowHideAction() {
    if (isVisible()) {
        hide();
    } else {
        show();
        activateWindow();
    }
}

void MainWindow::onQuitAction() {
    saveSettings();
    qApp->quit();
}

void MainWindow::onHotkeyPressed() {
    onStartStopClicked();
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (m_configManager->isMinimizeToTray()) {
        hide();
        event->ignore();
    } else {
        saveSettings();
        event->accept();
    }
}

void MainWindow::changeEvent(QEvent* event) {
    if (event->type() == QEvent::WindowStateChange) {
        if (isMinimized() && m_configManager->isMinimizeToTray()) {
            hide();
        }
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::loadSettings() {
    m_configManager->load();
    
    m_aimAssistSlider->setValue(m_configManager->getAimAssistStrength());
    m_responseSpeedSlider->setValue(m_configManager->getResponseSpeed());
    m_fovSlider->setValue(m_configManager->getFOVRadius());
    m_toleranceSlider->setValue(m_configManager->getColorTolerance());
    
    QColor color = m_configManager->getTargetColor();
    onColorSelected(color);
    
    m_overlayCheckbox->setChecked(m_configManager->isOverlayEnabled());
    m_fovCircleCheckbox->setChecked(m_configManager->isFOVCircleVisible());
    m_crosshairCheckbox->setChecked(m_configManager->isCrosshairVisible());
    
    QString lang = m_configManager->getLanguage();
    int langIndex = m_languageCombo->findData(lang);
    if (langIndex >= 0) {
        m_languageCombo->setCurrentIndex(langIndex);
    }
}

void MainWindow::saveSettings() {
    m_configManager->setAimAssistStrength(m_aimAssistSlider->value());
    m_configManager->setResponseSpeed(m_responseSpeedSlider->value());
    m_configManager->setFOVRadius(m_fovSlider->value());
    m_configManager->setColorTolerance(m_toleranceSlider->value());
    m_configManager->setTargetColor(m_selectedColor);
    
    m_configManager->setOverlayEnabled(m_overlayCheckbox->isChecked());
    m_configManager->setFOVCircleVisible(m_fovCircleCheckbox->isChecked());
    m_configManager->setCrosshairVisible(m_crosshairCheckbox->isChecked());
    
    m_configManager->setLanguage(m_languageCombo->currentData().toString());
    
    m_configManager->save();
}

void MainWindow::updateUILanguage() {
    // Update all translatable UI text
    setWindowTitle(m_translationManager->tr("app_title"));
    // Additional translations would be applied here
}

void MainWindow::updateStatus(const QString& status) {
    m_statusLabel->setText(status);
}

QGroupBox* MainWindow::createGroupBox(const QString& title) {
    QGroupBox* box = new QGroupBox(title);
    box->setStyleSheet("QGroupBox { font-size: 13px; }");
    return box;
}
