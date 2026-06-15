#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>

class DatabaseManager {
public:
    static bool connect();
    static QSqlDatabase database();
};

#endif