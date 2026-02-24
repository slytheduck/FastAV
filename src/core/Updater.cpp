#include "Updater.h"
#include <QSettings>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

Updater::Updater(QObject* parent)
    : QObject(parent)
    , m_process(nullptr)
    , m_isUpdating(false)
{
    // Try to read from freshclam.log
    QFile logFile("/var/log/clamav/freshclam.log");
    if (logFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&logFile);
        QDateTime lastUpdate;
        
        // Read all lines and find the most recent "ClamAV update process started at"
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.contains("ClamAV update process started at")) {
                // Extract date: "ClamAV update process started at Thu Feb 13 09:32:51 2026"
                int atPos = line.indexOf(" at ");
                if (atPos != -1) {
                    QString dateStr = line.mid(atPos + 4).trimmed();
                    // Parse format: "ddd MMM d HH:mm:ss yyyy"
                    QDateTime dt = QDateTime::fromString(dateStr, "ddd MMM d HH:mm:ss yyyy");
                    if (dt.isValid()) {
                        lastUpdate = dt;
                    }
                }
            }
        }
        logFile.close();
        
        if (lastUpdate.isValid()) {
            m_lastUpdateTime = lastUpdate;
        }
    }
    
    // Fallback to QSettings if log not readable
    if (!m_lastUpdateTime.isValid()) {
        QSettings settings("FastAV", "FastAV");
        m_lastUpdateTime = settings.value("lastUpdateTime").toDateTime();
    }
}

Updater::~Updater() {
    if (m_process) {
        m_process->kill();
        m_process->deleteLater();
    }
}

QDateTime Updater::getLastUpdateTime() const {
    return m_lastUpdateTime;
}

void Updater::checkForUpdates() {
    // Check if freshclam is available
    if (m_isUpdating) {
        emit updateError("Update already in progress");
        return;
    }
    
    // In a real implementation, you would check the version
    // For now, we'll just trigger an update
    emit updateAvailable("Latest");
}

void Updater::updateDatabase() {
    if (m_isUpdating) {
        emit updateError("Update already in progress");
        return;
    }
    
    m_isUpdating = true;
    emit updateStarted();
    
    m_process = new QProcess(this);
    connect(m_process, &QProcess::readyReadStandardOutput, 
            this, &Updater::onProcessOutput);
    connect(m_process, &QProcess::readyReadStandardError,
            this, &Updater::onProcessOutput);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &Updater::onProcessFinished);
    
    // Try different methods to run freshclam with privileges
    QStringList attempts;
    
#ifdef Q_OS_UNIX
    // Method 1: Try pkexec (PolicyKit - GUI prompt)
    if (QFile::exists("/usr/bin/pkexec")) {
        attempts << "pkexec";
    }
    
    // Method 2: Try sudo (may fail if no password cache)
    if (QFile::exists("/usr/bin/sudo")) {
        attempts << "sudo";
    }
    
    // Method 3: Try running freshclam directly (works if user is in clamav group)
    attempts << "";
#else
    // Windows: run freshclam directly
    attempts << "";
#endif
    
    bool started = false;
    QString lastError;
    
    for (const QString& method : attempts) {
        QStringList args;
        QString program;
        
        if (method.isEmpty()) {
            // Direct execution
            program = "freshclam";
            args << "--verbose";
        } else {
            // With privilege escalation
            program = method;
            args << "freshclam" << "--verbose";
        }
        
        emit updateProgress(QString("Trying to update with %1...").arg(
            method.isEmpty() ? "direct execution" : method));
        
        m_process->start(program, args);
        
        if (m_process->waitForStarted(3000)) {
            started = true;
            break;
        } else {
            lastError = m_process->errorString();
        }
    }
    
    if (!started) {
        m_isUpdating = false;
        QString errorMsg = "Cannot start freshclam. Try running manually:\n"
                          "sudo freshclam\n\n"
                          "Or add your user to the clamav group:\n"
                          "sudo usermod -a -G clamav $USER\n"
                          "Then log out and back in.\n\n"
                          "Last error: " + lastError;
        emit updateError(errorMsg);
        m_process->deleteLater();
        m_process = nullptr;
    }
}

void Updater::onProcessOutput() {
    if (!m_process) return;
    
    QString output = QString::fromUtf8(m_process->readAllStandardOutput());
    QString error = QString::fromUtf8(m_process->readAllStandardError());
    
    if (!output.isEmpty()) {
        emit updateProgress(output.trimmed());
    }
    if (!error.isEmpty()) {
        emit updateProgress(error.trimmed());
    }
}

void Updater::onProcessFinished(int exitCode, QProcess::ExitStatus status) {
    m_isUpdating = false;
    
    if (status == QProcess::NormalExit && exitCode == 0) {
        m_lastUpdateTime = QDateTime::currentDateTime();
        
        QSettings settings("FastAV", "FastAV");
        settings.setValue("lastUpdateTime", m_lastUpdateTime);
        
        emit updateCompleted(true);
    } else {
        emit updateCompleted(false);
        emit updateError("Update failed with exit code: " + QString::number(exitCode));
    }
    
    if (m_process) {
        m_process->deleteLater();
        m_process = nullptr;
    }
}
