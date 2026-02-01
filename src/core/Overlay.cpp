#include "core/Overlay.h"
#include <QGuiApplication>
#include <QScreen>
#include <algorithm>

#ifdef _WIN32
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#endif

Overlay::Overlay(QWidget* parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool)
    , m_fovRadius(150)
    , m_fovColor(QColor(0, 255, 0, 128))
    , m_fovVisible(true)
    , m_targetIndicatorVisible(true)
    , m_crosshairVisible(false)
    , m_crosshairColor(QColor(255, 0, 0, 200))
    , m_crosshairSize(20)
    , m_overlayEnabled(true)
{
    setupOverlayWindow();
    
    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setInterval(16); // ~60 FPS refresh
    connect(m_refreshTimer, &QTimer::timeout, this, &Overlay::refresh);
}

Overlay::~Overlay() {
    if (m_refreshTimer->isActive()) {
        m_refreshTimer->stop();
    }
}

void Overlay::setupOverlayWindow() {
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_ShowWithoutActivating);
    
#ifdef _WIN32
    makeClickThrough();
#endif
    
    // Set to full screen
    QScreen* screen = QGuiApplication::primaryScreen();
    if (screen) {
        setGeometry(screen->geometry());
    }
}

#ifdef _WIN32
void Overlay::makeClickThrough() {
    HWND hwnd = reinterpret_cast<HWND>(winId());
    
    // Set extended window style for click-through
    LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    SetWindowLong(hwnd, GWL_EXSTYLE, exStyle | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOPMOST);
    
    // Enable transparency
    SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
}
#endif

void Overlay::setFOVRadius(int radius) {
    m_fovRadius = std::clamp(radius, 50, 500);
    update();
}

int Overlay::getFOVRadius() const {
    return m_fovRadius;
}

void Overlay::setFOVColor(const QColor& color) {
    m_fovColor = color;
    update();
}

QColor Overlay::getFOVColor() const {
    return m_fovColor;
}

void Overlay::setFOVVisible(bool visible) {
    m_fovVisible = visible;
    update();
}

bool Overlay::isFOVVisible() const {
    return m_fovVisible;
}

void Overlay::setTargets(const std::vector<OverlayTarget>& targets) {
    m_targets = targets;
    update();
}

void Overlay::clearTargets() {
    m_targets.clear();
    update();
}

void Overlay::setTargetIndicatorVisible(bool visible) {
    m_targetIndicatorVisible = visible;
    update();
}

bool Overlay::isTargetIndicatorVisible() const {
    return m_targetIndicatorVisible;
}

void Overlay::setCrosshairVisible(bool visible) {
    m_crosshairVisible = visible;
    update();
}

bool Overlay::isCrosshairVisible() const {
    return m_crosshairVisible;
}

void Overlay::setCrosshairColor(const QColor& color) {
    m_crosshairColor = color;
    update();
}

QColor Overlay::getCrosshairColor() const {
    return m_crosshairColor;
}

void Overlay::setCrosshairSize(int size) {
    m_crosshairSize = std::clamp(size, 5, 100);
    update();
}

int Overlay::getCrosshairSize() const {
    return m_crosshairSize;
}

void Overlay::setOverlayEnabled(bool enabled) {
    m_overlayEnabled = enabled;
    
    if (enabled) {
        show();
        m_refreshTimer->start();
    } else {
        hide();
        m_refreshTimer->stop();
    }
}

bool Overlay::isOverlayEnabled() const {
    return m_overlayEnabled;
}

void Overlay::updatePosition() {
    QScreen* screen = QGuiApplication::primaryScreen();
    if (screen) {
        setGeometry(screen->geometry());
    }
}

void Overlay::refresh() {
    update();
}

void Overlay::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    m_refreshTimer->start();
}

void Overlay::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    
    if (!m_overlayEnabled) {
        return;
    }
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    if (m_fovVisible) {
        drawFOVCircle(painter);
    }
    
    if (m_targetIndicatorVisible) {
        drawTargets(painter);
    }
    
    if (m_crosshairVisible) {
        drawCrosshair(painter);
    }
}

void Overlay::drawFOVCircle(QPainter& painter) {
    QPoint center(width() / 2, height() / 2);
    
    // Draw FOV circle outline
    QPen pen(m_fovColor);
    pen.setWidth(2);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    
    painter.drawEllipse(center, m_fovRadius, m_fovRadius);
}

void Overlay::drawTargets(QPainter& painter) {
    for (const auto& target : m_targets) {
        QColor color = target.isSelected ? QColor(255, 255, 0, 200) : target.color;
        
        QPen pen(color);
        pen.setWidth(2);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        
        // Draw bounding box
        painter.drawRect(target.boundingBox);
        
        // Draw center marker
        int markerSize = 5;
        painter.drawLine(target.position.x() - markerSize, target.position.y(),
                         target.position.x() + markerSize, target.position.y());
        painter.drawLine(target.position.x(), target.position.y() - markerSize,
                         target.position.x(), target.position.y() + markerSize);
    }
}

void Overlay::drawCrosshair(QPainter& painter) {
    QPoint center(width() / 2, height() / 2);
    
    QPen pen(m_crosshairColor);
    pen.setWidth(2);
    painter.setPen(pen);
    
    int half = m_crosshairSize / 2;
    int gap = 3;
    
    // Horizontal lines
    painter.drawLine(center.x() - half, center.y(), center.x() - gap, center.y());
    painter.drawLine(center.x() + gap, center.y(), center.x() + half, center.y());
    
    // Vertical lines
    painter.drawLine(center.x(), center.y() - half, center.x(), center.y() - gap);
    painter.drawLine(center.x(), center.y() + gap, center.x(), center.y() + half);
    
    // Center dot
    painter.setBrush(m_crosshairColor);
    painter.drawEllipse(center, 2, 2);
}
