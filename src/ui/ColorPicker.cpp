#include "ui/ColorPicker.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QGuiApplication>
#include <QScreen>
#include <QCursor>

#ifdef _WIN32
#include <windows.h>
#endif

ColorPicker::ColorPicker(QWidget* parent)
    : QDialog(parent)
    , m_selectedColor(Qt::red)
    , m_originalColor(Qt::red)
    , m_isPicking(false)
{
    setWindowTitle("Color Picker");
    setModal(true);
    setFixedSize(350, 400);
    
    setupUI();
    updatePreview();
}

ColorPicker::~ColorPicker() {
    if (m_pickerTimer && m_pickerTimer->isActive()) {
        m_pickerTimer->stop();
    }
}

void ColorPicker::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    
    // Preview section
    QHBoxLayout* previewLayout = new QHBoxLayout();
    
    QGroupBox* currentBox = new QGroupBox("Current");
    QVBoxLayout* currentLayout = new QVBoxLayout(currentBox);
    m_previewLabel = new QLabel();
    m_previewLabel->setFixedSize(80, 60);
    m_previewLabel->setStyleSheet("border: 2px solid #3e3e42; border-radius: 4px;");
    currentLayout->addWidget(m_previewLabel, 0, Qt::AlignCenter);
    previewLayout->addWidget(currentBox);
    
    QGroupBox* originalBox = new QGroupBox("Original");
    QVBoxLayout* originalLayout = new QVBoxLayout(originalBox);
    m_originalLabel = new QLabel();
    m_originalLabel->setFixedSize(80, 60);
    m_originalLabel->setStyleSheet("border: 2px solid #3e3e42; border-radius: 4px;");
    originalLayout->addWidget(m_originalLabel, 0, Qt::AlignCenter);
    previewLayout->addWidget(originalBox);
    
    mainLayout->addLayout(previewLayout);
    
    // RGB sliders
    QGroupBox* rgbBox = new QGroupBox("RGB");
    QGridLayout* rgbLayout = new QGridLayout(rgbBox);
    
    rgbLayout->addWidget(new QLabel("R:"), 0, 0);
    m_redSlider = new QSlider(Qt::Horizontal);
    m_redSlider->setRange(0, 255);
    rgbLayout->addWidget(m_redSlider, 0, 1);
    m_redEdit = new QLineEdit("0");
    m_redEdit->setFixedWidth(50);
    rgbLayout->addWidget(m_redEdit, 0, 2);
    
    rgbLayout->addWidget(new QLabel("G:"), 1, 0);
    m_greenSlider = new QSlider(Qt::Horizontal);
    m_greenSlider->setRange(0, 255);
    rgbLayout->addWidget(m_greenSlider, 1, 1);
    m_greenEdit = new QLineEdit("0");
    m_greenEdit->setFixedWidth(50);
    rgbLayout->addWidget(m_greenEdit, 1, 2);
    
    rgbLayout->addWidget(new QLabel("B:"), 2, 0);
    m_blueSlider = new QSlider(Qt::Horizontal);
    m_blueSlider->setRange(0, 255);
    rgbLayout->addWidget(m_blueSlider, 2, 1);
    m_blueEdit = new QLineEdit("0");
    m_blueEdit->setFixedWidth(50);
    rgbLayout->addWidget(m_blueEdit, 2, 2);
    
    mainLayout->addWidget(rgbBox);
    
    // Hex input
    QHBoxLayout* hexLayout = new QHBoxLayout();
    hexLayout->addWidget(new QLabel("HEX:"));
    m_hexEdit = new QLineEdit("#FF0000");
    m_hexEdit->setFixedWidth(100);
    hexLayout->addWidget(m_hexEdit);
    hexLayout->addStretch();
    mainLayout->addLayout(hexLayout);
    
    // Pick from screen button
    m_pickButton = new QPushButton("🎯 Pick from Screen");
    mainLayout->addWidget(m_pickButton);
    
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
    connect(m_redSlider, &QSlider::valueChanged, this, &ColorPicker::onRGBChanged);
    connect(m_greenSlider, &QSlider::valueChanged, this, &ColorPicker::onRGBChanged);
    connect(m_blueSlider, &QSlider::valueChanged, this, &ColorPicker::onRGBChanged);
    
    connect(m_redEdit, &QLineEdit::editingFinished, [this]() {
        m_redSlider->setValue(m_redEdit->text().toInt());
    });
    connect(m_greenEdit, &QLineEdit::editingFinished, [this]() {
        m_greenSlider->setValue(m_greenEdit->text().toInt());
    });
    connect(m_blueEdit, &QLineEdit::editingFinished, [this]() {
        m_blueSlider->setValue(m_blueEdit->text().toInt());
    });
    
    connect(m_hexEdit, &QLineEdit::editingFinished, this, &ColorPicker::onHexChanged);
    connect(m_pickButton, &QPushButton::clicked, this, &ColorPicker::onPickFromScreenClicked);
    connect(m_pickerTimer, &QTimer::timeout, this, &ColorPicker::onPickerTimer);
    connect(m_okButton, &QPushButton::clicked, this, &ColorPicker::onOkClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &ColorPicker::onCancelClicked);
}

void ColorPicker::setColor(const QColor& color) {
    m_selectedColor = color;
    m_originalColor = color;
    updateSliders();
    updateEdits();
    updatePreview();
}

QColor ColorPicker::getSelectedColor() const {
    return m_selectedColor;
}

void ColorPicker::updatePreview() {
    m_previewLabel->setStyleSheet(
        QString("background-color: %1; border: 2px solid #3e3e42; border-radius: 4px;")
            .arg(m_selectedColor.name()));
    m_originalLabel->setStyleSheet(
        QString("background-color: %1; border: 2px solid #3e3e42; border-radius: 4px;")
            .arg(m_originalColor.name()));
}

void ColorPicker::updateSliders() {
    m_redSlider->blockSignals(true);
    m_greenSlider->blockSignals(true);
    m_blueSlider->blockSignals(true);
    
    m_redSlider->setValue(m_selectedColor.red());
    m_greenSlider->setValue(m_selectedColor.green());
    m_blueSlider->setValue(m_selectedColor.blue());
    
    m_redSlider->blockSignals(false);
    m_greenSlider->blockSignals(false);
    m_blueSlider->blockSignals(false);
}

void ColorPicker::updateEdits() {
    m_redEdit->setText(QString::number(m_selectedColor.red()));
    m_greenEdit->setText(QString::number(m_selectedColor.green()));
    m_blueEdit->setText(QString::number(m_selectedColor.blue()));
    m_hexEdit->setText(m_selectedColor.name().toUpper());
}

void ColorPicker::onRGBChanged() {
    m_selectedColor = QColor(m_redSlider->value(), 
                              m_greenSlider->value(), 
                              m_blueSlider->value());
    updateEdits();
    updatePreview();
    emit colorChanged(m_selectedColor);
}

void ColorPicker::onHexChanged() {
    QString hex = m_hexEdit->text();
    if (!hex.startsWith('#')) {
        hex = '#' + hex;
    }
    
    QColor color(hex);
    if (color.isValid()) {
        m_selectedColor = color;
        updateSliders();
        updatePreview();
        emit colorChanged(m_selectedColor);
    }
}

void ColorPicker::onPickFromScreenClicked() {
    if (m_isPicking) {
        m_isPicking = false;
        m_pickerTimer->stop();
        m_pickButton->setText("🎯 Pick from Screen");
        setCursor(Qt::ArrowCursor);
    } else {
        m_isPicking = true;
        m_pickerTimer->start();
        m_pickButton->setText("Click anywhere... (ESC to cancel)");
        setCursor(Qt::CrossCursor);
    }
}

void ColorPicker::onPickerTimer() {
    QColor color = getColorAtCursor();
    m_selectedColor = color;
    updateSliders();
    updateEdits();
    updatePreview();
    
    // Check for mouse click
    if (QGuiApplication::mouseButtons() & Qt::LeftButton) {
        m_isPicking = false;
        m_pickerTimer->stop();
        m_pickButton->setText("🎯 Pick from Screen");
        setCursor(Qt::ArrowCursor);
        emit colorSelected(m_selectedColor);
    }
}

QColor ColorPicker::getColorAtCursor() {
    QPoint pos = QCursor::pos();
    QScreen* screen = QGuiApplication::screenAt(pos);
    
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    
    QPixmap pixmap = screen->grabWindow(0, pos.x(), pos.y(), 1, 1);
    QImage image = pixmap.toImage();
    
    return image.pixelColor(0, 0);
}

void ColorPicker::onOkClicked() {
    emit colorSelected(m_selectedColor);
    accept();
}

void ColorPicker::onCancelClicked() {
    m_selectedColor = m_originalColor;
    reject();
}
