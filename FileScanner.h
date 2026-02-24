#ifndef FILESCANNER_H
#define FILESCANNER_H

#include <QString>
#include <QStringList>

class FileScanner {
public:
    static QStringList getCommonLocations();
    static QString formatFileSize(quint64 bytes);
    static QString formatDuration(qint64 seconds);
    static QString getFileIcon(const QString& filePath);
};

#endif // FILESCANNER_H
