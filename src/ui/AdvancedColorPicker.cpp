#include "ui/AdvancedColorPicker.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QGuiApplication>
#include <QScreen>
#include <QCursor>
#include <QSettings>
#include <cmath>

AdvancedColorPicker::AdvancedColorPicker(QWidget* parent)
    : QDialog(parent)
    , m_selectedColor(Qt::red)
    , m_isPicking(false)
    , m_updatingUI(false)
{
    setWindowTitle("Advanced Color Picker");
    setModal(true);
    setFixedSize(500, 600);
    
    setupUI();
    loadHistory();
    updateAllDisplays();
}

AdvancedColorPicker::~AdvancedColorPicker() {
    if (m_pickerTimer && m_pickerTimer->isActive()) {
        m_pickerTimer->stop();
    }
    saveHistory();
}

void AdvancedColorPicker::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    
    // Preview section
    QHBoxLayout* previewLayout = new QHBoxLayout();
    
    m_previewLabel = new QLabel();
    m_previewLabel->setFixedSize(120, 80);
    m_previewLabel->setStyleSheet("border: 2px solid #3e3e42; border-radius: 4px;");
    previewLayout->addWidget(m_previewLabel);
    
    m_magnifierLabel = new QLabel();
    m_magnifierLabel->setFixedSize(80, 80);
    m_magnifierLabel->setStyleSheet("border: 2px solid #3e3e42; border-radius: 4px; background: #2d2d30;");
    previewLayout->addWidget(m_magnifierLabel);
    
    previewLayout->addStretch();
    mainLayout->addLayout(previewLayout);
    
    // Tab widget for color modes
    m_tabWidget = new QTabWidget();
    m_tabWidget->addTab(createRGBTab(), "RGB");
    m_tabWidget->addTab(createHSVTab(), "HSV");
    m_tabWidget->addTab(createHSLTab(), "HSL");
    m_tabWidget->addTab(createCMYKTab(), "CMYK");
    mainLayout->addWidget(m_tabWidget);
    
    // Hex input
    QHBoxLayout* hexLayout = new QHBoxLayout();
    hexLayout->addWidget(new QLabel("HEX:"));
    m_hexEdit = new QLineEdit("#FF0000");
    m_hexEdit->setFixedWidth(100);
    hexLayout->addWidget(m_hexEdit);
    hexLayout->addStretch();
    mainLayout->addLayout(hexLayout);
    
    // All formats display
    m_allFormatsLabel = new QLabel();
    m_allFormatsLabel->setStyleSheet("font-family: monospace; background: #2d2d30; padding: 8px; border-radius: 4px;");
    m_allFormatsLabel->setWordWrap(true);
    mainLayout->addWidget(m_allFormatsLabel);
    
    // Pick from screen button
    m_pickButton = new QPushButton("🎯 Pick from Screen");
    mainLayout->addWidget(m_pickButton);
    
    // History
    QGroupBox* historyBox = new QGroupBox("History");
    QVBoxLayout* historyLayout = new QVBoxLayout(historyBox);
    m_historyList = new QListWidget();
    m_historyList->setFlow(QListView::LeftToRight);
    m_historyList->setFixedHeight(50);
    m_historyList->setSpacing(3);
    historyLayout->addWidget(m_historyList);
    mainLayout->addWidget(historyBox);
    
    // OK/Cancel buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_okButton = new QPushButton("OK");
    m_cancelButton = new QPushButton("Cancel");
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    mainLayout->addLayout(buttonLayout);
    
    // Timer for screen picking
    m_pickerTimer = new QTimer(this);
    m_pickerTimer->setInterval(50);
    
    // Connections
    connect(m_hexEdit, &QLineEdit::editingFinished, this, &AdvancedColorPicker::onHexChanged);
    connect(m_pickButton, &QPushButton::clicked, this, &AdvancedColorPicker::onPickFromScreenClicked);
    connect(m_pickerTimer, &QTimer::timeout, this, &AdvancedColorPicker::onPickerTimer);
    connect(m_historyList, &QListWidget::itemClicked, this, &AdvancedColorPicker::onHistoryItemClicked);
    connect(m_okButton, &QPushButton::clicked, this, &AdvancedColorPicker::onOkClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &AdvancedColorPicker::onCancelClicked);
}

QWidget* AdvancedColorPicker::createRGBTab() {
    QWidget* tab = new QWidget();
    QGridLayout* layout = new QGridLayout(tab);
    
    layout->addWidget(new QLabel("Red:"), 0, 0);
    m_redSlider = new QSlider(Qt::Horizontal);
    m_redSlider->setRange(0, 255);
    layout->addWidget(m_redSlider, 0, 1);
    m_redEdit = new QLineEdit("255");
    m_redEdit->setFixedWidth(50);
    layout->addWidget(m_redEdit, 0, 2);
    
    layout->addWidget(new QLabel("Green:"), 1, 0);
    m_greenSlider = new QSlider(Qt::Horizontal);
    m_greenSlider->setRange(0, 255);
    layout->addWidget(m_greenSlider, 1, 1);
    m_greenEdit = new QLineEdit("0");
    m_greenEdit->setFixedWidth(50);
    layout->addWidget(m_greenEdit, 1, 2);
    
    layout->addWidget(new QLabel("Blue:"), 2, 0);
    m_blueSlider = new QSlider(Qt::Horizontal);
    m_blueSlider->setRange(0, 255);
    layout->addWidget(m_blueSlider, 2, 1);
    m_blueEdit = new QLineEdit("0");
    m_blueEdit->setFixedWidth(50);
    layout->addWidget(m_blueEdit, 2, 2);
    
    connect(m_redSlider, &QSlider::valueChanged, this, &AdvancedColorPicker::onRGBChanged);
    connect(m_greenSlider, &QSlider::valueChanged, this, &AdvancedColorPicker::onRGBChanged);
    connect(m_blueSlider, &QSlider::valueChanged, this, &AdvancedColorPicker::onRGBChanged);
    
    return tab;
}

QWidget* AdvancedColorPicker::createHSVTab() {
    QWidget* tab = new QWidget();
    QGridLayout* layout = new QGridLayout(tab);
    
    layout->addWidget(new QLabel("Hue:"), 0, 0);
    m_hueSlider = new QSlider(Qt::Horizontal);
    m_hueSlider->setRange(0, 359);
    layout->addWidget(m_hueSlider, 0, 1);
    m_hueEdit = new QLineEdit("0");
    m_hueEdit->setFixedWidth(50);
    layout->addWidget(m_hueEdit, 0, 2);
    
    layout->addWidget(new QLabel("Saturation:"), 1, 0);
    m_saturationSlider = new QSlider(Qt::Horizontal);
    m_saturationSlider->setRange(0, 255);
    layout->addWidget(m_saturationSlider, 1, 1);
    m_saturationEdit = new QLineEdit("255");
    m_saturationEdit->setFixedWidth(50);
    layout->addWidget(m_saturationEdit, 1, 2);
    
    layout->addWidget(new QLabel("Value:"), 2, 0);
    m_valueSlider = new QSlider(Qt::Horizontal);
    m_valueSlider->setRange(0, 255);
    layout->addWidget(m_valueSlider, 2, 1);
    m_valueEdit = new QLineEdit("255");
    m_valueEdit->setFixedWidth(50);
    layout->addWidget(m_valueEdit, 2, 2);
    
    connect(m_hueSlider, &QSlider::valueChanged, this, &AdvancedColorPicker::onHSVChanged);
    connect(m_saturationSlider, &QSlider::valueChanged, this, &AdvancedColorPicker::onHSVChanged);
    connect(m_valueSlider, &QSlider::valueChanged, this, &AdvancedColorPicker::onHSVChanged);
    
    return tab;
}

QWidget* AdvancedColorPicker::createHSLTab() {
    QWidget* tab = new QWidget();
    QGridLayout* layout = new QGridLayout(tab);
    
    layout->addWidget(new QLabel("Hue:"), 0, 0);
    m_hslHueSlider = new QSlider(Qt::Horizontal);
    m_hslHueSlider->setRange(0, 359);
    layout->addWidget(m_hslHueSlider, 0, 1);
    m_hslHueEdit = new QLineEdit("0");
    m_hslHueEdit->setFixedWidth(50);
    layout->addWidget(m_hslHueEdit, 0, 2);
    
    layout->addWidget(new QLabel("Saturation:"), 1, 0);
    m_hslSatSlider = new QSlider(Qt::Horizontal);
    m_hslSatSlider->setRange(0, 255);
    layout->addWidget(m_hslSatSlider, 1, 1);
    m_hslSatEdit = new QLineEdit("255");
    m_hslSatEdit->setFixedWidth(50);
    layout->addWidget(m_hslSatEdit, 1, 2);
    
    layout->addWidget(new QLabel("Lightness:"), 2, 0);
    m_lightnessSlider = new QSlider(Qt::Horizontal);
    m_lightnessSlider->setRange(0, 255);
    layout->addWidget(m_lightnessSlider, 2, 1);
    m_lightnessEdit = new QLineEdit("128");
    m_lightnessEdit->setFixedWidth(50);
    layout->addWidget(m_lightnessEdit, 2, 2);
    
    connect(m_hslHueSlider, &QSlider::valueChanged, this, &AdvancedColorPicker::onHSLChanged);
    connect(m_hslSatSlider, &QSlider::valueChanged, this, &AdvancedColorPicker::onHSLChanged);
    connect(m_lightnessSlider, &QSlider::valueChanged, this, &AdvancedColorPicker::onHSLChanged);
    
    return tab;
}

QWidget* AdvancedColorPicker::createCMYKTab() {
    QWidget* tab = new QWidget();
    QGridLayout* layout = new QGridLayout(tab);
    
    layout->addWidget(new QLabel("Cyan:"), 0, 0);
    m_cyanSlider = new QSlider(Qt::Horizontal);
    m_cyanSlider->setRange(0, 100);
    layout->addWidget(m_cyanSlider, 0, 1);
    m_cyanEdit = new QLineEdit("0");
    m_cyanEdit->setFixedWidth(50);
    layout->addWidget(m_cyanEdit, 0, 2);
    
    layout->addWidget(new QLabel("Magenta:"), 1, 0);
    m_magentaSlider = new QSlider(Qt::Horizontal);
    m_magentaSlider->setRange(0, 100);
    layout->addWidget(m_magentaSlider, 1, 1);
    m_magentaEdit = new QLineEdit("100");
    m_magentaEdit->setFixedWidth(50);
    layout->addWidget(m_magentaEdit, 1, 2);
    
    layout->addWidget(new QLabel("Yellow:"), 2, 0);
    m_yellowSlider = new QSlider(Qt::Horizontal);
    m_yellowSlider->setRange(0, 100);
    layout->addWidget(m_yellowSlider, 2, 1);
    m_yellowEdit = new QLineEdit("100");
    m_yellowEdit->setFixedWidth(50);
    layout->addWidget(m_yellowEdit, 2, 2);
    
    layout->addWidget(new QLabel("Black:"), 3, 0);
    m_blackSlider = new QSlider(Qt::Horizontal);
    m_blackSlider->setRange(0, 100);
    layout->addWidget(m_blackSlider, 3, 1);
    m_blackEdit = new QLineEdit("0");
    m_blackEdit->setFixedWidth(50);
    layout->addWidget(m_blackEdit, 3, 2);
    
    connect(m_cyanSlider, &QSlider::valueChanged, this, &AdvancedColorPicker::onCMYKChanged);
    connect(m_magentaSlider, &QSlider::valueChanged, this, &AdvancedColorPicker::onCMYKChanged);
    connect(m_yellowSlider, &QSlider::valueChanged, this, &AdvancedColorPicker::onCMYKChanged);
    connect(m_blackSlider, &QSlider::valueChanged, this, &AdvancedColorPicker::onCMYKChanged);
    
    return tab;
}

void AdvancedColorPicker::setColor(const QColor& color) {
    m_selectedColor = color;
    updateAllDisplays();
}

QColor AdvancedColorPicker::getSelectedColor() const {
    return m_selectedColor;
}

void AdvancedColorPicker::updateAllDisplays() {
    m_updatingUI = true;
    updatePreview();
    updateRGBTab();
    updateHSVTab();
    updateHSLTab();
    updateCMYKTab();
    updateHexEdit();
    updateAllFormatsLabel();
    m_updatingUI = false;
}

void AdvancedColorPicker::updatePreview() {
    m_previewLabel->setStyleSheet(
        QString("background-color: %1; border: 2px solid #3e3e42; border-radius: 4px;")
            .arg(m_selectedColor.name()));
}

void AdvancedColorPicker::updateRGBTab() {
    m_redSlider->setValue(m_selectedColor.red());
    m_greenSlider->setValue(m_selectedColor.green());
    m_blueSlider->setValue(m_selectedColor.blue());
    m_redEdit->setText(QString::number(m_selectedColor.red()));
    m_greenEdit->setText(QString::number(m_selectedColor.green()));
    m_blueEdit->setText(QString::number(m_selectedColor.blue()));
}

void AdvancedColorPicker::updateHSVTab() {
    int h, s, v;
    m_selectedColor.getHsv(&h, &s, &v);
    m_hueSlider->setValue(h);
    m_saturationSlider->setValue(s);
    m_valueSlider->setValue(v);
    m_hueEdit->setText(QString::number(h));
    m_saturationEdit->setText(QString::number(s));
    m_valueEdit->setText(QString::number(v));
}

void AdvancedColorPicker::updateHSLTab() {
    int h, s, l;
    m_selectedColor.getHsl(&h, &s, &l);
    m_hslHueSlider->setValue(h);
    m_hslSatSlider->setValue(s);
    m_lightnessSlider->setValue(l);
    m_hslHueEdit->setText(QString::number(h));
    m_hslSatEdit->setText(QString::number(s));
    m_lightnessEdit->setText(QString::number(l));
}

void AdvancedColorPicker::updateCMYKTab() {
    int c, m, y, k;
    m_selectedColor.getCmyk(&c, &m, &y, &k);
    int cPct = c * 100 / 255;
    int mPct = m * 100 / 255;
    int yPct = y * 100 / 255;
    int kPct = k * 100 / 255;
    
    m_cyanSlider->setValue(cPct);
    m_magentaSlider->setValue(mPct);
    m_yellowSlider->setValue(yPct);
    m_blackSlider->setValue(kPct);
    m_cyanEdit->setText(QString::number(cPct));
    m_magentaEdit->setText(QString::number(mPct));
    m_yellowEdit->setText(QString::number(yPct));
    m_blackEdit->setText(QString::number(kPct));
}

void AdvancedColorPicker::updateHexEdit() {
    m_hexEdit->setText(m_selectedColor.name().toUpper());
}

void AdvancedColorPicker::updateAllFormatsLabel() {
    m_allFormatsLabel->setText(colorToAllFormats(m_selectedColor));
}

QString AdvancedColorPicker::colorToAllFormats(const QColor& color) {
    int h, s, v, hsl_h, hsl_s, l, c, m, y, k;
    color.getHsv(&h, &s, &v);
    color.getHsl(&hsl_h, &hsl_s, &l);
    color.getCmyk(&c, &m, &y, &k);
    
    return QString(
        "RGB: %1, %2, %3\n"
        "HEX: %4\n"
        "HSV: %5°, %6%, %7%\n"
        "HSL: %8°, %9%, %10%\n"
        "CMYK: %11%, %12%, %13%, %14%"
    ).arg(color.red()).arg(color.green()).arg(color.blue())
     .arg(color.name().toUpper())
     .arg(h).arg(s * 100 / 255).arg(v * 100 / 255)
     .arg(hsl_h).arg(hsl_s * 100 / 255).arg(l * 100 / 255)
     .arg(c * 100 / 255).arg(m * 100 / 255).arg(y * 100 / 255).arg(k * 100 / 255);
}

void AdvancedColorPicker::onRGBChanged() {
    if (m_updatingUI) return;
    m_selectedColor = QColor(m_redSlider->value(), m_greenSlider->value(), m_blueSlider->value());
    updateAllDisplays();
}

void AdvancedColorPicker::onHSVChanged() {
    if (m_updatingUI) return;
    m_selectedColor.setHsv(m_hueSlider->value(), m_saturationSlider->value(), m_valueSlider->value());
    updateAllDisplays();
}

void AdvancedColorPicker::onHSLChanged() {
    if (m_updatingUI) return;
    m_selectedColor.setHsl(m_hslHueSlider->value(), m_hslSatSlider->value(), m_lightnessSlider->value());
    updateAllDisplays();
}

void AdvancedColorPicker::onCMYKChanged() {
    if (m_updatingUI) return;
    int c = m_cyanSlider->value() * 255 / 100;
    int m = m_magentaSlider->value() * 255 / 100;
    int y = m_yellowSlider->value() * 255 / 100;
    int k = m_blackSlider->value() * 255 / 100;
    m_selectedColor.setCmyk(c, m, y, k);
    updateAllDisplays();
}

void AdvancedColorPicker::onHexChanged() {
    QString hex = m_hexEdit->text();
    if (!hex.startsWith('#')) hex = '#' + hex;
    QColor color(hex);
    if (color.isValid()) {
        m_selectedColor = color;
        updateAllDisplays();
    }
}

void AdvancedColorPicker::onPickFromScreenClicked() {
    m_isPicking = !m_isPicking;
    if (m_isPicking) {
        m_pickerTimer->start();
        m_pickButton->setText("Click to pick... (ESC to cancel)");
        setCursor(Qt::CrossCursor);
    } else {
        m_pickerTimer->stop();
        m_pickButton->setText("🎯 Pick from Screen");
        setCursor(Qt::ArrowCursor);
    }
}

void AdvancedColorPicker::onPickerTimer() {
    QColor color = getColorAtCursor();
    m_selectedColor = color;
    updateAllDisplays();
    
    if (QGuiApplication::mouseButtons() & Qt::LeftButton) {
        m_isPicking = false;
        m_pickerTimer->stop();
        m_pickButton->setText("🎯 Pick from Screen");
        setCursor(Qt::ArrowCursor);
        addToHistory(m_selectedColor);
    }
}

QColor AdvancedColorPicker::getColorAtCursor() {
    QPoint pos = QCursor::pos();
    QScreen* screen = QGuiApplication::screenAt(pos);
    if (!screen) screen = QGuiApplication::primaryScreen();
    QPixmap pixmap = screen->grabWindow(0, pos.x(), pos.y(), 1, 1);
    return pixmap.toImage().pixelColor(0, 0);
}

void AdvancedColorPicker::onHistoryItemClicked(QListWidgetItem* item) {
    QColor color = item->data(Qt::UserRole).value<QColor>();
    m_selectedColor = color;
    updateAllDisplays();
}

void AdvancedColorPicker::addToHistory(const QColor& color) {
    // Remove if already exists
    for (auto it = m_colorHistory.begin(); it != m_colorHistory.end(); ) {
        if (*it == color) {
            it = m_colorHistory.erase(it);
        } else {
            ++it;
        }
    }
    
    // Add to front
    m_colorHistory.insert(m_colorHistory.begin(), color);
    
    // Limit to 10
    if (m_colorHistory.size() > 10) {
        m_colorHistory.resize(10);
    }
    
    // Update list widget
    m_historyList->clear();
    for (const auto& c : m_colorHistory) {
        QListWidgetItem* item = new QListWidgetItem();
        item->setSizeHint(QSize(30, 30));
        item->setBackground(c);
        item->setData(Qt::UserRole, QVariant::fromValue(c));
        m_historyList->addItem(item);
    }
}

void AdvancedColorPicker::loadHistory() {
    QSettings settings("AccessibilityGamingAssistant", "ColorPicker");
    int count = settings.beginReadArray("history");
    for (int i = 0; i < count && i < 10; ++i) {
        settings.setArrayIndex(i);
        QString colorStr = settings.value("color").toString();
        QColor color(colorStr);
        if (color.isValid()) {
            m_colorHistory.push_back(color);
        }
    }
    settings.endArray();
    
    // Populate list
    for (const auto& c : m_colorHistory) {
        QListWidgetItem* item = new QListWidgetItem();
        item->setSizeHint(QSize(30, 30));
        item->setBackground(c);
        item->setData(Qt::UserRole, QVariant::fromValue(c));
        m_historyList->addItem(item);
    }
}

void AdvancedColorPicker::saveHistory() {
    QSettings settings("AccessibilityGamingAssistant", "ColorPicker");
    settings.beginWriteArray("history");
    for (size_t i = 0; i < m_colorHistory.size(); ++i) {
        settings.setArrayIndex(static_cast<int>(i));
        settings.setValue("color", m_colorHistory[i].name());
    }
    settings.endArray();
}

void AdvancedColorPicker::onOkClicked() {
    addToHistory(m_selectedColor);
    emit colorSelected(m_selectedColor);
    accept();
}

void AdvancedColorPicker::onCancelClicked() {
    reject();
}
