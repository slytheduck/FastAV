#include "ScanDialog.h"
#include "../utils/MaterialTheme.h"
#include "../utils/FileScanner.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QGroupBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QStandardPaths>
#include <QDir>

ScanDialog::ScanDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Select Scan Type");
    setMinimumSize(600, 500);
    setupUI();
}

void ScanDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    
    // Title
    QLabel* title = new QLabel("What would you like to scan?", this);
    QFont titleFont;
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setStyleSheet(QString("color: %1;").arg(MaterialTheme::Primary.name()));
    mainLayout->addWidget(title);
    
    // Quick scan options
    QGroupBox* quickScanGroup = new QGroupBox("Quick Scan Options", this);
    quickScanGroup->setStyleSheet(QString(R"(
        QGroupBox {
            font-size: 14px;
            font-weight: bold;
            border: 2px solid #2A2A2A;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px;
        }
    )"));
    
    QVBoxLayout* quickLayout = new QVBoxLayout(quickScanGroup);
    
    m_locationList = new QListWidget(this);
    m_locationList->setStyleSheet(MaterialTheme::getListViewStyle());
    m_locationList->setSelectionMode(QAbstractItemView::SingleSelection);
    addQuickScanOptions();
    quickLayout->addWidget(m_locationList);
    
    mainLayout->addWidget(quickScanGroup);
    
    // Custom path
    QGroupBox* customGroup = new QGroupBox("Custom Path", this);
    customGroup->setStyleSheet(quickScanGroup->styleSheet());
    
    QHBoxLayout* customLayout = new QHBoxLayout(customGroup);
    
    m_customPathEdit = new QLineEdit(this);
    m_customPathEdit->setPlaceholderText("Enter custom path or browse...");
    m_customPathEdit->setStyleSheet(QString(R"(
        QLineEdit {
            background-color: %1;
            border: 2px solid #2A2A2A;
            border-radius: 8px;
            padding: 10px;
            font-size: 13px;
        }
        QLineEdit:focus {
            border: 2px solid %2;
        }
    )").arg(MaterialTheme::Surface.name())
       .arg(MaterialTheme::Primary.name()));
    
    m_browseButton = new QPushButton("Browse", this);
    m_browseButton->setStyleSheet(QString(R"(
        QPushButton {
            background-color: %1;
            color: %2;
            border: none;
            border-radius: 8px;
            padding: 10px 20px;
            font-size: 13px;
        }
        QPushButton:hover {
            background-color: %3;
        }
    )").arg(MaterialTheme::Secondary.name())
       .arg(MaterialTheme::OnSecondary.name())
       .arg(MaterialTheme::Secondary.lighter(110).name()));
    m_browseButton->setCursor(Qt::PointingHandCursor);
    connect(m_browseButton, &QPushButton::clicked, this, &ScanDialog::onBrowseClicked);
    
    customLayout->addWidget(m_customPathEdit, 1);
    customLayout->addWidget(m_browseButton);
    
    mainLayout->addWidget(customGroup);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_cancelButton = new QPushButton("Cancel", this);
    m_cancelButton->setStyleSheet(QString(R"(
        QPushButton {
            background-color: transparent;
            color: #888888;
            border: 2px solid #888888;
            border-radius: 8px;
            padding: 10px 30px;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #2A2A2A;
        }
    )"));
    m_cancelButton->setCursor(Qt::PointingHandCursor);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    m_scanButton = new QPushButton("Start Scan", this);
    m_scanButton->setStyleSheet(MaterialTheme::getButtonStyle());
    m_scanButton->setCursor(Qt::PointingHandCursor);
    QFont buttonFont;
    buttonFont.setBold(true);
    m_scanButton->setFont(buttonFont);
    connect(m_scanButton, &QPushButton::clicked, this, &ScanDialog::onScanClicked);
    
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_scanButton);
    
    mainLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
}

void ScanDialog::addQuickScanOptions() {
    QStringList locations = FileScanner::getCommonLocations();
    
    for (const QString& location : locations) {
        QFileInfo info(location);
        QString displayName = info.fileName();
        if (displayName.isEmpty()) {
            displayName = location;
        }
        
        QListWidgetItem* item = new QListWidgetItem(displayName + " - " + location);
        item->setData(Qt::UserRole, location);
        m_locationList->addItem(item);
    }
}

void ScanDialog::onQuickScanClicked() {
    // Quick scan of common locations
    m_selectedPaths = FileScanner::getCommonLocations();
}

void ScanDialog::onFullScanClicked() {
    // Full system scan
    m_selectedPaths.clear();
    m_selectedPaths << QDir::homePath();
}

void ScanDialog::onCustomScanClicked() {
    onBrowseClicked();
}

void ScanDialog::onBrowseClicked() {
    QString path = QFileDialog::getExistingDirectory(this, "Select Directory to Scan");
    if (!path.isEmpty()) {
        m_customPathEdit->setText(path);
    }
}

void ScanDialog::onScanClicked() {
    m_selectedPaths.clear();
    
    // Get selected quick scan locations
    for (int i = 0; i < m_locationList->count(); ++i) {
        QListWidgetItem* item = m_locationList->item(i);
        if (item->isSelected()) {
            m_selectedPaths << item->data(Qt::UserRole).toString();
        }
    }
    
    // Add custom path if specified
    QString customPath = m_customPathEdit->text().trimmed();
    if (!customPath.isEmpty() && QFileInfo(customPath).exists()) {
        m_selectedPaths << customPath;
    }
    
    if (m_selectedPaths.isEmpty()) {
        // No selection, default to home
        m_selectedPaths << QDir::homePath();
    }
    
    accept();
}
