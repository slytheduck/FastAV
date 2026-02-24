#ifndef THREATREPORT_H
#define THREATREPORT_H

#include <QString>
#include <QDateTime>
#include <QVector>
#include <QMetaType>

struct ThreatInfo {
    QString filePath;
    QString virusName;
    quint64 fileSize;
    QDateTime detectionTime;
    
    ThreatInfo() : fileSize(0) {}
    ThreatInfo(const QString& path, const QString& virus, quint64 size)
        : filePath(path), virusName(virus), fileSize(size)
        , detectionTime(QDateTime::currentDateTime()) {}
};

class ThreatReport {
public:
    ThreatReport();
    
    void addThreat(const QString& path, const QString& virusName, quint64 fileSize);
    
    // Getters
    QVector<ThreatInfo> getThreats() const { return m_threats; }
    int getThreatCount() const { return m_threats.size(); }
    quint64 getTotalFilesScanned() const { return m_totalFilesScanned; }
    quint64 getTotalBytesScanned() const { return m_totalBytesScanned; }
    QDateTime getStartTime() const { return m_startTime; }
    QDateTime getEndTime() const { return m_endTime; }
    qint64 getScanDuration() const { return m_scanDuration; }
    
    // Setters
    void setTotalFilesScanned(quint64 total) { m_totalFilesScanned = total; }
    void setTotalBytesScanned(quint64 total) { m_totalBytesScanned = total; }
    void setStartTime(const QDateTime& time) { m_startTime = time; }
    void setEndTime(const QDateTime& time) { m_endTime = time; }
    void setScanDuration(qint64 seconds) { m_scanDuration = seconds; }
    
    // Summary
    QString getSummary() const;
    QString getFormattedSize(quint64 bytes) const;
    
private:
    QVector<ThreatInfo> m_threats;
    quint64 m_totalFilesScanned;
    quint64 m_totalBytesScanned;
    QDateTime m_startTime;
    QDateTime m_endTime;
    qint64 m_scanDuration; // in seconds
};

Q_DECLARE_METATYPE(ThreatReport)

#endif // THREATREPORT_H
