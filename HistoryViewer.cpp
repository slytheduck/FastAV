#include "HistoryViewer.h"
#include "ThreatViewer.h"
#include "../utils/MaterialTheme.h"
#include "../utils/FileScanner.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>
#include <QSet>

HistoryViewer::HistoryViewer(Database* database, QWidget* parent)
    : QDialog(parent)
    , m_database(database)
{
    setWindowTitle("Scan History");
    setMinimumSize(900, 600);
    setupUI();
    loadHistory();
    
    // Connect double-click signal
    connect(m_historyTable, &QTableWidget::cellDoubleClicked,
            this, &HistoryViewer::onRowDoubleClicked);
}

void HistoryViewer::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    
    // Title
    QLabel* title = new QLabel("Scan History", this);
    QFont titleFont;
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setStyleSheet(QString("color: %1;").arg(MaterialTheme::Primary.name()));
    mainLayout->addWidget(title);
    
    // History table
    m_historyTable = new QTableWidget(this);
    m_historyTable->setColumnCount(6);
    m_historyTable->setHorizontalHeaderLabels({
        "Date", "Scan Path", "Files", "Data Scanned", "Threats", "Duration"
    });
    
    m_historyTable->setStyleSheet(QString(R"(
        QTableWidget {
            background-color: %1;
            border: none;
            border-radius: 8px;
            gridline-color: #2A2A2A;
        }
        QTableWidget::item {
            padding: 10px;
            border-bottom: 1px solid #2A2A2A;
        }
        QTableWidget::item:selected {
            background-color: %2;
        }
        QHeaderView::section {
            background-color: #2A2A2A;
            color: %3;
            padding: 12px;
            border: none;
            font-weight: bold;
            font-size: 13px;
        }
    )").arg(MaterialTheme::Surface.name())
       .arg(MaterialTheme::Primary.darker(150).name())
       .arg(MaterialTheme::OnSurface.name()));
    
    m_historyTable->horizontalHeader()->setStretchLastSection(true);
    m_historyTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_historyTable->verticalHeader()->setVisible(false);
    m_historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_historyTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_historyTable->setAlternatingRowColors(true);
    
    mainLayout->addWidget(m_historyTable);
    
    // Hint label
    QLabel* hintLabel = new QLabel("💡 Tip: Double-click on any scan to view detailed report", this);
    hintLabel->setStyleSheet("color: #888888; font-size: 12px; font-style: italic; padding: 5px;");
    mainLayout->addWidget(hintLabel);
    
    // Close button
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    QPushButton* deleteButton = new QPushButton("Delete Selected", this);
    deleteButton->setStyleSheet(MaterialTheme::getButtonStyle());
    deleteButton->setCursor(Qt::PointingHandCursor);
    deleteButton->setMinimumWidth(150);
    connect(deleteButton, &QPushButton::clicked, this, &HistoryViewer::onDeleteClicked);
    buttonLayout->addWidget(deleteButton);
    
    m_closeButton = new QPushButton("Close", this);
    m_closeButton->setStyleSheet(MaterialTheme::getButtonStyle());
    m_closeButton->setCursor(Qt::PointingHandCursor);
    m_closeButton->setMinimumWidth(120);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    
    buttonLayout->addWidget(m_closeButton);
    
    mainLayout->addLayout(buttonLayout);
}

void HistoryViewer::loadHistory() {
    m_historyData = m_database->getHistory(100);
    m_historyTable->setRowCount(m_historyData.size());
    
    for (int i = 0; i < m_historyData.size(); ++i) {
        const ScanHistoryEntry& entry = m_historyData[i];
        
        // Date
        m_historyTable->setItem(i, 0, new QTableWidgetItem(
            entry.scanDate.toString("yyyy-MM-dd hh:mm")
        ));
        
        // Scan path
        m_historyTable->setItem(i, 1, new QTableWidgetItem(entry.scanPath));
        
        // Files scanned
        m_historyTable->setItem(i, 2, new QTableWidgetItem(
            QString::number(entry.filesScanned)
        ));
        
        // Data scanned
        m_historyTable->setItem(i, 3, new QTableWidgetItem(
            FileScanner::formatFileSize(entry.bytesScanned)
        ));
        
        // Threats found
        QTableWidgetItem* threatsItem = new QTableWidgetItem(
            QString::number(entry.threatsFound)
        );
        if (entry.threatsFound > 0) {
            threatsItem->setForeground(QBrush(MaterialTheme::Error));
            QFont boldFont;
            boldFont.setBold(true);
            threatsItem->setFont(boldFont);
        }
        m_historyTable->setItem(i, 4, threatsItem);
        
        // Duration
        m_historyTable->setItem(i, 5, new QTableWidgetItem(
            FileScanner::formatDuration(entry.scanDuration)
        ));
    }
    
    m_historyTable->resizeColumnsToContents();
}

void HistoryViewer::onRowDoubleClicked(int row, int column) {
    Q_UNUSED(column);
    
    if (row < 0 || row >= m_historyData.size()) {
        return;
    }
    
    const ScanHistoryEntry& entry = m_historyData[row];
    
    // Get full scan details with threats
    ThreatReport report = m_database->getScanDetails(entry.id);
    
    // Show detailed report
    if (entry.threatsFound > 0) {
        // Show threat viewer for scans with threats
        ThreatViewer* viewer = new ThreatViewer(report, this);
        viewer->setWindowTitle(QString("Scan Report - %1").arg(entry.scanDate.toString("yyyy-MM-dd hh:mm")));
        viewer->exec();
        viewer->deleteLater();
    } else {
        // Show summary for clean scans
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Scan Report");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("Scan Summary");
        msgBox.setInformativeText(
            QString("Date: %1\n"
                   "Path: %2\n"
                   "Files scanned: %3\n"
                   "Data scanned: %4\n"
                   "Duration: %5\n"
                   "Threats found: 0\n\n"
                   "✅ No threats detected!")
                .arg(entry.scanDate.toString("yyyy-MM-dd hh:mm"))
                .arg(entry.scanPath)
                .arg(entry.filesScanned)
                .arg(FileScanner::formatFileSize(entry.bytesScanned))
                .arg(FileScanner::formatDuration(entry.scanDuration))
        );
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }
}

void HistoryViewer::onDeleteClicked() {
    QList<QTableWidgetItem*> selectedItems = m_historyTable->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::information(this, "No Selection", "Please select a scan to delete.");
        return;
    }
    
    // Get unique selected rows
    QSet<int> selectedRows;
    for (QTableWidgetItem* item : selectedItems) {
        selectedRows.insert(item->row());
    }
    
    // Confirm deletion
    QString message = QString("Are you sure you want to delete %1 scan(s)?\nThis cannot be undone.").arg(selectedRows.size());
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Deletion", message, 
                                                              QMessageBox::Yes | QMessageBox::No);
    
    if (reply != QMessageBox::Yes) {
        return;
    }
    
    // Delete scans
    int deleted = 0;
    for (int row : selectedRows) {
        if (row >= 0 && row < m_historyData.size()) {
            int scanId = m_historyData[row].id;
            if (m_database->deleteScan(scanId)) {
                deleted++;
            }
        }
    }
    
    // Reload history
    loadHistory();
    
    QMessageBox::information(this, "Deletion Complete", 
                            QString("Successfully deleted %1 scan(s).").arg(deleted));
}
