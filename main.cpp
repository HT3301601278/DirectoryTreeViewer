#include <QApplication>
#include <QIcon>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setWindowIcon(QIcon(":/favicon.ico"));
    
    MainWindow window;
    window.show();
    
    return app.exec();
}