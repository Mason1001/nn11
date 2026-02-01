#ifndef TRACKER_H
#define TRACKER_H

#include <QObject>
#include <QTimer>
#include <QPoint>
#include <QElapsedTimer>
#include <memory>
#include <vector>
#include <algorithm>
#include "ScreenCapture.h"
#include "ColorDetection.h"
#include "MouseController.h"

class Tracker : public QObject {
    Q_OBJECT

public:
    explicit Tracker(QObject* parent = nullptr);
    ~Tracker();

    // Control
    void start();
    void stop();
    void toggle();
    bool isRunning() const;

    // Components access
    ScreenCapture* screenCapture() const;
    ColorDetection* colorDetection() const;
    MouseController* mouseController() const;

    // Settings
    void setTargetFPS(int fps);
    int getTargetFPS() const;

    void setEnabled(bool enabled);
    bool isEnabled() const;

    // Stats
    double getCurrentFPS() const;
    int getTotalTargetsDetected() const;
    int getTotalAssists() const;
    qint64 getRunningTimeMs() const;

signals:
    void started();
    void stopped();
    void fpsUpdated(double fps);
    void targetFound(const QPoint& position);
    void assistApplied(const QPoint& from, const QPoint& to);
    void statsUpdated(double fps, int targets, int assists);

private slots:
    void onTrackerTick();
    void updateStats();

private:
    std::unique_ptr<ScreenCapture> m_screenCapture;
    std::unique_ptr<ColorDetection> m_colorDetection;
    std::unique_ptr<MouseController> m_mouseController;

    QTimer* m_trackerTimer;
    QTimer* m_statsTimer;
    QElapsedTimer m_runningTimer;
    QElapsedTimer m_frameTimer;

    bool m_isRunning;
    bool m_isEnabled;
    int m_targetFPS;

    // Stats
    double m_currentFPS;
    int m_frameCount;
    int m_totalTargetsDetected;
    int m_totalAssists;
    qint64 m_totalRunningTime;

    void processFrame();
    DetectedTarget selectBestTarget(const std::vector<DetectedTarget>& targets);
};

#endif // TRACKER_H
