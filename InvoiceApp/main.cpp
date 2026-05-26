#include <QApplication>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>

#include "core/database.h"
#include "ui/loginwindow.h"
#include "ui/mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    const QString dataDir =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QString dbPath = dataDir + "/invoice.db";

    if (!Database::instance().connect(dbPath)) {
        QMessageBox::critical(nullptr,
                              "Fatal",
                              "Cannot open database.");
        return 1;
    }

    if (!Database::instance().initSchema()) {
        QMessageBox::critical(nullptr,
                              "Fatal",
                              "Cannot initialize schema.");
        return 1;
    }

    LoginWindow login;
    if (login.exec() != QDialog::Accepted) {
        return 0;
    }

    MainWindow window;
    window.show();

    return app.exec();
}
