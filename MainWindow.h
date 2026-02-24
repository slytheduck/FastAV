#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "../core/Scanner.h"
#include "../core/Database.h"
#include "../core/Updater.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    
private slots:
    void onNewScanClicked();
    void onViewHistoryClicked();
    void onUpdateDatabaseClicked();
    void onAboutClicked();
    
    void onUpdateCompleted(bool success);
    void onUpdateProgress(const QString& message);
    
private:
    void setupUI();
    void createWelcomeScreen();
    void createStatsCard();
    void updateStats();
    
    // Core components - CRITICAL ORDER: destroyed in reverse!
    Database* m_database;   // Declared first = destroyed LAST
    Scanner* m_scanner;     // Declared second = destroyed FIRST (stops threads)
    Updater* m_updater;
    
    // UI components
    QWidget* m_centralWidget;
    QVBoxLayout* m_mainLayout;
    
    QLabel* m_titleLabel;
    QLabel* m_subtitleLabel;
    QLabel* m_lastScanLabel;
    
    QPushButton* m_newScanButton;
    QPushButton* m_historyButton;
    QPushButton* m_updateButton;
    QPushButton* m_aboutButton;
    
    // Stats
    QLabel* m_totalScansLabel;
    QLabel* m_totalFilesLabel;
    QLabel* m_totalThreatsLabel;
    QLabel* m_lastUpdateLabel;
};

#endif // MAINWINDOW_H
