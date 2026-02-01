#ifndef STATSTRACKER_H
#define STATSTRACKER_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QElapsedTimer>

struct SessionStats {
    qint64 startTime;
    qint64 endTime;
    int targetsDetected;
    int assistsApplied;
    double avgFPS;
    int peakFPS;
};

class StatsTracker : public QObject {
    Q_OBJECT

public:
    explicit StatsTracker(QObject* parent = nullptr);
    ~StatsTracker();

    // Session control
    void startSession();
    void endSession();
    bool isSessionActive() const;

    // Recording stats
    void recordTargetDetected();
    void recordAssistApplied();
    void recordFPS(double fps);

    // Current session stats
    int getSessionTargets() const;
    int getSessionAssists() const;
    qint64 getSessionDuration() const;
    double getSessionAvgFPS() const;

    // Lifetime stats
    int getTotalTargets() const;
    int getTotalAssists() const;
    qint64 getTotalRuntime() const;
    int getTotalSessions() const;

    // Stats display
    QString getFormattedSessionTime() const;
    QString getFormattedTotalTime() const;
    QString getStatsReport() const;

    // Persistence
    bool loadStats();
    bool saveStats();
    void resetStats();

signals:
    void statsUpdated();
    void sessionStarted();
    void sessionEnded(const SessionStats& stats);

private:
    // Current session
    bool m_sessionActive;
    QElapsedTimer m_sessionTimer;
    int m_sessionTargets;
    int m_sessionAssists;
    double m_sessionFPSSum;
    int m_sessionFPSCount;
    int m_sessionPeakFPS;

    // Lifetime stats
    int m_totalTargets;
    int m_totalAssists;
    qint64 m_totalRuntime;
    int m_totalSessions;

    QString getStatsFilePath() const;
    QString formatDuration(qint64 ms) const;
};

#endif // STATSTRACKER_H
