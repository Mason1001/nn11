#include "core/ScreenCapture.h"
#include <QGuiApplication>
#include <QScreen>
#include <QPixmap>
#include <QElapsedTimer>
#include <QList>
#include <algorithm>
#include <cstdlib>

ScreenCapture::ScreenCapture(QObject* parent)
    : QObject(parent)
    , m_activeMonitor(0)
    , m_lastCaptureTime(0.0)
#ifdef _WIN32
    , m_screenDC(nullptr)
    , m_memDC(nullptr)
    , m_bitmap(nullptr)
    , m_bitmapData(nullptr)
    , m_captureWidth(0)
    , m_captureHeight(0)
#endif
{
    detectMonitors();
#ifdef _WIN32
    initWindowsCapture();
#endif
}

ScreenCapture::~ScreenCapture() {
#ifdef _WIN32
    cleanupWindowsCapture();
#endif
}

void ScreenCapture::detectMonitors() {
    m_monitors.clear();
    
    QList<QScreen*> screens = QGuiApplication::screens();
    QScreen* primaryScreen = QGuiApplication::primaryScreen();
    
    for (int i = 0; i < screens.size(); ++i) {
        QScreen* screen = screens[i];
        MonitorInfo info;
        info.index = i;
        info.name = screen->name();
        info.geometry = screen->geometry();
        info.isPrimary = (screen == primaryScreen);
        info.dpi = screen->logicalDotsPerInch();
        m_monitors.push_back(info);
    }
}

std::vector<MonitorInfo> ScreenCapture::getMonitors() const {
    return m_monitors;
}

void ScreenCapture::setActiveMonitor(int index) {
    if (index >= 0 && index < static_cast<int>(m_monitors.size())) {
        m_activeMonitor = index;
#ifdef _WIN32
        cleanupWindowsCapture();
        initWindowsCapture();
#endif
        emit monitorChanged(index);
    }
}

int ScreenCapture::getActiveMonitor() const {
    return m_activeMonitor;
}

MonitorInfo ScreenCapture::getCurrentMonitorInfo() const {
    if (m_activeMonitor >= 0 && m_activeMonitor < static_cast<int>(m_monitors.size())) {
        return m_monitors[m_activeMonitor];
    }
    return MonitorInfo{};
}

QSize ScreenCapture::getScreenSize() const {
    if (m_activeMonitor >= 0 && m_activeMonitor < static_cast<int>(m_monitors.size())) {
        return m_monitors[m_activeMonitor].geometry.size();
    }
    return QSize(1920, 1080);
}

QPoint ScreenCapture::getScreenCenter() const {
    QSize size = getScreenSize();
    QPoint offset(0, 0);
    
    if (m_activeMonitor >= 0 && m_activeMonitor < static_cast<int>(m_monitors.size())) {
        offset = m_monitors[m_activeMonitor].geometry.topLeft();
    }
    
    return QPoint(offset.x() + size.width() / 2, offset.y() + size.height() / 2);
}

double ScreenCapture::getLastCaptureTime() const {
    return m_lastCaptureTime;
}

#ifdef _WIN32
void ScreenCapture::initWindowsCapture() {
    MonitorInfo monitor = getCurrentMonitorInfo();
    m_captureWidth = monitor.geometry.width();
    m_captureHeight = monitor.geometry.height();
    
    m_screenDC = GetDC(nullptr);
    m_memDC = CreateCompatibleDC(m_screenDC);
    
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = m_captureWidth;
    bmi.bmiHeader.biHeight = -m_captureHeight; // Top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    m_bitmap = CreateDIBSection(m_memDC, &bmi, DIB_RGB_COLORS, &m_bitmapData, nullptr, 0);
    SelectObject(m_memDC, m_bitmap);
}

void ScreenCapture::cleanupWindowsCapture() {
    if (m_bitmap) {
        DeleteObject(m_bitmap);
        m_bitmap = nullptr;
    }
    if (m_memDC) {
        DeleteDC(m_memDC);
        m_memDC = nullptr;
    }
    if (m_screenDC) {
        ReleaseDC(nullptr, m_screenDC);
        m_screenDC = nullptr;
    }
    m_bitmapData = nullptr;
}

cv::Mat ScreenCapture::captureWindows() {
    QElapsedTimer timer;
    timer.start();
    
    MonitorInfo monitor = getCurrentMonitorInfo();
    
    BitBlt(m_memDC, 0, 0, m_captureWidth, m_captureHeight,
           m_screenDC, monitor.geometry.x(), monitor.geometry.y(), SRCCOPY);
    
    cv::Mat result(m_captureHeight, m_captureWidth, CV_8UC4, m_bitmapData);
    cv::Mat bgr;
    cv::cvtColor(result, bgr, cv::COLOR_BGRA2BGR);
    
    m_lastCaptureTime = timer.elapsed();
    return bgr;
}

cv::Mat ScreenCapture::captureWindowsRegion(const QRect& region) {
    QElapsedTimer timer;
    timer.start();
    
    MonitorInfo monitor = getCurrentMonitorInfo();
    int x = monitor.geometry.x() + region.x();
    int y = monitor.geometry.y() + region.y();
    
    HDC regionDC = CreateCompatibleDC(m_screenDC);
    
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = region.width();
    bmi.bmiHeader.biHeight = -region.height();
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    void* regionData = nullptr;
    HBITMAP regionBitmap = CreateDIBSection(regionDC, &bmi, DIB_RGB_COLORS, &regionData, nullptr, 0);
    SelectObject(regionDC, regionBitmap);
    
    BitBlt(regionDC, 0, 0, region.width(), region.height(), m_screenDC, x, y, SRCCOPY);
    
    cv::Mat result(region.height(), region.width(), CV_8UC4, regionData);
    cv::Mat bgr;
    cv::cvtColor(result, bgr, cv::COLOR_BGRA2BGR);
    
    DeleteObject(regionBitmap);
    DeleteDC(regionDC);
    
    m_lastCaptureTime = timer.elapsed();
    return bgr.clone();
}
#endif

cv::Mat ScreenCapture::capture() {
#ifdef _WIN32
    return captureWindows();
#else
    QElapsedTimer timer;
    timer.start();
    
    QScreen* screen = QGuiApplication::screens().at(m_activeMonitor);
    QPixmap pixmap = screen->grabWindow(0);
    QImage image = pixmap.toImage().convertToFormat(QImage::Format_RGB888);
    
    cv::Mat mat(image.height(), image.width(), CV_8UC3, 
                const_cast<uchar*>(image.bits()), image.bytesPerLine());
    cv::Mat bgr;
    cv::cvtColor(mat, bgr, cv::COLOR_RGB2BGR);
    
    m_lastCaptureTime = timer.elapsed();
    return bgr.clone();
#endif
}

cv::Mat ScreenCapture::captureRegion(const QRect& region) {
#ifdef _WIN32
    return captureWindowsRegion(region);
#else
    cv::Mat fullCapture = capture();
    
    int x = std::max(0, region.x());
    int y = std::max(0, region.y());
    int width = std::min(region.width(), fullCapture.cols - x);
    int height = std::min(region.height(), fullCapture.rows - y);
    
    return fullCapture(cv::Rect(x, y, width, height)).clone();
#endif
}

cv::Mat ScreenCapture::captureFOV(int centerX, int centerY, int radius) {
    int x = centerX - radius;
    int y = centerY - radius;
    int size = radius * 2;
    
    return captureRegion(QRect(x, y, size, size));
}

QImage ScreenCapture::convertToQImage(const cv::Mat& mat) {
    if (mat.type() == CV_8UC3) {
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        return QImage(rgb.data, rgb.cols, rgb.rows, rgb.step, 
                      QImage::Format_RGB888).copy();
    }
    return QImage();
}

cv::Mat ScreenCapture::convertToCvMat(const QImage& image) {
    QImage converted = image.convertToFormat(QImage::Format_RGB888);
    cv::Mat mat(converted.height(), converted.width(), CV_8UC3,
                const_cast<uchar*>(converted.bits()), converted.bytesPerLine());
    cv::Mat bgr;
    cv::cvtColor(mat, bgr, cv::COLOR_RGB2BGR);
    return bgr.clone();
}
