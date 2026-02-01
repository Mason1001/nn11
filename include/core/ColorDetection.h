#ifndef COLORDETECTION_H
#define COLORDETECTION_H

#include <QObject>
#include <QColor>
#include <QPoint>
#include <QRect>
#include <opencv2/opencv.hpp>
#include <vector>
#include <algorithm>

struct DetectedTarget {
    QPoint center;
    QRect boundingBox;
    double confidence;
    double distanceFromCenter;
    double area;
};

struct ColorRange {
    cv::Scalar lower;
    cv::Scalar upper;
};

class ColorDetection : public QObject {
    Q_OBJECT

public:
    explicit ColorDetection(QObject* parent = nullptr);
    ~ColorDetection();

    // Main detection method
    std::vector<DetectedTarget> detect(const cv::Mat& frame, const QPoint& screenCenter);

    // Color settings
    void setTargetColor(const QColor& color);
    QColor getTargetColor() const;

    void setColorTolerance(int tolerance);
    int getColorTolerance() const;

    // FOV settings
    void setFOVRadius(int radius);
    int getFOVRadius() const;

    // Detection parameters
    void setMinArea(double area);
    double getMinArea() const;

    void setMaxArea(double area);
    double getMaxArea() const;

    void setMorphologyEnabled(bool enabled);
    bool isMorphologyEnabled() const;

    // Performance stats
    double getLastDetectionTime() const;
    int getLastTargetCount() const;

signals:
    void targetDetected(const DetectedTarget& target);
    void detectionComplete(int targetCount, double timeMs);

private:
    QColor m_targetColor;
    int m_colorTolerance;
    int m_fovRadius;
    double m_minArea;
    double m_maxArea;
    bool m_morphologyEnabled;
    double m_lastDetectionTime;
    int m_lastTargetCount;

    ColorRange calculateColorRange(const QColor& color, int tolerance);
    cv::Mat createFOVMask(const cv::Size& frameSize, const QPoint& center, int radius);
    cv::Mat applyMorphology(const cv::Mat& mask);
    std::vector<DetectedTarget> findTargets(const cv::Mat& mask, const QPoint& screenCenter);
    double calculateConfidence(double area, double distanceFromCenter);
};

#endif // COLORDETECTION_H
