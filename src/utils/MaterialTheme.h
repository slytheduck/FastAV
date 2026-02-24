#ifndef MATERIALTHEME_H
#define MATERIALTHEME_H

#include <QWidget>
#include <QPalette>
#include <QString>
#include <QColor>

class MaterialTheme {
public:
    // Material Design 3 Dark Theme Colors
    static const QColor Primary;
    static const QColor PrimaryVariant;
    static const QColor Secondary;
    static const QColor SecondaryVariant;
    static const QColor Background;
    static const QColor Surface;
    static const QColor Error;
    static const QColor OnPrimary;
    static const QColor OnSecondary;
    static const QColor OnBackground;
    static const QColor OnSurface;
    static const QColor OnError;
    static const QColor Success;
    static const QColor Warning;
    
    // Apply theme to application
    static void applyTheme(QWidget* widget = nullptr);
    static QPalette createPalette();
    static QString getStyleSheet();
    
    // Helper functions
    static QString getButtonStyle();
    static QString getCardStyle();
    static QString getProgressBarStyle();
    static QString getListViewStyle();
};

#endif // MATERIALTHEME_H
