#ifndef MOUSECONTROLLER_H
#define MOUSECONTROLLER_H

#include <QObject>
#include <QPoint>
#include <QTimer>
#include <vector>
#include <random>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#endif

struct BezierPoint {
    double x;
    double y;
};

class MouseController : public QObject {
    Q_OBJECT

public:
    explicit MouseController(QObject* parent = nullptr);
    ~MouseController();

    // Mouse movement
    void moveToTarget(const QPoint& target);
    void moveSmoothly(const QPoint& target, int durationMs);
    void moveRelative(int dx, int dy);

    // Aim assist
    void setAimAssistStrength(int strength); // 0-100
    int getAimAssistStrength() const;

    void setResponseSpeed(int speed); // 0-100
    int getResponseSpeed() const;

    void applyAimAssist(const QPoint& targetPos);

    // Mouse state
    QPoint getCurrentPosition() const;
    bool isMoving() const;

    // Click simulation
    void leftClick();
    void rightClick();
    void leftDown();
    void leftUp();

    // Human-like behavior settings
    void setHumanizeEnabled(bool enabled);
    bool isHumanizeEnabled() const;

    void setRandomizationFactor(double factor); // 0.0 - 1.0
    double getRandomizationFactor() const;

signals:
    void movementStarted();
    void movementCompleted();
    void positionChanged(const QPoint& pos);

private:
    int m_aimAssistStrength;
    int m_responseSpeed;
    bool m_isMoving;
    bool m_humanizeEnabled;
    double m_randomizationFactor;
    QTimer* m_movementTimer;
    std::vector<BezierPoint> m_currentPath;
    int m_pathIndex;
    QPoint m_targetPosition;

    std::mt19937 m_rng;
    std::uniform_real_distribution<double> m_distribution;

    // Bezier curve generation
    std::vector<BezierPoint> generateBezierPath(const QPoint& start, const QPoint& end, int steps);
    BezierPoint bezierPoint(double t, const BezierPoint& p0, const BezierPoint& p1, 
                            const BezierPoint& p2, const BezierPoint& p3);

    // Human-like movement
    void addMicroMovements(std::vector<BezierPoint>& path);
    double getRandomDelay();
    QPoint addRandomOffset(const QPoint& point, double maxOffset);

    // Platform-specific movement
    void platformMove(int x, int y);
    void platformMoveRelative(int dx, int dy);

private slots:
    void onMovementTick();
};

#endif // MOUSECONTROLLER_H
