#ifndef SCANPROGRESS_H
#define SCANPROGRESS_H

#include <QDialog>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QTimer>
#include "../core/Scanner.h"
#include "../core/Database.h"
#include "../core/ThreatReport.h"

class ScanProgress : public QDialog {
    Q_OBJECT
    
public:
    explicit ScanProgress(Scanner* scanner, Database* database, 
                         const QStringList& paths, QWidget* parent = nullptr);
    
private slots:
    void onScanStarted(quint64 totalFiles);
    void onScanProgress(quint64 scanned, quint64 total);
    void onFileScanned(const QString& path, bool infected, const QString& virusName);
    void onScanCompleted(const ThreatReport& report);
    void onCancelClicked();
    void updateStats();
    
private:
    void setupUI();
    void showResults(const ThreatReport& report);
    
    Scanner* m_scanner;
    Database* m_database;
    QStringList m_scanPaths;
    
    QProgressBar* m_progressBar;
    QLabel* m_statusLabel;
    QLabel* m_filesScannedLabel;
    QLabel* m_threatsFoundLabel;
    QLabel* m_speedLabel;
    QLabel* m_currentFileLabel;
    QTextEdit* m_logText;
    QPushButton* m_cancelButton;
    QPushButton* m_closeButton;
    
    QTimer* m_statsTimer;
    QDateTime m_startTime;
    bool m_scanCompleted;
};

#endif // SCANPROGRESS_H
