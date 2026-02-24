#ifndef THREATVIEWER_H
#define THREATVIEWER_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include "../core/ThreatReport.h"

class ThreatViewer : public QDialog {
    Q_OBJECT
    
public:
    explicit ThreatViewer(const ThreatReport& report, QWidget* parent = nullptr);
    
private:
    void setupUI();
    void populateTable();
    
    ThreatReport m_report;
    QTableWidget* m_threatTable;
    QPushButton* m_closeButton;
};

#endif // THREATVIEWER_H
