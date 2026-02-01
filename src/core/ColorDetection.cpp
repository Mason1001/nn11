#include "core/ColorDetection.h"
#include <QElapsedTimer>
#include <algorithm>
#include <cmath>
#include <cstdlib>

ColorDetection::ColorDetection(QObject* parent)
    : QObject(parent)
    , m_targetColor(Qt::red)
    , m_colorTolerance(30)
    , m_fovRadius(150)
    , m_minArea(50.0)
    , m_maxArea(50000.0)
    , m_morphologyEnabled(true)
    , m_lastDetectionTime(0.0)
    , m_lastTargetCount(0)
{
}

ColorDetection::~ColorDetection() {
}

void ColorDetection::setTargetColor(const QColor& color) {
    m_targetColor = color;
}

QColor ColorDetection::getTargetColor() const {
    return m_targetColor;
}

void ColorDetection::setColorTolerance(int tolerance) {
    m_colorTolerance = std::clamp(tolerance, 0, 100);
}

int ColorDetection::getColorTolerance() const {
    return m_colorTolerance;
}

void ColorDetection::setFOVRadius(int radius) {
    m_fovRadius = std::clamp(radius, 50, 500);
}

int ColorDetection::getFOVRadius() const {
    return m_fovRadius;
}

void ColorDetection::setMinArea(double area) {
    m_minArea = area;
}

double ColorDetection::getMinArea() const {
    return m_minArea;
}

void ColorDetection::setMaxArea(double area) {
    m_maxArea = area;
}

double ColorDetection::getMaxArea() const {
    return m_maxArea;
}

void ColorDetection::setMorphologyEnabled(bool enabled) {
    m_morphologyEnabled = enabled;
}

bool ColorDetection::isMorphologyEnabled() const {
    return m_morphologyEnabled;
}

double ColorDetection::getLastDetectionTime() const {
    return m_lastDetectionTime;
}

int ColorDetection::getLastTargetCount() const {
    return m_lastTargetCount;
}

ColorRange ColorDetection::calculateColorRange(const QColor& color, int tolerance) {
    // Convert QColor to HSV
    int h, s, v;
    color.getHsv(&h, &s, &v);
    
    // Calculate tolerance ranges
    int hTolerance = tolerance * 180 / 100; // Hue range: 0-180 in OpenCV
    int svTolerance = tolerance * 255 / 100;
    
    // OpenCV uses H: 0-180, S: 0-255, V: 0-255
    int cvH = h / 2; // Qt uses 0-360, OpenCV uses 0-180
    
    ColorRange range;
    range.lower = cv::Scalar(
        std::max(0, cvH - hTolerance),
        std::max(0, s - svTolerance),
        std::max(0, v - svTolerance)
    );
    range.upper = cv::Scalar(
        std::min(180, cvH + hTolerance),
        std::min(255, s + svTolerance),
        std::min(255, v + svTolerance)
    );
    
    return range;
}

cv::Mat ColorDetection::createFOVMask(const cv::Size& frameSize, const QPoint& center, int radius) {
    cv::Mat mask = cv::Mat::zeros(frameSize, CV_8UC1);
    cv::circle(mask, cv::Point(center.x(), center.y()), radius, cv::Scalar(255), -1);
    return mask;
}

cv::Mat ColorDetection::applyMorphology(const cv::Mat& mask) {
    cv::Mat result;
    
    // Opening to remove noise
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
    cv::morphologyEx(mask, result, cv::MORPH_OPEN, kernel);
    
    // Closing to fill small holes
    cv::morphologyEx(result, result, cv::MORPH_CLOSE, kernel);
    
    return result;
}

std::vector<DetectedTarget> ColorDetection::findTargets(const cv::Mat& mask, const QPoint& screenCenter) {
    std::vector<DetectedTarget> targets;
    
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    for (const auto& contour : contours) {
        double area = cv::contourArea(contour);
        
        if (area < m_minArea || area > m_maxArea) {
            continue;
        }
        
        cv::Rect boundingRect = cv::boundingRect(contour);
        cv::Moments moments = cv::moments(contour);
        
        if (moments.m00 == 0) continue;
        
        int centerX = static_cast<int>(moments.m10 / moments.m00);
        int centerY = static_cast<int>(moments.m01 / moments.m00);
        
        double dx = centerX - screenCenter.x();
        double dy = centerY - screenCenter.y();
        double distance = std::sqrt(dx * dx + dy * dy);
        
        DetectedTarget target;
        target.center = QPoint(centerX, centerY);
        target.boundingBox = QRect(boundingRect.x, boundingRect.y, 
                                   boundingRect.width, boundingRect.height);
        target.area = area;
        target.distanceFromCenter = distance;
        target.confidence = calculateConfidence(area, distance);
        
        targets.push_back(target);
    }
    
    // Sort by distance from center
    std::sort(targets.begin(), targets.end(), 
              [](const DetectedTarget& a, const DetectedTarget& b) {
                  return a.distanceFromCenter < b.distanceFromCenter;
              });
    
    return targets;
}

double ColorDetection::calculateConfidence(double area, double distanceFromCenter) {
    // Higher area and lower distance = higher confidence
    double areaScore = std::min(area / 1000.0, 1.0);
    double distanceScore = std::max(0.0, 1.0 - (distanceFromCenter / m_fovRadius));
    
    return (areaScore * 0.4 + distanceScore * 0.6);
}

std::vector<DetectedTarget> ColorDetection::detect(const cv::Mat& frame, const QPoint& screenCenter) {
    QElapsedTimer timer;
    timer.start();
    
    if (frame.empty()) {
        m_lastDetectionTime = 0;
        m_lastTargetCount = 0;
        return {};
    }
    
    // Convert to HSV
    cv::Mat hsv;
    cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
    
    // Calculate color range
    ColorRange range = calculateColorRange(m_targetColor, m_colorTolerance);
    
    // Create color mask
    cv::Mat colorMask;
    cv::inRange(hsv, range.lower, range.upper, colorMask);
    
    // Apply FOV mask
    QPoint frameCenter(frame.cols / 2, frame.rows / 2);
    cv::Mat fovMask = createFOVMask(frame.size(), frameCenter, m_fovRadius);
    cv::bitwise_and(colorMask, fovMask, colorMask);
    
    // Apply morphology if enabled
    if (m_morphologyEnabled) {
        colorMask = applyMorphology(colorMask);
    }
    
    // Find targets
    std::vector<DetectedTarget> targets = findTargets(colorMask, frameCenter);
    
    m_lastDetectionTime = timer.elapsed();
    m_lastTargetCount = static_cast<int>(targets.size());
    
    emit detectionComplete(m_lastTargetCount, m_lastDetectionTime);
    
    if (!targets.empty()) {
        emit targetDetected(targets[0]);
    }
    
    return targets;
}
