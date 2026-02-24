#ifndef HISTORYVIEWER_H
#define HISTORYVIEWER_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include "../core/Database.h"

class HistoryViewer : public QDialog {
    Q_OBJECT
    
public:
    explicit HistoryViewer(Database* database, QWidget* parent = nullptr);
    
private slots:
    void onRowDoubleClicked(int row, int column);
    void onDeleteClicked();
    
private:
    void setupUI();
    void loadHistory();
    
    Database* m_database;
    QTableWidget* m_historyTable;
    QPushButton* m_closeButton;
    QVector<ScanHistoryEntry> m_historyData; // Store full history data
};

#endif // HISTORYVIEWER_H
