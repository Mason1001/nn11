#include "utils/StatsTracker.h"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QStandardPaths>

StatsTracker::StatsTracker(QObject* parent)
    : QObject(parent)
    , m_sessionActive(false)
    , m_sessionTargets(0)
    , m_sessionAssists(0)
    , m_sessionFPSSum(0.0)
    , m_sessionFPSCount(0)
    , m_sessionPeakFPS(0)
    , m_totalTargets(0)
    , m_totalAssists(0)
    , m_totalRuntime(0)
    , m_totalSessions(0)
{
    loadStats();
}

StatsTracker::~StatsTracker() {
    if (m_sessionActive) {
        endSession();
    }
    saveStats();
}

void StatsTracker::startSession() {
    if (m_sessionActive) {
        return;
    }
    
    m_sessionActive = true;
    m_sessionTargets = 0;
    m_sessionAssists = 0;
    m_sessionFPSSum = 0.0;
    m_sessionFPSCount = 0;
    m_sessionPeakFPS = 0;
    
    m_sessionTimer.start();
    
    emit sessionStarted();
}

void StatsTracker::endSession() {
    if (!m_sessionActive) {
        return;
    }
    
    m_sessionActive = false;
    
    // Update lifetime stats
    m_totalTargets += m_sessionTargets;
    m_totalAssists += m_sessionAssists;
    m_totalRuntime += m_sessionTimer.elapsed();
    m_totalSessions++;
    
    // Create session stats
    SessionStats stats;
    stats.startTime = 0; // Would need to track actual start time
    stats.endTime = 0;
    stats.targetsDetected = m_sessionTargets;
    stats.assistsApplied = m_sessionAssists;
    stats.avgFPS = getSessionAvgFPS();
    stats.peakFPS = m_sessionPeakFPS;
    
    saveStats();
    
    emit sessionEnded(stats);
}

bool StatsTracker::isSessionActive() const {
    return m_sessionActive;
}

void StatsTracker::recordTargetDetected() {
    if (m_sessionActive) {
        m_sessionTargets++;
        emit statsUpdated();
    }
}

void StatsTracker::recordAssistApplied() {
    if (m_sessionActive) {
        m_sessionAssists++;
        emit statsUpdated();
    }
}

void StatsTracker::recordFPS(double fps) {
    if (m_sessionActive) {
        m_sessionFPSSum += fps;
        m_sessionFPSCount++;
        
        int intFPS = static_cast<int>(fps);
        if (intFPS > m_sessionPeakFPS) {
            m_sessionPeakFPS = intFPS;
        }
    }
}

int StatsTracker::getSessionTargets() const {
    return m_sessionTargets;
}

int StatsTracker::getSessionAssists() const {
    return m_sessionAssists;
}

qint64 StatsTracker::getSessionDuration() const {
    if (m_sessionActive) {
        return m_sessionTimer.elapsed();
    }
    return 0;
}

double StatsTracker::getSessionAvgFPS() const {
    if (m_sessionFPSCount > 0) {
        return m_sessionFPSSum / m_sessionFPSCount;
    }
    return 0.0;
}

int StatsTracker::getTotalTargets() const {
    return m_totalTargets + (m_sessionActive ? m_sessionTargets : 0);
}

int StatsTracker::getTotalAssists() const {
    return m_totalAssists + (m_sessionActive ? m_sessionAssists : 0);
}

qint64 StatsTracker::getTotalRuntime() const {
    qint64 total = m_totalRuntime;
    if (m_sessionActive) {
        total += m_sessionTimer.elapsed();
    }
    return total;
}

int StatsTracker::getTotalSessions() const {
    return m_totalSessions + (m_sessionActive ? 1 : 0);
}

QString StatsTracker::getFormattedSessionTime() const {
    return formatDuration(getSessionDuration());
}

QString StatsTracker::getFormattedTotalTime() const {
    return formatDuration(getTotalRuntime());
}

QString StatsTracker::formatDuration(qint64 ms) const {
    int hours = ms / 3600000;
    int minutes = (ms % 3600000) / 60000;
    int seconds = (ms % 60000) / 1000;
    
    return QString("%1:%2:%3")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'));
}

QString StatsTracker::getStatsReport() const {
    QString report;
    report += QString("=== Session Stats ===\n");
    report += QString("Duration: %1\n").arg(getFormattedSessionTime());
    report += QString("Targets: %1\n").arg(m_sessionTargets);
    report += QString("Assists: %1\n").arg(m_sessionAssists);
    report += QString("Avg FPS: %1\n").arg(getSessionAvgFPS(), 0, 'f', 1);
    report += QString("Peak FPS: %1\n").arg(m_sessionPeakFPS);
    report += QString("\n=== Lifetime Stats ===\n");
    report += QString("Total Runtime: %1\n").arg(getFormattedTotalTime());
    report += QString("Total Sessions: %1\n").arg(getTotalSessions());
    report += QString("Total Targets: %1\n").arg(getTotalTargets());
    report += QString("Total Assists: %1\n").arg(getTotalAssists());
    
    return report;
}

QString StatsTracker::getStatsFilePath() const {
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(appDataPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return appDataPath + "/stats.json";
}

bool StatsTracker::loadStats() {
    QFile file(getStatsFilePath());
    if (!file.exists()) {
        return false;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        return false;
    }
    
    QJsonObject obj = doc.object();
    m_totalTargets = obj["totalTargets"].toInt(0);
    m_totalAssists = obj["totalAssists"].toInt(0);
    m_totalRuntime = obj["totalRuntime"].toVariant().toLongLong();
    m_totalSessions = obj["totalSessions"].toInt(0);
    
    return true;
}

bool StatsTracker::saveStats() {
    QJsonObject obj;
    obj["totalTargets"] = m_totalTargets;
    obj["totalAssists"] = m_totalAssists;
    obj["totalRuntime"] = static_cast<qint64>(m_totalRuntime);
    obj["totalSessions"] = m_totalSessions;
    
    QJsonDocument doc(obj);
    
    QFile file(getStatsFilePath());
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    return true;
}

void StatsTracker::resetStats() {
    m_totalTargets = 0;
    m_totalAssists = 0;
    m_totalRuntime = 0;
    m_totalSessions = 0;
    saveStats();
}
