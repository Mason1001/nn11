#include "core/MouseController.h"
#include <QCursor>
#include <QGuiApplication>
#include <QScreen>
#include <algorithm>
#include <cmath>
#include <chrono>

MouseController::MouseController(QObject* parent)
    : QObject(parent)
    , m_aimAssistStrength(30)
    , m_responseSpeed(50)
    , m_isMoving(false)
    , m_humanizeEnabled(true)
    , m_randomizationFactor(0.3)
    , m_pathIndex(0)
    , m_rng(std::chrono::steady_clock::now().time_since_epoch().count())
    , m_distribution(0.0, 1.0)
{
    m_movementTimer = new QTimer(this);
    m_movementTimer->setInterval(1); // 1ms for smooth movement
    connect(m_movementTimer, &QTimer::timeout, this, &MouseController::onMovementTick);
}

MouseController::~MouseController() {
    if (m_movementTimer->isActive()) {
        m_movementTimer->stop();
    }
}

void MouseController::setAimAssistStrength(int strength) {
    m_aimAssistStrength = std::clamp(strength, 0, 100);
}

int MouseController::getAimAssistStrength() const {
    return m_aimAssistStrength;
}

void MouseController::setResponseSpeed(int speed) {
    m_responseSpeed = std::clamp(speed, 0, 100);
}

int MouseController::getResponseSpeed() const {
    return m_responseSpeed;
}

void MouseController::setHumanizeEnabled(bool enabled) {
    m_humanizeEnabled = enabled;
}

bool MouseController::isHumanizeEnabled() const {
    return m_humanizeEnabled;
}

void MouseController::setRandomizationFactor(double factor) {
    m_randomizationFactor = std::clamp(factor, 0.0, 1.0);
}

double MouseController::getRandomizationFactor() const {
    return m_randomizationFactor;
}

QPoint MouseController::getCurrentPosition() const {
    return QCursor::pos();
}

bool MouseController::isMoving() const {
    return m_isMoving;
}

double MouseController::getRandomDelay() {
    // Calculate base delay from response speed
    // 0% = 1000ms, 100% = 0ms
    double baseDelay = 1000.0 * (1.0 - m_responseSpeed / 100.0);
    
    // Add randomization (±30%)
    double variation = baseDelay * m_randomizationFactor * (m_distribution(m_rng) * 2.0 - 1.0);
    
    return std::max(0.0, baseDelay + variation);
}

QPoint MouseController::addRandomOffset(const QPoint& point, double maxOffset) {
    if (!m_humanizeEnabled || maxOffset <= 0) {
        return point;
    }
    
    double offsetX = (m_distribution(m_rng) * 2.0 - 1.0) * maxOffset;
    double offsetY = (m_distribution(m_rng) * 2.0 - 1.0) * maxOffset;
    
    return QPoint(point.x() + static_cast<int>(offsetX),
                  point.y() + static_cast<int>(offsetY));
}

std::vector<BezierPoint> MouseController::generateBezierPath(const QPoint& start, const QPoint& end, int steps) {
    std::vector<BezierPoint> path;
    
    BezierPoint p0 = {static_cast<double>(start.x()), static_cast<double>(start.y())};
    BezierPoint p3 = {static_cast<double>(end.x()), static_cast<double>(end.y())};
    
    // Generate control points with some randomness for natural movement
    double dx = p3.x - p0.x;
    double dy = p3.y - p0.y;
    double distance = std::sqrt(dx * dx + dy * dy);
    
    // Control point offset based on distance
    double offsetRange = distance * 0.3;
    
    BezierPoint p1 = {
        p0.x + dx * 0.3 + (m_distribution(m_rng) * 2.0 - 1.0) * offsetRange,
        p0.y + dy * 0.3 + (m_distribution(m_rng) * 2.0 - 1.0) * offsetRange
    };
    
    BezierPoint p2 = {
        p0.x + dx * 0.7 + (m_distribution(m_rng) * 2.0 - 1.0) * offsetRange,
        p0.y + dy * 0.7 + (m_distribution(m_rng) * 2.0 - 1.0) * offsetRange
    };
    
    for (int i = 0; i <= steps; ++i) {
        double t = static_cast<double>(i) / steps;
        path.push_back(bezierPoint(t, p0, p1, p2, p3));
    }
    
    return path;
}

BezierPoint MouseController::bezierPoint(double t, const BezierPoint& p0, const BezierPoint& p1,
                                          const BezierPoint& p2, const BezierPoint& p3) {
    double u = 1.0 - t;
    double tt = t * t;
    double uu = u * u;
    double uuu = uu * u;
    double ttt = tt * t;
    
    BezierPoint p;
    p.x = uuu * p0.x + 3 * uu * t * p1.x + 3 * u * tt * p2.x + ttt * p3.x;
    p.y = uuu * p0.y + 3 * uu * t * p1.y + 3 * u * tt * p2.y + ttt * p3.y;
    
    return p;
}

void MouseController::addMicroMovements(std::vector<BezierPoint>& path) {
    if (!m_humanizeEnabled || path.empty()) {
        return;
    }
    
    for (size_t i = 1; i < path.size() - 1; ++i) {
        // Small random micro-movements (1-2 pixels)
        if (m_distribution(m_rng) < 0.3) {
            path[i].x += (m_distribution(m_rng) * 2.0 - 1.0) * 2.0;
            path[i].y += (m_distribution(m_rng) * 2.0 - 1.0) * 2.0;
        }
    }
}

void MouseController::moveToTarget(const QPoint& target) {
    platformMove(target.x(), target.y());
    emit positionChanged(target);
}

void MouseController::moveSmoothly(const QPoint& target, int durationMs) {
    if (m_isMoving) {
        m_movementTimer->stop();
    }
    
    QPoint start = getCurrentPosition();
    
    // Calculate number of steps based on duration and timer interval
    int steps = std::max(10, durationMs / m_movementTimer->interval());
    
    m_currentPath = generateBezierPath(start, target, steps);
    
    if (m_humanizeEnabled) {
        addMicroMovements(m_currentPath);
    }
    
    m_pathIndex = 0;
    m_targetPosition = target;
    m_isMoving = true;
    
    emit movementStarted();
    m_movementTimer->start();
}

void MouseController::moveRelative(int dx, int dy) {
    platformMoveRelative(dx, dy);
    emit positionChanged(getCurrentPosition());
}

void MouseController::applyAimAssist(const QPoint& targetPos) {
    if (m_aimAssistStrength == 0) {
        return;
    }
    
    QPoint currentPos = getCurrentPosition();
    
    // Calculate direction to target
    int dx = targetPos.x() - currentPos.x();
    int dy = targetPos.y() - currentPos.y();
    
    // Apply aim assist strength (0-100%)
    double strength = m_aimAssistStrength / 100.0;
    
    int assistDx = static_cast<int>(dx * strength);
    int assistDy = static_cast<int>(dy * strength);
    
    // Add slight randomization for natural feel
    if (m_humanizeEnabled) {
        QPoint offset = addRandomOffset(QPoint(0, 0), 2.0);
        assistDx += offset.x();
        assistDy += offset.y();
    }
    
    // Calculate movement duration based on response speed
    double delay = getRandomDelay();
    int duration = static_cast<int>(std::max(10.0, delay * 0.5));
    
    QPoint newTarget(currentPos.x() + assistDx, currentPos.y() + assistDy);
    
    if (duration > 20) {
        moveSmoothly(newTarget, duration);
    } else {
        moveRelative(assistDx, assistDy);
    }
}

void MouseController::onMovementTick() {
    if (m_pathIndex >= static_cast<int>(m_currentPath.size())) {
        m_movementTimer->stop();
        m_isMoving = false;
        emit movementCompleted();
        return;
    }
    
    const BezierPoint& point = m_currentPath[m_pathIndex];
    platformMove(static_cast<int>(point.x), static_cast<int>(point.y));
    
    emit positionChanged(QPoint(static_cast<int>(point.x), static_cast<int>(point.y)));
    
    ++m_pathIndex;
}

void MouseController::platformMove(int x, int y) {
#ifdef _WIN32
    SetCursorPos(x, y);
#else
    QCursor::setPos(x, y);
#endif
}

void MouseController::platformMoveRelative(int dx, int dy) {
#ifdef _WIN32
    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dx = dx;
    input.mi.dy = dy;
    input.mi.dwFlags = MOUSEEVENTF_MOVE;
    SendInput(1, &input, sizeof(INPUT));
#else
    QPoint current = QCursor::pos();
    QCursor::setPos(current.x() + dx, current.y() + dy);
#endif
}

void MouseController::leftClick() {
#ifdef _WIN32
    INPUT inputs[2] = {};
    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(2, inputs, sizeof(INPUT));
#endif
}

void MouseController::rightClick() {
#ifdef _WIN32
    INPUT inputs[2] = {};
    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dwFlags = MOUSEEVENTF_RIGHTUP;
    SendInput(2, inputs, sizeof(INPUT));
#endif
}

void MouseController::leftDown() {
#ifdef _WIN32
    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &input, sizeof(INPUT));
#endif
}

void MouseController::leftUp() {
#ifdef _WIN32
    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &input, sizeof(INPUT));
#endif
}
