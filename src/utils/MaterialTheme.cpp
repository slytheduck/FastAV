#include "MaterialTheme.h"
#include <QApplication>

// Material Design 3 Dark Theme - Modern Palette
const QColor MaterialTheme::Primary = QColor("#BB86FC");
const QColor MaterialTheme::PrimaryVariant = QColor("#3700B3");
const QColor MaterialTheme::Secondary = QColor("#03DAC6");
const QColor MaterialTheme::SecondaryVariant = QColor("#018786");
const QColor MaterialTheme::Background = QColor("#121212");
const QColor MaterialTheme::Surface = QColor("#1E1E1E");
const QColor MaterialTheme::Error = QColor("#CF6679");
const QColor MaterialTheme::OnPrimary = QColor("#000000");
const QColor MaterialTheme::OnSecondary = QColor("#000000");
const QColor MaterialTheme::OnBackground = QColor("#FFFFFF");
const QColor MaterialTheme::OnSurface = QColor("#FFFFFF");
const QColor MaterialTheme::OnError = QColor("#000000");
const QColor MaterialTheme::Success = QColor("#4CAF50");
const QColor MaterialTheme::Warning = QColor("#FF9800");

QPalette MaterialTheme::createPalette() {
    QPalette palette;
    
    palette.setColor(QPalette::Window, Background);
    palette.setColor(QPalette::WindowText, OnBackground);
    palette.setColor(QPalette::Base, Surface);
    palette.setColor(QPalette::AlternateBase, QColor("#2A2A2A"));
    palette.setColor(QPalette::ToolTipBase, Surface);
    palette.setColor(QPalette::ToolTipText, OnSurface);
    palette.setColor(QPalette::Text, OnSurface);
    palette.setColor(QPalette::Button, Surface);
    palette.setColor(QPalette::ButtonText, OnSurface);
    palette.setColor(QPalette::BrightText, Qt::white);
    palette.setColor(QPalette::Link, Primary);
    palette.setColor(QPalette::Highlight, Primary);
    palette.setColor(QPalette::HighlightedText, OnPrimary);
    
    return palette;
}

QString MaterialTheme::getButtonStyle() {
    return QString(R"(
        QPushButton {
            background-color: %1;
            color: %2;
            border: none;
            border-radius: 8px;
            padding: 12px 24px;
            font-size: 14px;
            font-weight: 500;
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }
        QPushButton:hover {
            background-color: %3;
        }
        QPushButton:pressed {
            background-color: %4;
        }
        QPushButton:disabled {
            background-color: #2A2A2A;
            color: #666666;
        }
    )").arg(Primary.name())
       .arg(OnPrimary.name())
       .arg(Primary.lighter(110).name())
       .arg(PrimaryVariant.name());
}

QString MaterialTheme::getCardStyle() {
    return QString(R"(
        QFrame {
            background-color: %1;
            border-radius: 12px;
            padding: 16px;
        }
    )").arg(Surface.name());
}

QString MaterialTheme::getProgressBarStyle() {
    return QString(R"(
        QProgressBar {
            border: none;
            border-radius: 4px;
            background-color: #2A2A2A;
            height: 8px;
            text-align: center;
        }
        QProgressBar::chunk {
            border-radius: 4px;
            background-color: %1;
        }
    )").arg(Primary.name());
}

QString MaterialTheme::getListViewStyle() {
    return QString(R"(
        QListView {
            background-color: %1;
            border: none;
            border-radius: 8px;
            padding: 8px;
        }
        QListView::item {
            border-radius: 4px;
            padding: 12px;
            margin: 2px;
        }
        QListView::item:hover {
            background-color: #2A2A2A;
        }
        QListView::item:selected {
            background-color: %2;
            color: %3;
        }
    )").arg(Surface.name())
       .arg(Primary.name())
       .arg(OnPrimary.name());
}

QString MaterialTheme::getStyleSheet() {
    return QString(R"(
        * {
            font-family: 'Segoe UI', 'Roboto', 'Ubuntu', sans-serif;
        }
        
        QMainWindow {
            background-color: %1;
        }
        
        QLabel {
            color: %2;
        }
        
        QLineEdit {
            background-color: %3;
            border: 2px solid #2A2A2A;
            border-radius: 8px;
            padding: 10px;
            color: %2;
            selection-background-color: %4;
        }
        QLineEdit:focus {
            border: 2px solid %4;
        }
        
        QTextEdit, QPlainTextEdit {
            background-color: %3;
            border: 2px solid #2A2A2A;
            border-radius: 8px;
            padding: 10px;
            color: %2;
            selection-background-color: %4;
        }
        
        QScrollBar:vertical {
            background-color: #2A2A2A;
            width: 12px;
            border-radius: 6px;
        }
        QScrollBar::handle:vertical {
            background-color: %5;
            border-radius: 6px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: %4;
        }
        
        QTableView {
            background-color: %3;
            border: none;
            border-radius: 8px;
            gridline-color: #2A2A2A;
        }
        QTableView::item {
            padding: 8px;
        }
        QTableView::item:selected {
            background-color: %4;
        }
        
        QHeaderView::section {
            background-color: #2A2A2A;
            color: %2;
            padding: 10px;
            border: none;
            font-weight: 600;
        }
        
        QMenuBar {
            background-color: %3;
            color: %2;
        }
        QMenuBar::item:selected {
            background-color: %4;
        }
        
        QMenu {
            background-color: %3;
            color: %2;
            border: 1px solid #2A2A2A;
            border-radius: 8px;
        }
        QMenu::item:selected {
            background-color: %4;
        }
    )").arg(Background.name())
       .arg(OnBackground.name())
       .arg(Surface.name())
       .arg(Primary.name())
       .arg(Primary.darker(150).name());
}

void MaterialTheme::applyTheme(QWidget* widget) {
    QPalette palette = createPalette();
    
    if (widget) {
        widget->setPalette(palette);
        widget->setStyleSheet(getStyleSheet());
    } else {
        QApplication::setPalette(palette);
        qApp->setStyleSheet(getStyleSheet());
    }
}
