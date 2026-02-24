#include "gui/MainWindow.h"
#include "utils/MaterialTheme.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application info
    QApplication::setApplicationName("FastAV");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("FastAV");
    QApplication::setOrganizationDomain("fastav.app");
    
    // Apply Material Design theme
    MaterialTheme::applyTheme();
    
    // Create and show main window
    MainWindow window;
    window.show();
    
    return app.exec();
}
