#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>
#include <QProcess>
#include <QDateTime>
#include <QTimer>

class Updater : public QObject {
    Q_OBJECT
public:
    explicit Updater(QObject* parent = nullptr);
    ~Updater();
    
    void checkForUpdates();
    void updateDatabase();
    QDateTime getLastUpdateTime() const;
    bool isUpdating() const { return m_isUpdating; }
    
signals:
    void updateStarted();
    void updateProgress(const QString& message);
    void updateCompleted(bool success);
    void updateAvailable(const QString& version);
    void updateError(const QString& error);
    
private slots:
    void onProcessOutput();
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);
    
private:
    QProcess* m_process;
    bool m_isUpdating;
    QDateTime m_lastUpdateTime;
};

#endif // UPDATER_H
