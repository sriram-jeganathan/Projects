#include <QApplication>

#include "mainwindow.h"

#include "database/databasemanager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    DatabaseManager::connect();

    MainWindow window;

    window.show();

    return app.exec();
}