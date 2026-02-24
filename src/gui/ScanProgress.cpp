#include "ScanProgress.h"
#include "ThreatViewer.h"
#include "../utils/MaterialTheme.h"
#include "../utils/FileScanner.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QFileInfo>
#include <QTimer>

ScanProgress::ScanProgress(Scanner* scanner, Database* database,
                           const QStringList& paths, QWidget* parent)
    : QDialog(parent)
    , m_scanner(scanner)
    , m_database(database)
    , m_scanPaths(paths)
    , m_scanCompleted(false)
{
    setWindowTitle("Scanning in Progress");
    setMinimumSize(800, 600);
    setModal(true);
    
    setupUI();
    
    // Connect scanner signals
    connect(m_scanner, &Scanner::scanStarted, this, &ScanProgress::onScanStarted);
    connect(m_scanner, &Scanner::scanProgress, this, &ScanProgress::onScanProgress);
    connect(m_scanner, &Scanner::fileScanned, this, &ScanProgress::onFileScanned);
    connect(m_scanner, &Scanner::scanCompleted, this, &ScanProgress::onScanCompleted);
    
    // Stats timer
    m_statsTimer = new QTimer(this);
    connect(m_statsTimer, &QTimer::timeout, this, &ScanProgress::updateStats);
    m_statsTimer->start(500);
    
    // Start scan
    m_startTime = QDateTime::currentDateTime();
    m_scanner->startScan(paths);
}

void ScanProgress::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    
    // Title
    QLabel* title = new QLabel("Scanning Files", this);
    QFont titleFont;
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setStyleSheet(QString("color: %1;").arg(MaterialTheme::Primary.name()));
    mainLayout->addWidget(title);
    
    // Progress bar
    m_progressBar = new QProgressBar(this);
    m_progressBar->setStyleSheet(MaterialTheme::getProgressBarStyle());
    m_progressBar->setMinimumHeight(12);
    m_progressBar->setTextVisible(false);
    mainLayout->addWidget(m_progressBar);
    
    // Status
    m_statusLabel = new QLabel("Initializing scan...", this);
    m_statusLabel->setStyleSheet("color: #AAAAAA; font-size: 14px;");
    mainLayout->addWidget(m_statusLabel);
    
    // Stats group
    QGroupBox* statsGroup = new QGroupBox("Statistics", this);
    statsGroup->setStyleSheet(QString(R"(
        QGroupBox {
            font-size: 14px;
            font-weight: bold;
            border: 2px solid #2A2A2A;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 15px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px;
        }
    )"));
    
    QGridLayout* statsLayout = new QGridLayout(statsGroup);
    statsLayout->setSpacing(15);
    
    auto createStatRow = [](const QString& label, QLabel** valueLabel) {
        QHBoxLayout* layout = new QHBoxLayout();
        
        QLabel* labelWidget = new QLabel(label + ":");
        labelWidget->setStyleSheet("color: #888888; font-size: 13px;");
        
        *valueLabel = new QLabel("0");
        QFont valueFont;
        valueFont.setBold(true);
        valueFont.setPointSize(13);
        (*valueLabel)->setFont(valueFont);
        (*valueLabel)->setStyleSheet(QString("color: %1;").arg(MaterialTheme::Primary.name()));
        
        layout->addWidget(labelWidget);
        layout->addStretch();
        layout->addWidget(*valueLabel);
        
        return layout;
    };
    
    statsLayout->addLayout(createStatRow("Files Scanned", &m_filesScannedLabel), 0, 0);
    statsLayout->addLayout(createStatRow("Threats Found", &m_threatsFoundLabel), 0, 1);
    statsLayout->addLayout(createStatRow("Scan Speed", &m_speedLabel), 1, 0);
    
    mainLayout->addWidget(statsGroup);
    
    // Current file
    QLabel* currentLabel = new QLabel("Current File:", this);
    currentLabel->setStyleSheet("color: #888888; font-size: 12px; margin-top: 10px;");
    mainLayout->addWidget(currentLabel);
    
    m_currentFileLabel = new QLabel("...", this);
    m_currentFileLabel->setStyleSheet("color: #FFFFFF; font-size: 12px; font-family: monospace;");
    m_currentFileLabel->setWordWrap(true);
    mainLayout->addWidget(m_currentFileLabel);
    
    // Log area
    m_logText = new QTextEdit(this);
    m_logText->setReadOnly(true);
    m_logText->setStyleSheet(QString(R"(
        QTextEdit {
            background-color: %1;
            border: 2px solid #2A2A2A;
            border-radius: 8px;
            padding: 10px;
            font-family: monospace;
            font-size: 11px;
        }
    )").arg(MaterialTheme::Surface.name()));
    m_logText->setMaximumHeight(150);
    mainLayout->addWidget(m_logText);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_cancelButton = new QPushButton("Cancel", this);
    m_cancelButton->setStyleSheet(QString(R"(
        QPushButton {
            background-color: %1;
            color: %2;
            border: none;
            border-radius: 8px;
            padding: 10px 30px;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: %3;
        }
    )").arg(MaterialTheme::Error.name())
       .arg(MaterialTheme::OnError.name())
       .arg(MaterialTheme::Error.lighter(110).name()));
    m_cancelButton->setCursor(Qt::PointingHandCursor);
    connect(m_cancelButton, &QPushButton::clicked, this, &ScanProgress::onCancelClicked);
    
    m_closeButton = new QPushButton("Close", this);
    m_closeButton->setStyleSheet(MaterialTheme::getButtonStyle());
    m_closeButton->setCursor(Qt::PointingHandCursor);
    m_closeButton->setVisible(false);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_closeButton);
    
    mainLayout->addLayout(buttonLayout);
}

void ScanProgress::onScanStarted(quint64 totalFiles) {
    m_progressBar->setMaximum(totalFiles);
    m_statusLabel->setText(QString("Scanning %1 files...").arg(totalFiles));
    m_logText->append(QString("[INFO] Scan started - %1 files to scan").arg(totalFiles));
}

void ScanProgress::onScanProgress(quint64 scanned, quint64 total) {
    m_progressBar->setValue(scanned);
    
    int percentage = total > 0 ? (scanned * 100) / total : 0;
    m_statusLabel->setText(QString("Progress: %1% (%2 / %3 files)")
        .arg(percentage)
        .arg(scanned)
        .arg(total));
}

void ScanProgress::onFileScanned(const QString& path, bool infected, const QString& virusName) {
    m_currentFileLabel->setText(path);
    
    if (infected) {
        QString logEntry = QString("[THREAT] %1: %2").arg(virusName).arg(path);
        m_logText->append(QString("<span style='color: %1;'>%2</span>")
            .arg(MaterialTheme::Error.name())
            .arg(logEntry));
        m_logText->ensureCursorVisible();
    }
}

void ScanProgress::onScanCompleted(const ThreatReport& report) {
    m_scanCompleted = true;
    m_statsTimer->stop();
    
    // Database already updated by Scanner, no need to save again
    
    // Update UI
    m_statusLabel->setText("Scan completed!");
    m_cancelButton->setVisible(false);
    m_closeButton->setVisible(true);
    
    quint64 threatsFound = m_scanner ? m_scanner->getThreatsFound() : 0;
    
    QString summary = QString("[COMPLETED] Scan finished in %1\n")
        .arg(FileScanner::formatDuration(report.getScanDuration()));
    summary += QString("[STATS] Files: %1, Threats: %2, Data: %3")
        .arg(report.getTotalFilesScanned())
        .arg(threatsFound)
        .arg(report.getFormattedSize(report.getTotalBytesScanned()));
    
    m_logText->append(QString("<span style='color: %1;'>%2</span>")
        .arg(MaterialTheme::Success.name())
        .arg(summary));
    
    // Show detailed results if threats found
    if (threatsFound > 0) {
        QMessageBox::information(this, "Scan Complete",
            QString("Scan completed!\n\n"
                   "Files scanned: %1\n"
                   "Threats found: %2\n"
                   "Scan duration: %3\n\n"
                   "View details in Scan History.")
                   .arg(report.getTotalFilesScanned())
                   .arg(threatsFound)
                   .arg(FileScanner::formatDuration(report.getScanDuration())));
    } else {
        QMessageBox::information(this, "Scan Complete",
            QString("No threats detected!\n\n"
                   "Files scanned: %1\n"
                   "Scan duration: %2")
                   .arg(report.getTotalFilesScanned())
                   .arg(FileScanner::formatDuration(report.getScanDuration())));
    }
}

void ScanProgress::onCancelClicked() {
    if (QMessageBox::question(this, "Cancel Scan",
        "Are you sure you want to cancel the scan?") == QMessageBox::Yes) {
        m_scanner->stopScan();
        m_logText->append(QString("<span style='color: %1;'>[CANCELLED] Scan cancelled by user</span>")
            .arg(MaterialTheme::Warning.name()));
        reject();
    }
}

void ScanProgress::updateStats() {
    if (!m_scanner) return;
    
    if (!m_scanner->isScanning() && !m_scanCompleted) {
        return;
    }
    
    m_filesScannedLabel->setText(QString::number(m_scanner->getFilesScanned()));
    m_threatsFoundLabel->setText(QString::number(m_scanner->getThreatsFound()));
    
    qint64 elapsed = m_startTime.secsTo(QDateTime::currentDateTime());
    if (elapsed > 0) {
        double speed = m_scanner->getFilesScanned() / (double)elapsed;
        m_speedLabel->setText(QString::number(speed, 'f', 1) + " files/s");
    }
}
