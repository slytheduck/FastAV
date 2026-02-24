#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QVector>
#include <QSqlDatabase>
#include <QMutex>
#include "ThreatReport.h"

struct ScanHistoryEntry {
    int id;
    QDateTime scanDate;
    QString scanPath;
    quint64 filesScanned;
    quint64 bytesScanned;
    int threatsFound;
    qint64 scanDuration;
};

class Database : public QObject {
    Q_OBJECT
public:
    explicit Database(QObject* parent = nullptr);
    ~Database();

    bool initialize();
    
    // Scan management - simplified
    int createScan(const QString& scanPath);
    bool updateScan(int scanId, quint64 filesScanned, quint64 bytesScanned, int threatsFound, qint64 duration);
    bool addThreat(int scanId, const QString& filePath, const QString& virusName, quint64 fileSize);
    
    // History
    QVector<ScanHistoryEntry> getHistory(int limit = 50);
    ScanHistoryEntry getLastScan();
    ThreatReport getScanDetails(int scanId);
    bool deleteScan(int scanId);
    
    // Statistics
    quint64 getTotalScans();
    quint64 getTotalFilesScanned();
    quint64 getTotalThreatsFound();

signals:
    void databaseError(const QString& error);

private:
    bool createTables();
    void logError(const QString& context, const QString& error);
    
    QSqlDatabase m_db;
    QString m_dbPath;
    QString m_connectionName;
    QMutex m_mutex;  // Protect all database operations
};

#endif // DATABASE_H
