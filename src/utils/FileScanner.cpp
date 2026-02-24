#include "FileScanner.h"
#include <QStandardPaths>
#include <QDir>

QStringList FileScanner::getCommonLocations() {
    QStringList locations;
    
    // Home directory
    locations << QDir::homePath();
    
    // Common directories
    locations << QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    locations << QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    locations << QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    
    // Remove empty entries
    locations.removeAll(QString());
    
    return locations;
}

QString FileScanner::formatFileSize(quint64 bytes) {
    const quint64 KB = 1024;
    const quint64 MB = KB * 1024;
    const quint64 GB = MB * 1024;
    const quint64 TB = GB * 1024;
    
    if (bytes >= TB) {
        return QString::number(bytes / (double)TB, 'f', 2) + " TB";
    } else if (bytes >= GB) {
        return QString::number(bytes / (double)GB, 'f', 2) + " GB";
    } else if (bytes >= MB) {
        return QString::number(bytes / (double)MB, 'f', 2) + " MB";
    } else if (bytes >= KB) {
        return QString::number(bytes / (double)KB, 'f', 2) + " KB";
    } else {
        return QString::number(bytes) + " B";
    }
}

QString FileScanner::formatDuration(qint64 seconds) {
    if (seconds < 60) {
        return QString::number(seconds) + " sec";
    } else if (seconds < 3600) {
        int mins = seconds / 60;
        int secs = seconds % 60;
        return QString("%1m %2s").arg(mins).arg(secs);
    } else {
        int hours = seconds / 3600;
        int mins = (seconds % 3600) / 60;
        return QString("%1h %2m").arg(hours).arg(mins);
    }
}

QString FileScanner::getFileIcon(const QString& filePath) {
    // Simple icon mapping based on extension
    QString ext = filePath.section('.', -1).toLower();
    
    if (ext == "exe" || ext == "dll" || ext == "so") return "⚙️";
    if (ext == "pdf") return "📄";
    if (ext == "doc" || ext == "docx") return "📝";
    if (ext == "zip" || ext == "rar" || ext == "7z") return "📦";
    if (ext == "jpg" || ext == "png" || ext == "gif") return "🖼️";
    if (ext == "mp3" || ext == "wav") return "🎵";
    if (ext == "mp4" || ext == "avi") return "🎬";
    
    return "📁";
}
