#include "MainWindow.h"
#include "ScanDialog.h"
#include "ScanProgress.h"
#include "HistoryViewer.h"
#include "../utils/MaterialTheme.h"
#include "../utils/FileScanner.h"
#include <QMessageBox>
#include <QFrame>
#include <QGridLayout>
#include <QStatusBar>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_database(new Database(this))
    , m_scanner(nullptr)
    , m_updater(new Updater(this))
{
    setWindowTitle("FastAV - Modern Antivirus Scanner");
    setMinimumSize(900, 700);
    
    // Initialize database FIRST
    if (!m_database->initialize()) {
        QMessageBox::warning(this, "Database Error", 
            "Could not initialize database. Statistics will not be saved.");
    }
    
    // NOW create Scanner with initialized database
    m_scanner = new Scanner(m_database, this);
    
    // Connect updater signals
    connect(m_updater, &Updater::updateCompleted, this, &MainWindow::onUpdateCompleted);
    connect(m_updater, &Updater::updateProgress, this, &MainWindow::onUpdateProgress);
    
    setupUI();
    updateStats();
}

MainWindow::~MainWindow() {
    // CRITICAL: Stop scanner threads before database is destroyed
    if (m_scanner) {
        m_scanner->stopScan();
    }
}

void MainWindow::setupUI() {
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(40, 40, 40, 40);
    m_mainLayout->setSpacing(30);
    
    createWelcomeScreen();
    createStatsCard();
    
    m_mainLayout->addStretch();
}

void MainWindow::createWelcomeScreen() {
    // Title
    m_titleLabel = new QLabel("FastAV", this);
    QFont titleFont;
    titleFont.setPointSize(48);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setStyleSheet(QString("color: %1;").arg(MaterialTheme::Primary.name()));
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_mainLayout->addWidget(m_titleLabel);
    
    // Subtitle
    m_subtitleLabel = new QLabel("Modern & Lightning-Fast Antivirus Scanner", this);
    QFont subtitleFont;
    subtitleFont.setPointSize(16);
    m_subtitleLabel->setFont(subtitleFont);
    m_subtitleLabel->setStyleSheet("color: #AAAAAA; margin-bottom: 20px;");
    m_subtitleLabel->setAlignment(Qt::AlignCenter);
    m_mainLayout->addWidget(m_subtitleLabel);
    
    // Last scan info
    m_lastScanLabel = new QLabel("", this);
    m_lastScanLabel->setAlignment(Qt::AlignCenter);
    m_lastScanLabel->setStyleSheet("color: #888888; font-size: 14px;");
    m_mainLayout->addWidget(m_lastScanLabel);
    
    // Main action buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(20);
    buttonLayout->addStretch();
    
    m_newScanButton = new QPushButton("New Scan", this);
    m_newScanButton->setStyleSheet(MaterialTheme::getButtonStyle());
    m_newScanButton->setMinimumSize(180, 60);
    m_newScanButton->setCursor(Qt::PointingHandCursor);
    QFont buttonFont;
    buttonFont.setPointSize(14);
    buttonFont.setBold(true);
    m_newScanButton->setFont(buttonFont);
    connect(m_newScanButton, &QPushButton::clicked, this, &MainWindow::onNewScanClicked);
    buttonLayout->addWidget(m_newScanButton);
    
    buttonLayout->addStretch();
    m_mainLayout->addLayout(buttonLayout);
    
    // Secondary buttons
    QHBoxLayout* secondaryLayout = new QHBoxLayout();
    secondaryLayout->setSpacing(15);
    secondaryLayout->addStretch();
    
    m_historyButton = new QPushButton("View History", this);
    m_historyButton->setStyleSheet(QString(R"(
        QPushButton {
            background-color: transparent;
            color: %1;
            border: 2px solid %1;
            border-radius: 8px;
            padding: 10px 20px;
            font-size: 13px;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: %2;
        }
    )").arg(MaterialTheme::Primary.name())
       .arg(MaterialTheme::Primary.darker(150).name()));
    m_historyButton->setCursor(Qt::PointingHandCursor);
    connect(m_historyButton, &QPushButton::clicked, this, &MainWindow::onViewHistoryClicked);
    secondaryLayout->addWidget(m_historyButton);
    
    m_updateButton = new QPushButton("Update Database", this);
    m_updateButton->setStyleSheet(QString(R"(
        QPushButton {
            background-color: transparent;
            color: %1;
            border: 2px solid %1;
            border-radius: 8px;
            padding: 10px 20px;
            font-size: 13px;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: %2;
        }
    )").arg(MaterialTheme::Secondary.name())
       .arg(MaterialTheme::Secondary.darker(150).name()));
    m_updateButton->setCursor(Qt::PointingHandCursor);
    connect(m_updateButton, &QPushButton::clicked, this, &MainWindow::onUpdateDatabaseClicked);
    secondaryLayout->addWidget(m_updateButton);
    
    secondaryLayout->addStretch();
    m_mainLayout->addLayout(secondaryLayout);
}

void MainWindow::createStatsCard() {
    QFrame* statsFrame = new QFrame(this);
    statsFrame->setStyleSheet(MaterialTheme::getCardStyle());
    statsFrame->setMaximumWidth(700);
    
    QGridLayout* statsLayout = new QGridLayout(statsFrame);
    statsLayout->setSpacing(20);
    
    // Stats title
    QLabel* statsTitle = new QLabel("Statistics", statsFrame);
    QFont statsTitleFont;
    statsTitleFont.setPointSize(18);
    statsTitleFont.setBold(true);
    statsTitle->setFont(statsTitleFont);
    statsLayout->addWidget(statsTitle, 0, 0, 1, 2);
    
    // Create stat items with proper object names
    auto createStatWidget = [this](const QString& label, const QString& initialValue, const QString& objectName) {
        QFrame* statFrame = new QFrame();
        QVBoxLayout* layout = new QVBoxLayout(statFrame);
        layout->setSpacing(5);
        
        QLabel* valueLabel = new QLabel(initialValue);
        valueLabel->setObjectName(objectName); // Set object name for finding later
        QFont valueFont;
        valueFont.setPointSize(24);
        valueFont.setBold(true);
        valueLabel->setFont(valueFont);
        valueLabel->setStyleSheet(QString("color: %1;").arg(MaterialTheme::Primary.name()));
        
        QLabel* labelLabel = new QLabel(label);
        labelLabel->setStyleSheet("color: #888888; font-size: 12px;");
        
        layout->addWidget(valueLabel);
        layout->addWidget(labelLabel);
        
        return statFrame;
    };
    
    // Create stats widgets with object names
    QWidget* totalScansWidget = createStatWidget("Total Scans", "0", "totalScans");
    QWidget* totalFilesWidget = createStatWidget("Files Scanned", "0", "totalFiles");
    QWidget* totalThreatsWidget = createStatWidget("Threats Found", "0", "totalThreats");
    QWidget* lastUpdateWidget = createStatWidget("Last Update", "Never", "lastUpdate");
    
    statsLayout->addWidget(totalScansWidget, 1, 0);
    statsLayout->addWidget(totalFilesWidget, 1, 1);
    statsLayout->addWidget(totalThreatsWidget, 2, 0);
    statsLayout->addWidget(lastUpdateWidget, 2, 1);
    
    QHBoxLayout* statsContainer = new QHBoxLayout();
    statsContainer->addStretch();
    statsContainer->addWidget(statsFrame);
    statsContainer->addStretch();
    
    m_mainLayout->addLayout(statsContainer);
}

void MainWindow::updateStats() {
    quint64 totalScans = m_database->getTotalScans();
    quint64 totalFiles = m_database->getTotalFilesScanned();
    quint64 totalThreats = m_database->getTotalThreatsFound();
    
    // Update last scan info
    ScanHistoryEntry lastScan = m_database->getLastScan();
    if (lastScan.id > 0) {
        m_lastScanLabel->setText(QString("Last scan: %1 - %2 files, %3 threats")
            .arg(lastScan.scanDate.toString("MMM dd, yyyy hh:mm"))
            .arg(lastScan.filesScanned)
            .arg(lastScan.threatsFound));
    } else {
        m_lastScanLabel->setText("No previous scans");
    }
    
    // Update last database update time
    QDateTime lastUpdate = m_updater->getLastUpdateTime();
    QString updateText = lastUpdate.isValid() ? 
        lastUpdate.toString("MMM dd, yyyy") : "Never";
    
    // Find and update stat labels by object name
    QLabel* totalScansLabel = findChild<QLabel*>("totalScans");
    if (totalScansLabel) {
        totalScansLabel->setText(QString::number(totalScans));
    }
    
    QLabel* totalFilesLabel = findChild<QLabel*>("totalFiles");
    if (totalFilesLabel) {
        totalFilesLabel->setText(QString::number(totalFiles));
    }
    
    QLabel* totalThreatsLabel = findChild<QLabel*>("totalThreats");
    if (totalThreatsLabel) {
        totalThreatsLabel->setText(QString::number(totalThreats));
    }
    
    QLabel* lastUpdateLabel = findChild<QLabel*>("lastUpdate");
    if (lastUpdateLabel) {
        lastUpdateLabel->setText(updateText);
    }
}

void MainWindow::onNewScanClicked() {
    ScanDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QStringList paths = dialog.getSelectedPaths();
        if (!paths.isEmpty()) {
            ScanProgress progressDialog(m_scanner, m_database, paths, this);
            progressDialog.exec();
            updateStats();
        }
    }
}

void MainWindow::onViewHistoryClicked() {
    HistoryViewer* viewer = new HistoryViewer(m_database, this);
    viewer->exec();
}

void MainWindow::onUpdateDatabaseClicked() {
    m_updateButton->setEnabled(false);
    m_updateButton->setText("Updating...");
    m_updater->updateDatabase();
}

void MainWindow::onAboutClicked() {
    QMessageBox::about(this, "About FastAV",
        "<h2>FastAV v1.0</h2>"
        "<p>Modern and lightning-fast antivirus scanner powered by ClamAV.</p>"
        "<p>Built with Qt6 and Material Design 3.</p>"
        "<p>Copyright © 2024</p>");
}

void MainWindow::onUpdateCompleted(bool success) {
    m_updateButton->setEnabled(true);
    m_updateButton->setText("Update Database");
    
    if (success) {
        QMessageBox::information(this, "Update Complete",
            "Virus database updated successfully!");
        updateStats();
    } else {
        QMessageBox::warning(this, "Update Failed",
            "Failed to update virus database. Please check your internet connection.");
    }
}

void MainWindow::onUpdateProgress(const QString& message) {
    statusBar()->showMessage(message, 3000);
}
