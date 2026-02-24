#ifndef SCANDIALOG_H
#define SCANDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QStringList>

class ScanDialog : public QDialog {
    Q_OBJECT
    
public:
    explicit ScanDialog(QWidget* parent = nullptr);
    QStringList getSelectedPaths() const { return m_selectedPaths; }
    
private slots:
    void onQuickScanClicked();
    void onFullScanClicked();
    void onCustomScanClicked();
    void onBrowseClicked();
    void onScanClicked();
    
private:
    void setupUI();
    void addQuickScanOptions();
    
    QListWidget* m_locationList;
    QLineEdit* m_customPathEdit;
    QPushButton* m_browseButton;
    QPushButton* m_scanButton;
    QPushButton* m_cancelButton;
    
    QStringList m_selectedPaths;
};

#endif // SCANDIALOG_H
