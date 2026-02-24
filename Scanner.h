#ifndef SCANNER_H
#define SCANNER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QThreadPool>
#include <QRunnable>
#include <QMutex>
#include <QDateTime>
#include <atomic>
#include "ThreatReport.h"
#include "Database.h"

class Scanner;

class ScanTask : public QRunnable {
public:
    ScanTask(const QString& filePath, Scanner* scanner);
    void run() override;

private:
    QString m_filePath;
    Scanner* m_scanner;
    QString scanWithClamdscan(const QString& path);
};

class Scanner : public QObject {
    Q_OBJECT
public:
    explicit Scanner(Database* database, QObject* parent = nullptr);
    ~Scanner();

    void startScan(const QStringList& paths);
    void stopScan();
    bool isScanning() const { return m_isScanning.load(); }
    
    quint64 getFilesScanned() const { return m_filesScanned.load(); }
    quint64 getThreatsFound() const { return m_threatsFound.load(); }
    quint64 getBytesScanned() const { return m_bytesScanned.load(); }
    
    void reportResult(const QString& path, bool infected, const QString& virusName, quint64 fileSize);

private slots:
    void finalizeScan();

signals:
    void scanStarted(quint64 totalFiles);
    void fileScanned(const QString& path, bool infected, const QString& virusName);
    void scanProgress(quint64 scanned, quint64 total);
    void scanCompleted(const ThreatReport& report);
    void scanError(const QString& error);

private:
    QStringList expandPaths(const QStringList& paths);
    
    Database* m_database;
    int m_currentScanId;
    QThreadPool* m_threadPool;
    
    std::atomic<bool> m_isScanning;
    std::atomic<quint64> m_filesScanned;
    std::atomic<quint64> m_threatsFound;
    std::atomic<quint64> m_bytesScanned;
    std::atomic<quint64> m_totalFiles;
    
    QDateTime m_scanStartTime;
};

#endif // SCANNER_H
