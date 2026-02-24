#include "Database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QVariant>

Database::Database(QObject* parent)
    : QObject(parent)
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataPath);
    m_dbPath = dataPath + "/fastav.db";
    
    static int counter = 0;
    m_connectionName = QString("fastav_db_%1").arg(counter++);
}

Database::~Database() {
    if (m_db.isOpen()) {
        // Close all active queries
        m_db.commit(); // Commit any pending transaction
        m_db.close();
    }
    
    if (!m_connectionName.isEmpty()) {
        QSqlDatabase::removeDatabase(m_connectionName);
    }
}

bool Database::initialize() {
    m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    m_db.setDatabaseName(m_dbPath);
    
    if (!m_db.open()) {
        logError("initialize", m_db.lastError().text());
        return false;
    }
    
    qDebug() << "Database opened:" << m_dbPath;
    
    return createTables();
}

bool Database::createTables() {
    QSqlQuery query(m_db);
    
    // Create scan_history table
    QString createHistory = R"(
        CREATE TABLE IF NOT EXISTS scan_history (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            scan_date DATETIME NOT NULL,
            scan_path TEXT NOT NULL,
            files_scanned INTEGER DEFAULT 0,
            bytes_scanned INTEGER DEFAULT 0,
            threats_found INTEGER DEFAULT 0,
            scan_duration INTEGER DEFAULT 0
        )
    )";
    
    if (!query.exec(createHistory)) {
        logError("createTables - scan_history", query.lastError().text());
        return false;
    }
    
    // Create threats table
    QString createThreats = R"(
        CREATE TABLE IF NOT EXISTS threats (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            scan_id INTEGER NOT NULL,
            file_path TEXT NOT NULL,
            virus_name TEXT NOT NULL,
            file_size INTEGER DEFAULT 0,
            detection_time DATETIME NOT NULL,
            FOREIGN KEY (scan_id) REFERENCES scan_history(id) ON DELETE CASCADE
        )
    )";
    
    if (!query.exec(createThreats)) {
        logError("createTables - threats", query.lastError().text());
        return false;
    }
    
    // Enable foreign keys
    query.exec("PRAGMA foreign_keys = ON");
    
    qDebug() << "Database tables verified/created successfully";
    return true;
}

int Database::createScan(const QString& scanPath) {
    QMutexLocker locker(&m_mutex);
    
    QSqlQuery query(m_db);
    
    query.prepare("INSERT INTO scan_history (scan_date, scan_path) VALUES (?, ?)");
    query.addBindValue(QDateTime::currentDateTime());
    query.addBindValue(scanPath);
    
    if (!query.exec()) {
        logError("createScan", query.lastError().text());
        return -1;
    }
    
    int scanId = query.lastInsertId().toInt();
    qDebug() << "Created scan ID:" << scanId << "for path:" << scanPath;
    return scanId;
}

bool Database::updateScan(int scanId, quint64 filesScanned, quint64 bytesScanned, int threatsFound, qint64 duration) {
    QMutexLocker locker(&m_mutex);
    
    QSqlQuery query(m_db);
    
    query.prepare(R"(
        UPDATE scan_history 
        SET files_scanned = ?, bytes_scanned = ?, threats_found = ?, scan_duration = ?
        WHERE id = ?
    )");
    
    query.addBindValue(filesScanned);
    query.addBindValue(bytesScanned);
    query.addBindValue(threatsFound);
    query.addBindValue(duration);
    query.addBindValue(scanId);
    
    if (!query.exec()) {
        logError("updateScan", query.lastError().text());
        return false;
    }
    
    qDebug() << "Updated scan ID:" << scanId << "files:" << filesScanned << "threats:" << threatsFound;
    return true;
}

bool Database::addThreat(int scanId, const QString& filePath, const QString& virusName, quint64 fileSize) {
    QMutexLocker locker(&m_mutex);
    
    QSqlQuery query(m_db);
    
    query.prepare(R"(
        INSERT INTO threats (scan_id, file_path, virus_name, file_size, detection_time)
        VALUES (?, ?, ?, ?, ?)
    )");
    
    query.addBindValue(scanId);
    query.addBindValue(filePath);
    query.addBindValue(virusName);
    query.addBindValue(fileSize);
    query.addBindValue(QDateTime::currentDateTime());
    
    if (!query.exec()) {
        logError("addThreat", query.lastError().text());
        return false;
    }
    
    qDebug() << "Added threat:" << virusName << "in scan:" << scanId;
    return true;
}

QVector<ScanHistoryEntry> Database::getHistory(int limit) {
    QVector<ScanHistoryEntry> history;
    QSqlQuery query(m_db);
    
    // Only get scans with at least 1 file scanned
    query.prepare("SELECT * FROM scan_history WHERE files_scanned > 0 ORDER BY scan_date DESC LIMIT ?");
    query.addBindValue(limit);
    
    if (!query.exec()) {
        logError("getHistory", query.lastError().text());
        return history;
    }
    
    while (query.next()) {
        ScanHistoryEntry entry;
        entry.id = query.value("id").toInt();
        entry.scanDate = query.value("scan_date").toDateTime();
        entry.scanPath = query.value("scan_path").toString();
        entry.filesScanned = query.value("files_scanned").toULongLong();
        entry.bytesScanned = query.value("bytes_scanned").toULongLong();
        entry.threatsFound = query.value("threats_found").toInt();
        entry.scanDuration = query.value("scan_duration").toLongLong();
        history.append(entry);
    }
    
    return history;
}

ScanHistoryEntry Database::getLastScan() {
    ScanHistoryEntry entry;
    QSqlQuery query(m_db);
    
    query.exec("SELECT * FROM scan_history WHERE files_scanned > 0 ORDER BY scan_date DESC LIMIT 1");
    
    if (query.next()) {
        entry.id = query.value("id").toInt();
        entry.scanDate = query.value("scan_date").toDateTime();
        entry.scanPath = query.value("scan_path").toString();
        entry.filesScanned = query.value("files_scanned").toULongLong();
        entry.bytesScanned = query.value("bytes_scanned").toULongLong();
        entry.threatsFound = query.value("threats_found").toInt();
        entry.scanDuration = query.value("scan_duration").toLongLong();
    }
    
    return entry;
}

ThreatReport Database::getScanDetails(int scanId) {
    ThreatReport report;
    QSqlQuery query(m_db);
    
    // Get scan info
    query.prepare("SELECT * FROM scan_history WHERE id = ?");
    query.addBindValue(scanId);
    
    if (!query.exec() || !query.next()) {
        logError("getScanDetails - scan_history", query.lastError().text());
        return report;
    }
    
    report.setTotalFilesScanned(query.value("files_scanned").toULongLong());
    report.setTotalBytesScanned(query.value("bytes_scanned").toULongLong());
    report.setStartTime(query.value("scan_date").toDateTime());
    report.setScanDuration(query.value("scan_duration").toLongLong());
    
    // Get threats
    query.prepare("SELECT * FROM threats WHERE scan_id = ?");
    query.addBindValue(scanId);
    
    if (!query.exec()) {
        logError("getScanDetails - threats", query.lastError().text());
        return report;
    }
    
    while (query.next()) {
        QString path = query.value("file_path").toString();
        QString virus = query.value("virus_name").toString();
        quint64 size = query.value("file_size").toULongLong();
        report.addThreat(path, virus, size);
    }
    
    return report;
}

bool Database::deleteScan(int scanId) {
    QSqlQuery query(m_db);
    
    // Delete threats first (foreign key will cascade, but explicit is safer)
    query.prepare("DELETE FROM threats WHERE scan_id = ?");
    query.addBindValue(scanId);
    
    if (!query.exec()) {
        logError("deleteScan - threats", query.lastError().text());
        return false;
    }
    
    // Delete scan
    query.prepare("DELETE FROM scan_history WHERE id = ?");
    query.addBindValue(scanId);
    
    if (!query.exec()) {
        logError("deleteScan - scan_history", query.lastError().text());
        return false;
    }
    
    qDebug() << "Deleted scan ID:" << scanId;
    return true;
}

quint64 Database::getTotalScans() {
    QSqlQuery query(m_db);
    query.exec("SELECT COUNT(*) FROM scan_history WHERE files_scanned > 0");
    
    if (query.next()) {
        return query.value(0).toULongLong();
    }
    return 0;
}

quint64 Database::getTotalFilesScanned() {
    QSqlQuery query(m_db);
    query.exec("SELECT COALESCE(SUM(files_scanned), 0) FROM scan_history");
    
    if (query.next()) {
        return query.value(0).toULongLong();
    }
    return 0;
}

quint64 Database::getTotalThreatsFound() {
    QSqlQuery query(m_db);
    query.exec("SELECT COALESCE(SUM(threats_found), 0) FROM scan_history");
    
    if (query.next()) {
        return query.value(0).toULongLong();
    }
    return 0;
}

void Database::logError(const QString& context, const QString& error) {
    QString msg = QString("Database error in %1: %2").arg(context, error);
    qCritical() << msg;
    emit databaseError(msg);
}
