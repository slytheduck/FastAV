#include "ThreatViewer.h"
#include "../utils/MaterialTheme.h"
#include "../utils/FileScanner.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>

ThreatViewer::ThreatViewer(const ThreatReport& report, QWidget* parent)
    : QDialog(parent)
    , m_report(report)
{
    setWindowTitle("Threats Detected");
    setMinimumSize(900, 600);
    setupUI();
    populateTable();
}

void ThreatViewer::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    
    // Warning header
    QLabel* warningIcon = new QLabel("⚠️", this);
    warningIcon->setStyleSheet("font-size: 48px;");
    warningIcon->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(warningIcon);
    
    QLabel* title = new QLabel("Threats Detected!", this);
    QFont titleFont;
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setStyleSheet(QString("color: %1;").arg(MaterialTheme::Error.name()));
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);
    
    QLabel* subtitle = new QLabel(
        QString("Found %1 threat(s) during the scan").arg(m_report.getThreatCount()),
        this
    );
    subtitle->setStyleSheet("color: #AAAAAA; font-size: 14px;");
    subtitle->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(subtitle);
    
    // Threat table
    m_threatTable = new QTableWidget(this);
    m_threatTable->setColumnCount(4);
    m_threatTable->setHorizontalHeaderLabels({"Virus Name", "File Path", "Size", "Detection Time"});
    m_threatTable->setStyleSheet(QString(R"(
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
    
    m_threatTable->horizontalHeader()->setStretchLastSection(true);
    m_threatTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_threatTable->verticalHeader()->setVisible(false);
    m_threatTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_threatTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_threatTable->setAlternatingRowColors(true);
    
    mainLayout->addWidget(m_threatTable);
    
    // Info label
    QLabel* infoLabel = new QLabel(
        "⚠️ These files may be harmful. Consider deleting or quarantining them.",
        this
    );
    infoLabel->setStyleSheet(QString("color: %1; font-size: 12px; padding: 10px; "
                                    "background-color: #2A2A2A; border-radius: 6px;")
                            .arg(MaterialTheme::Warning.name()));
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);
    
    // Close button
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_closeButton = new QPushButton("Close", this);
    m_closeButton->setStyleSheet(MaterialTheme::getButtonStyle());
    m_closeButton->setCursor(Qt::PointingHandCursor);
    m_closeButton->setMinimumWidth(120);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    
    buttonLayout->addWidget(m_closeButton);
    
    mainLayout->addLayout(buttonLayout);
}

void ThreatViewer::populateTable() {
    const QVector<ThreatInfo>& threats = m_report.getThreats();
    m_threatTable->setRowCount(threats.size());
    
    for (int i = 0; i < threats.size(); ++i) {
        const ThreatInfo& threat = threats[i];
        
        // Virus name
        QTableWidgetItem* virusItem = new QTableWidgetItem(threat.virusName);
        virusItem->setForeground(QBrush(MaterialTheme::Error));
        QFont boldFont;
        boldFont.setBold(true);
        virusItem->setFont(boldFont);
        m_threatTable->setItem(i, 0, virusItem);
        
        // File path
        QTableWidgetItem* pathItem = new QTableWidgetItem(threat.filePath);
        pathItem->setToolTip(threat.filePath);
        m_threatTable->setItem(i, 1, pathItem);
        
        // File size
        QTableWidgetItem* sizeItem = new QTableWidgetItem(
            FileScanner::formatFileSize(threat.fileSize)
        );
        m_threatTable->setItem(i, 2, sizeItem);
        
        // Detection time
        QTableWidgetItem* timeItem = new QTableWidgetItem(
            threat.detectionTime.toString("yyyy-MM-dd hh:mm:ss")
        );
        m_threatTable->setItem(i, 3, timeItem);
    }
    
    m_threatTable->resizeColumnsToContents();
}
