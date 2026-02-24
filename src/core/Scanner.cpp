#include "Scanner.h"
#include <QDir>
#include <QFileInfo>
#include <QDirIterator>
#include <QProcess>
#include <QDebug>
#include <QThread>
#include <QMetaObject>

// ScanTask implementation
ScanTask::ScanTask(const QString& filePath, Scanner* scanner)
    : m_filePath(filePath), m_scanner(scanner) {
    setAutoDelete(true);
}

QString ScanTask::scanWithClamdscan(const QString& path) {
    QProcess process;
    process.start("clamdscan", QStringList() << "--fdpass" << "--no-summary" << path);
    
    if (!process.waitForFinished(60000)) { // 60 second timeout
        return "TIMEOUT";
    }
    
    QString output = process.readAllStandardOutput();
    return output.trimmed();
}

void ScanTask::run() {
    QFileInfo info(m_filePath);
    quint64 fileSize = info.size();
    
    QString result = scanWithClamdscan(m_filePath);
    
    bool infected = false;
    QString virusName;
    
    if (result.contains("FOUND")) {
        infected = true;
        // Parse virus name from output like: "file: Virus.Name FOUND"
        int colonPos = result.indexOf(':');
        if (colonPos != -1) {
            virusName = result.mid(colonPos + 1).trimmed();
            virusName.remove(" FOUND");
        }
    }
    
    m_scanner->reportResult(m_filePath, infected, virusName, fileSize);
}

// Scanner implementation
Scanner::Scanner(Database* database, QObject* parent)
    : QObject(parent)
    , m_database(database)
    , m_currentScanId(-1)
    , m_threadPool(new QThreadPool(this))
    , m_isScanning(false)
    , m_filesScanned(0)
    , m_threatsFound(0)
    , m_bytesScanned(0)
    , m_totalFiles(0)
{
    m_threadPool->setMaxThreadCount(QThread::idealThreadCount());
}

Scanner::~Scanner() {
    stopScan();
}

void Scanner::reportResult(const QString& path, bool infected, const QString& virusName, quint64 fileSize) {
    // Safety check: ignore results if not scanning anymore
    if (!m_isScanning.load()) {
        return;
    }
    
    m_filesScanned++;
    m_bytesScanned += fileSize;
    
    if (infected) {
        m_threatsFound++;
        
        // Save to database (thread-safe: Database has internal mutex)
        if (m_database && m_currentScanId >= 0) {
            m_database->addThreat(m_currentScanId, path, virusName, fileSize);
        }
        
        emit fileScanned(path, true, virusName);
    } else {
        emit fileScanned(path, false, QString());
    }
    
    emit scanProgress(m_filesScanned.load(), m_totalFiles.load());
    
    // Check if scan complete
    if (m_filesScanned >= m_totalFiles) {
        bool expected = true;
        if (m_isScanning.compare_exchange_strong(expected, false)) {
            // Scan complete - emit signal immediately
            // Database update will happen when thread pool is done
            QMetaObject::invokeMethod(this, [this]() {
                finalizeScan();
            }, Qt::QueuedConnection);
        }
    }
}

void Scanner::startScan(const QStringList& paths) {
    if (m_isScanning.load()) {
        emit scanError("Scan already in progress");
        return;
    }
    
    if (!m_database) {
        emit scanError("Database not initialized");
        return;
    }
    
    // Reset counters
    m_filesScanned = 0;
    m_threatsFound = 0;
    m_bytesScanned = 0;
    m_scanStartTime = QDateTime::currentDateTime();
    
    // Create scan record
    m_currentScanId = m_database->createScan(paths.join(", "));
    if (m_currentScanId < 0) {
        emit scanError("Cannot create scan record in database");
        return;
    }
    
    // Expand paths
    QStringList allFiles = expandPaths(paths);
    m_totalFiles = allFiles.size();
    
    if (m_totalFiles == 0) {
        emit scanError("No files to scan");
        return;
    }
    
    m_isScanning = true;
    emit scanStarted(m_totalFiles);
    
    // Queue all tasks
    for (const QString& file : allFiles) {
        ScanTask* task = new ScanTask(file, this);
        m_threadPool->start(task);
    }
}

void Scanner::stopScan() {
    m_isScanning = false;
    m_threadPool->clear();
    m_threadPool->waitForDone();
}

QStringList Scanner::expandPaths(const QStringList& paths) {
    QStringList files;
    
    for (const QString& path : paths) {
        QFileInfo info(path);
        
        if (info.isFile()) {
            files.append(info.absoluteFilePath());
        } else if (info.isDir()) {
            QDirIterator it(path, QDir::Files | QDir::NoDotAndDotDot, 
                           QDirIterator::Subdirectories);
            while (it.hasNext()) {
                files.append(it.next());
            }
        }
    }
    
    return files;
}

void Scanner::finalizeScan() {
    // Wait for all threads to finish before touching database
    m_threadPool->waitForDone();
    
    // Now it's safe to update database from main thread
    qint64 duration = m_scanStartTime.secsTo(QDateTime::currentDateTime());
    
    if (m_database && m_currentScanId >= 0) {
        m_database->updateScan(m_currentScanId, m_filesScanned.load(), 
                              m_bytesScanned.load(), m_threatsFound.load(), duration);
    }
    
    // Create report
    ThreatReport report;
    report.setTotalFilesScanned(m_filesScanned.load());
    report.setTotalBytesScanned(m_bytesScanned.load());
    report.setStartTime(m_scanStartTime);
    report.setEndTime(QDateTime::currentDateTime());
    report.setScanDuration(duration);
    
    emit scanCompleted(report);
}
