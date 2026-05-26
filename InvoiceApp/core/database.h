#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QString>

class Database
{
public:
    // Singleton access point
    static Database& instance();

    // Open SQLite database
    bool connect(const QString& filePath);

    // Return database handle
    QSqlDatabase& db();

    // Initialize tables
    bool initSchema();

private:
    Database() = default;

    // Prevent copying
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    QSqlDatabase m_db;
};

#endif // DATABASE_H
