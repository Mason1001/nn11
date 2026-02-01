#ifndef SCREENCAPTURE_H
#define SCREENCAPTURE_H

#include <QObject>
#include <QImage>
#include <QRect>
#include <QSize>
#include <QPoint>
#include <QString>
#include <QScreen>
#include <opencv2/opencv.hpp>
#include <vector>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#endif

struct MonitorInfo {
    int index;
    QString name;
    QRect geometry;
    bool isPrimary;
    qreal dpi;
};

class ScreenCapture : public QObject {
    Q_OBJECT

public:
    explicit ScreenCapture(QObject* parent = nullptr);
    ~ScreenCapture();

    // Capture methods
    cv::Mat capture();
    cv::Mat captureRegion(const QRect& region);
    cv::Mat captureFOV(int centerX, int centerY, int radius);

    // Monitor management
    std::vector<MonitorInfo> getMonitors() const;
    void setActiveMonitor(int index);
    int getActiveMonitor() const;
    MonitorInfo getCurrentMonitorInfo() const;

    // Screen info
    QSize getScreenSize() const;
    QPoint getScreenCenter() const;

    // Performance
    double getLastCaptureTime() const;

signals:
    void monitorChanged(int index);
    void captureError(const QString& error);

private:
    int m_activeMonitor;
    double m_lastCaptureTime;
    std::vector<MonitorInfo> m_monitors;

#ifdef _WIN32
    HDC m_screenDC;
    HDC m_memDC;
    HBITMAP m_bitmap;
    void* m_bitmapData;
    int m_captureWidth;
    int m_captureHeight;

    void initWindowsCapture();
    void cleanupWindowsCapture();
    cv::Mat captureWindows();
    cv::Mat captureWindowsRegion(const QRect& region);
#endif

    void detectMonitors();
    QImage convertToQImage(const cv::Mat& mat);
    cv::Mat convertToCvMat(const QImage& image);
};

#endif // SCREENCAPTURE_H
