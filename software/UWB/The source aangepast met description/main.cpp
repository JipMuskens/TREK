#include "RTLSDisplayApplication.h"
#include "mainwindow.h"
#include <QApplication>

/**
* @brief this is the application main entry point
*
*/
int main(int argc, char *argv[])
{
    RTLSDisplayApplication app(argc, argv);

    app.mainWindow()->show();

    return app.QApplication::exec();
}
