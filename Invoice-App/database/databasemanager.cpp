#include "databasemanager.h"

#include <QSqlError>
#include <QDebug>

bool DatabaseManager::connect() {
    QSqlDatabase db = QSqlDatabase::addDatabase( "QMYSQL" );
    db.setHostName("localhost");
    db.setDatabaseName("invoice_system");
    db.setUserName("sriram");
    db.setPassword("root");

    if ( db.open( ) ) {
        qDebug() << "Database Connected";
        return true;
    }

    qDebug() << db.lastError().text();
    return false;
}

QSqlDatabase DatabaseManager::database() {
    return QSqlDatabase::database();
}