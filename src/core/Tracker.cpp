#include "core/Tracker.h"
#include <algorithm>
#include <cstdlib>

Tracker::Tracker(QObject* parent)
    : QObject(parent)
    , m_screenCapture(std::make_unique<ScreenCapture>())
    , m_colorDetection(std::make_unique<ColorDetection>())
    , m_mouseController(std::make_unique<MouseController>())
    , m_isRunning(false)
    , m_isEnabled(true)
    , m_targetFPS(144)
    , m_currentFPS(0.0)
    , m_frameCount(0)
    , m_totalTargetsDetected(0)
    , m_totalAssists(0)
    , m_totalRunningTime(0)
{
    m_trackerTimer = new QTimer(this);
    connect(m_trackerTimer, &QTimer::timeout, this, &Tracker::onTrackerTick);
    
    m_statsTimer = new QTimer(this);
    m_statsTimer->setInterval(1000); // Update stats every second
    connect(m_statsTimer, &QTimer::timeout, this, &Tracker::updateStats);
}

Tracker::~Tracker() {
    stop();
}

void Tracker::start() {
    if (m_isRunning) {
        return;
    }
    
    m_isRunning = true;
    m_frameCount = 0;
    
    // Calculate timer interval from target FPS
    int interval = std::max(1, 1000 / m_targetFPS);
    m_trackerTimer->setInterval(interval);
    
    m_frameTimer.start();
    m_runningTimer.start();
    
    m_trackerTimer->start();
    m_statsTimer->start();
    
    emit started();
}

void Tracker::stop() {
    if (!m_isRunning) {
        return;
    }
    
    m_isRunning = false;
    m_totalRunningTime += m_runningTimer.elapsed();
    
    m_trackerTimer->stop();
    m_statsTimer->stop();
    
    emit stopped();
}

void Tracker::toggle() {
    if (m_isRunning) {
        stop();
    } else {
        start();
    }
}

bool Tracker::isRunning() const {
    return m_isRunning;
}

ScreenCapture* Tracker::screenCapture() const {
    return m_screenCapture.get();
}

ColorDetection* Tracker::colorDetection() const {
    return m_colorDetection.get();
}

MouseController* Tracker::mouseController() const {
    return m_mouseController.get();
}

void Tracker::setTargetFPS(int fps) {
    m_targetFPS = std::clamp(fps, 30, 300);
    
    if (m_isRunning) {
        int interval = std::max(1, 1000 / m_targetFPS);
        m_trackerTimer->setInterval(interval);
    }
}

int Tracker::getTargetFPS() const {
    return m_targetFPS;
}

void Tracker::setEnabled(bool enabled) {
    m_isEnabled = enabled;
}

bool Tracker::isEnabled() const {
    return m_isEnabled;
}

double Tracker::getCurrentFPS() const {
    return m_currentFPS;
}

int Tracker::getTotalTargetsDetected() const {
    return m_totalTargetsDetected;
}

int Tracker::getTotalAssists() const {
    return m_totalAssists;
}

qint64 Tracker::getRunningTimeMs() const {
    if (m_isRunning) {
        return m_totalRunningTime + m_runningTimer.elapsed();
    }
    return m_totalRunningTime;
}

void Tracker::onTrackerTick() {
    if (!m_isEnabled) {
        return;
    }
    
    processFrame();
    ++m_frameCount;
}

void Tracker::processFrame() {
    // Capture screen
    cv::Mat frame = m_screenCapture->capture();
    
    if (frame.empty()) {
        return;
    }
    
    // Get screen center
    QPoint screenCenter = m_screenCapture->getScreenCenter();
    
    // Detect targets
    std::vector<DetectedTarget> targets = m_colorDetection->detect(frame, screenCenter);
    
    if (targets.empty()) {
        return;
    }
    
    m_totalTargetsDetected += static_cast<int>(targets.size());
    
    // Select best target
    DetectedTarget bestTarget = selectBestTarget(targets);
    
    emit targetFound(bestTarget.center);
    
    // Apply aim assist if mouse controller has strength > 0
    if (m_mouseController->getAimAssistStrength() > 0) {
        QPoint currentPos = m_mouseController->getCurrentPosition();
        m_mouseController->applyAimAssist(bestTarget.center);
        ++m_totalAssists;
        
        emit assistApplied(currentPos, bestTarget.center);
    }
}

DetectedTarget Tracker::selectBestTarget(const std::vector<DetectedTarget>& targets) {
    // Already sorted by distance, so first target is closest
    // But we also consider confidence
    
    if (targets.size() == 1) {
        return targets[0];
    }
    
    // Find target with best combination of distance and confidence
    auto best = std::max_element(targets.begin(), targets.end(),
        [](const DetectedTarget& a, const DetectedTarget& b) {
            // Score = confidence - normalized distance penalty
            double scoreA = a.confidence - (a.distanceFromCenter / 500.0) * 0.5;
            double scoreB = b.confidence - (b.distanceFromCenter / 500.0) * 0.5;
            return scoreA < scoreB;
        });
    
    return *best;
}

void Tracker::updateStats() {
    // Calculate FPS
    qint64 elapsed = m_frameTimer.elapsed();
    if (elapsed > 0) {
        m_currentFPS = (m_frameCount * 1000.0) / elapsed;
    }
    
    m_frameCount = 0;
    m_frameTimer.restart();
    
    emit fpsUpdated(m_currentFPS);
    emit statsUpdated(m_currentFPS, m_totalTargetsDetected, m_totalAssists);
}
