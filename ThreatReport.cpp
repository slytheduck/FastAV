#include "ThreatReport.h"

ThreatReport::ThreatReport()
    : m_totalFilesScanned(0)
    , m_totalBytesScanned(0)
    , m_scanDuration(0)
{
    m_startTime = QDateTime::currentDateTime();
}

void ThreatReport::addThreat(const QString& path, const QString& virusName, quint64 fileSize) {
    m_threats.append(ThreatInfo(path, virusName, fileSize));
}

QString ThreatReport::getFormattedSize(quint64 bytes) const {
    const quint64 KB = 1024;
    const quint64 MB = KB * 1024;
    const quint64 GB = MB * 1024;
    
    if (bytes >= GB) {
        return QString::number(bytes / (double)GB, 'f', 2) + " GB";
    } else if (bytes >= MB) {
        return QString::number(bytes / (double)MB, 'f', 2) + " MB";
    } else if (bytes >= KB) {
        return QString::number(bytes / (double)KB, 'f', 2) + " KB";
    } else {
        return QString::number(bytes) + " bytes";
    }
}

QString ThreatReport::getSummary() const {
    QString summary;
    summary += QString("Scan completed in %1 seconds\n").arg(m_scanDuration);
    summary += QString("Files scanned: %1\n").arg(m_totalFilesScanned);
    summary += QString("Data scanned: %1\n").arg(getFormattedSize(m_totalBytesScanned));
    summary += QString("Threats found: %1\n").arg(m_threats.size());
    
    if (!m_threats.isEmpty()) {
        summary += "\nDetected threats:\n";
        for (const ThreatInfo& threat : m_threats) {
            summary += QString("  • %1: %2 (%3)\n")
                .arg(threat.virusName)
                .arg(threat.filePath)
                .arg(getFormattedSize(threat.fileSize));
        }
    }
    
    return summary;
}
